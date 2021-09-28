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

struct monitor *selected_monitor;

static void handle_output_damage_frame(struct wl_listener *listener, void *data);
static void handle_output_frame(struct wl_listener *listener, void *data);
static void handle_output_mode(struct wl_listener *listener, void *data);

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
    struct monitor *m = output->data = calloc(1, sizeof(struct monitor));

    m->wlr_output = output;

    /* damage tracking must be initialized before setting the workspace because
     * it to damage a region */
    m->damage = wlr_output_damage_create(m->wlr_output);
    m->damage_frame.notify = handle_output_damage_frame;
    wl_signal_add(&m->damage->events.frame, &m->damage_frame);

    m->frame.notify = handle_output_frame;
    wl_signal_add(&m->wlr_output->events.frame, &m->frame);

    /* Set up event listeners */
    m->destroy.notify = destroy_monitor;
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
        focus_monitor(m);

        if (server.default_layout->options.tag_names->len <= 0) {
            handle_error("tag_names is empty, loading default tag_names");
            create_tagnames(&server.default_layout->options.tag_names);
        }

        server.workspaces = create_workspaces(server.default_layout->options.tag_names);
        server.previous_bitset = bitset_create(server.workspaces->len);
        bitset_set(server.previous_bitset, server.previous_workspace);

        call_on_start_function(server.default_layout->options.event_handler);
    }

    apply_mon_rules(server.default_layout->options.mon_rules, m);

    focus_next_unoccupied_workspace(m, server.workspaces, get_workspace(0));
    load_default_layout(L);
    struct workspace *ws = monitor_get_active_workspace(m);
    copy_layout_from_selected_workspace(server.workspaces);
    set_root_color(m->root, ws->layout->options.root_color);

    if (!wlr_output_commit(output))
        return;
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

static void handle_output_frame(struct wl_listener *listener, void *data)
{
    /* NOOP */
}

static void handle_output_damage_frame(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, damage_frame);

    if (!m->wlr_output->enabled) {
        return;
    }

    /* Check if we can scan-out the primary view. */
    pixman_region32_t damage;
    pixman_region32_init(&damage);
    bool needs_frame;
    if (!wlr_output_damage_attach_render(m->damage, &needs_frame, &damage)) {
        goto damage_finish;
    }

    if (!needs_frame) {
        wlr_output_rollback(m->wlr_output);
        goto damage_finish;
    }

    render_monitor(m, &damage);

damage_finish:
    pixman_region32_fini(&damage);
}

static void handle_output_mode(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, mode);
    m->geom = *wlr_output_layout_get_box(server.output_layout, m->wlr_output);
    arrange_monitor(m);
    arrange_layers(m);
}

void destroy_monitor(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, destroy);

    wl_list_remove(&m->mode.link);
    wl_list_remove(&m->frame.link);
    wl_list_remove(&m->damage_frame.link);
    wl_list_remove(&m->destroy.link);

    destroy_tagset(m->tagset);
    destroy_root(m->root);
    g_ptr_array_remove(server.mons, m);
    m->wlr_output->data = NULL;

    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(server.workspaces, i);
        if (ws->prev_m == m) {
            ws->prev_m = NULL;
        }
    }

    free(m);

    if (server.mons->len <= 0)
        return;

    struct monitor *new_focused_monitor = g_ptr_array_index(server.mons, 0);
    selected_monitor = new_focused_monitor;
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
    if (selected_monitor == m)
        return;

    /* wlr_xwayland_set_seat(server.xwayland.wlr_xwayland, m->wlr_output.) */

    // move floating containers over
    struct tagset *tagset = monitor_get_active_tagset(m);

    selected_monitor = m;
    focus_tagset(tagset);
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

struct tagset *monitor_get_active_tagset(struct monitor *m)
{
    if (!m)
        return NULL;

    return m->tagset;
}

inline struct workspace *monitor_get_active_workspace(struct monitor *m)
{
    if (!m)
        return NULL;

    struct tagset *tagset = monitor_get_active_tagset(m);
    struct workspace *ws = tagset_get_workspace(tagset);
    return ws;
}

inline struct layout *get_layout_in_monitor(struct monitor *m)
{
    if (!m)
        return NULL;
    return monitor_get_active_workspace(m)->layout;
}

struct root *monitor_get_active_root(struct monitor *m)
{
    struct root *root = m->root;
    return root;
}

struct wlr_box monitor_get_active_geom(struct monitor *m)
{
    struct workspace *ws = monitor_get_active_workspace(m);
    debug_print("geom ws: %i\n", ws->id);
    struct wlr_box geom = workspace_get_active_geom(ws);
    return geom;
}
