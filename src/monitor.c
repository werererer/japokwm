#include "monitor.h"
#include <assert.h>
#include <stdlib.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>
#include <wlr/backend/wayland.h>
#include <wlr/backend/headless.h>
#include <wlr/backend/x11.h>

#include "ipc-server.h"
#include "render/render.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "workspace.h"
#include "utils/parseConfigUtils.h"
#include "layer_shell.h"
#include "rules/mon_rule.h"
#include "root.h"
#include "tagset.h"
#include "list_sets/container_stack_set.h"
#include "client.h"
#include "container.h"
#include "workspace.h"

static void handle_output_damage_frame(struct wl_listener *listener, void *data);
static void handle_output_frame(struct wl_listener *listener, void *data);
static void handle_output_mode(struct wl_listener *listener, void *data);
static void monitor_get_initial_workspace(struct monitor *m, GPtrArray *workspaces);

void create_monitor(struct wl_listener *listener, void *data)
{
    /* This event is raised by the backend when a new output (aka a display or
     * monitor) becomes available. */
    struct wlr_output *output = data;

    /* The mode is a tuple of (width, height, refresh rate), and each
     * monitor supports only a specific set of modes. We just pick the
     * monitor's preferred mode; a more sophisticated compositor would let
     * the user configure it. */
    wlr_output_set_mode(output, wlr_output_preferred_mode(output));

    /* Allocates and configures monitor state using configured rules */
    struct monitor *m = output->data = calloc(1, sizeof(*m));
    m->ws_id = INVALID_WORKSPACE_ID;

    m->wlr_output = output;

    /* damage tracking must be initialized before setting the workspace because
     * it to damage a region */
    m->damage = wlr_output_damage_create(m->wlr_output);
    m->damage_frame.notify = handle_output_damage_frame;
    wl_signal_add(&m->damage->events.frame, &m->damage_frame);

    m->frame.notify = handle_output_frame;
    wl_signal_add(&m->wlr_output->events.frame, &m->frame);

    /* Set up event listeners */
    m->destroy.notify = handle_destroy_monitor;
    wl_signal_add(&output->events.destroy, &m->destroy);
    m->mode.notify = handle_output_mode;
    wl_signal_add(&output->events.mode, &m->mode);

    bool is_first_monitor = server.mons->len == 0;
    g_ptr_array_add(server.mons, m);

    /* Adds this to the output layout. The add_auto function arranges outputs
     * from left-to-right in the order they appear. A more sophisticated
     * compositor would let the user configure the arrangement of outputs in the
     * layout.
     *
     * The output layout utility automatically adds a wl_output global to the
     * display, which Wayland clients can see to find out information about the
     * output (such as DPI, scale factor, manufacturer, etc).
     */
    wlr_output_layout_add_auto(server.output_layout, output);

    wlr_output_enable(output, true);

    m->geom = *wlr_output_layout_get_box(server.output_layout, m->wlr_output);
    m->root = create_root(m, m->geom);

    if (is_first_monitor) {
        server_set_selected_monitor(m);
    }
    monitor_get_initial_workspace(m, server.workspaces);

    if (is_first_monitor) {
        load_config(L);
        load_workspaces(server.workspaces, server.default_layout->options->tag_names);
        server_allow_reloading_config();

        bitset_set(server.previous_bitset, server.previous_workspace);

        call_on_start_function(server.event_handler);
    }

    apply_mon_rules(server.default_layout->options->mon_rules, m);

    arrange();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = workspace_get_layout(ws);
    set_root_color(m->root, lt->options->root_color);
}

void create_output(struct wlr_backend *backend, void *data)
{
    bool *done = data;
    if (*done) {
        return;
    }

    if (wlr_backend_is_wl(backend)) {
        wlr_wl_output_create(backend);
        *done = true;
    } else if (wlr_backend_is_headless(backend)) {
        wlr_headless_add_output(backend, 1920, 1080);
        *done = true;
    }
/* #if WLR_HAS_X11_BACKEND */
    else if (wlr_backend_is_x11(backend)) {
        wlr_x11_output_create(backend);
        *done = true;
    }
/* #endif */
}

int i = 0;
static void handle_output_frame(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, damage_frame);
    // debug_print("frame: %i\n", i++);

    /* NOOP */
}

int j = 0;
static void handle_output_damage_frame(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, damage_frame);

    if (!m->wlr_output->enabled) {
        return;
    }

    /* Check if we can scan-out the primary view. */
    pixman_region32_t frame_damage;
    pixman_region32_init(&frame_damage);
    bool needs_frame;
    if (!wlr_output_damage_attach_render(m->damage, &needs_frame, &frame_damage)) {
        goto damage_finish;
    }

    if (!needs_frame) {
        wlr_output_rollback(m->wlr_output);
        goto damage_finish;
    }

    render_monitor(m, &frame_damage);

damage_finish:
    pixman_region32_fini(&frame_damage);
}

static void handle_output_mode(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, mode);
    m->geom = *wlr_output_layout_get_box(server.output_layout, m->wlr_output);
    arrange_monitor(m);
    arrange_layers(m);
}

static void monitor_get_initial_workspace(struct monitor *m, GPtrArray *workspaces)
{
    struct workspace *ws = find_next_unoccupied_workspace(workspaces, get_workspace(0));

    assert(ws != NULL);

    int ws_id = ws->id;
    BitSet *bitset = bitset_create();
    bitset_set(bitset, ws_id);

    monitor_focus_tags(m, ws, bitset);
}

void handle_destroy_monitor(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, destroy);

    wl_list_remove(&m->mode.link);
    wl_list_remove(&m->frame.link);
    wl_list_remove(&m->destroy.link);

    struct workspace *ws = monitor_get_active_workspace(m);
    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = get_workspace(i);
        if (ws->m == m) {
            workspace_set_selected_monitor(ws, NULL);
        }
        if (ws->current_m == m) {
            workspace_set_current_monitor(ws, NULL);
        }
    }

    GPtrArray *stack_list = workspace_get_complete_stack_copy(ws);
    for (int i = 0; i < stack_list->len; i++) {
        struct container *con = g_ptr_array_index(stack_list, i);

        if (con->client->type == LAYER_SHELL && con->client->m  == m) {
            // FIXME: this needs to be done so that instead of segfaulting when
            // layer shell based programs are open it leaks the memory instead
            // this seems to be a bug in wlroots or wayland
            wl_list_remove(&con->client->commit.link);
            wl_list_remove(&con->client->destroy.link);
        }
        if (con->client->m == m) {
            con->client->m = NULL;
        }
    }
    g_ptr_array_unref(stack_list);

    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(server.workspaces, i);
        if (ws->current_m == m) {
            debug_print("unset ws: %i\n", ws->id);
            workspace_set_current_monitor(ws, NULL);
        }
    }

    destroy_root(m->root);
    g_ptr_array_remove(server.mons, m);
    m->wlr_output->data = NULL;

    free(m);

    if (server.mons->len <= 0) {
        /* wl_display_terminate(server.wl_display); */
        return;
    }

    struct monitor *new_focused_monitor = g_ptr_array_index(server.mons, 0);
    server_set_selected_monitor(new_focused_monitor);
}

void monitor_set_selected_workspace(struct monitor *m, struct workspace *ws)
{
    int prev_ws_id = m->ws_id;
    struct workspace *prev_ws = get_workspace(prev_ws_id);
    if (prev_ws) {
        prev_ws->m = NULL;
    }

    m->ws_id = ws->id;
    workspace_set_selected_monitor(ws, m);
}

BitSet *monitor_get_workspaces(struct monitor *m)
{
    struct workspace *sel_ws = monitor_get_active_workspace(m);
    return sel_ws->workspaces;
}

void center_cursor_in_monitor(struct cursor *cursor, struct monitor *m)
{
    if (!m)
        return;

    int xcenter = m->geom.x + m->geom.width/2;
    int ycenter = m->geom.y + m->geom.height/2;
    wlr_cursor_warp(cursor->wlr_cursor, NULL, xcenter, ycenter);
}

void scale_monitor(struct monitor *m, float scale)
{
    struct wlr_output *output = m->wlr_output;
    m->scale = scale;
    wlr_output_set_scale(output, scale);
}

void transform_monitor(struct monitor *m, enum wl_output_transform transform)
{
    struct wlr_output *output = m->wlr_output;
    wlr_output_set_transform(output, transform);
}

void update_monitor_geometries()
{
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        m->geom = *wlr_output_layout_get_box(server.output_layout, m->wlr_output);
    }
}

void focus_monitor(struct monitor *m)
{
    if (!m)
        return;
    if (server_get_selected_monitor() == m)
        return;

    /* wlr_xwayland_set_seat(server.xwayland.wlr_xwayland, m->wlr_output.) */

    // move floating containers over
    struct workspace *ws = monitor_get_active_workspace(m);
    tagset_focus_tags(ws, ws->workspaces);
    server_set_selected_monitor(m);
}

struct monitor *output_to_monitor(struct wlr_output *output)
{
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        if (m->wlr_output == output) {
            return m;
        }
    }

    return NULL;
}

struct monitor *xy_to_monitor(double x, double y)
{
    struct wlr_output *o = wlr_output_layout_output_at(server.output_layout, x, y);
    return o ? o->data : NULL;
}

inline struct workspace *monitor_get_active_workspace(struct monitor *m)
{
    if (!m)
        return NULL;

    struct workspace *ws = get_workspace(m->ws_id);
    // if this is not correct the whole application is in an undefined state
    if (ws) {
        assert(ws->m == m);
    }
    return ws;
}

inline struct layout *get_layout_in_monitor(struct monitor *m)
{
    if (!m)
        return NULL;
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = workspace_get_layout(ws);
    return lt;
}

struct root *monitor_get_active_root(struct monitor *m)
{
    struct root *root = m->root;
    return root;
}

struct wlr_box monitor_get_active_geom(struct monitor *m)
{
    struct workspace *ws = monitor_get_active_workspace(m);
    struct wlr_box geom = workspace_get_active_geom(ws);
    return geom;
}
