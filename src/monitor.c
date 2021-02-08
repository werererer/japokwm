#include "monitor.h"
#include <assert.h>
#include <stdlib.h>
#include <wlr/types/wlr_list.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

#include "ipc-server.h"
#include "lib/actions/actions.h"
#include "parseConfig.h"
#include "render/render.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "workspace.h"

struct wl_list stack;
struct wl_list focus_stack;
struct wl_list containers;
struct wl_list layer_stack;
struct wl_list popups;
struct wl_list sticky_stack;

struct wl_list mons;
struct monitor *selected_monitor;

static void handle_output_damage_frame(struct wl_listener *listener, void *data);
static void handle_output_frame(struct wl_listener *listener, void *data);
static void handle_output_mode(struct wl_listener *listener, void *data);

void create_monitor(struct wl_listener *listener, void *data)
{
    /* This event is raised by the backend when a new output (aka a display or
     * monitor) becomes available. */
    struct wlr_output *output = data;
    struct monitor *m;

    /* The mode is a tuple of (width, height, refresh rate), and each
     * monitor supports only a specific set of modes. We just pick the
     * monitor's preferred mode; a more sophisticated compositor would let
     * the user configure it. */
    wlr_output_set_mode(output, wlr_output_preferred_mode(output));

    /* Allocates and configures monitor state using configured rules */
    m = output->data = calloc(1, sizeof(struct monitor));

    m->wlr_output = output;

    /* damage tracking must be initialized before setting the workspace because
     * it to damage a region */
    m->damage = wlr_output_damage_create(m->wlr_output);
    m->damage_frame.notify = handle_output_damage_frame;
    wl_signal_add(&m->damage->events.frame, &m->damage_frame);

    m->frame.notify = handle_output_frame;
    wl_signal_add(&m->wlr_output->events.frame, &m->frame);

    wlr_xcursor_manager_load(server.cursor_mgr, 1);
    for (int i = 0; i < server.default_layout.options.monrule_count; i++) {
        struct monrule r = server.default_layout.options.monrules[i];
        if (!r.name || strstr(output->name, r.name)) {
            m->mfact = r.mfact;
            wlr_output_set_scale(output, r.scale);
            set_selected_layout(m->ws[0], r.lt);
            wlr_output_set_transform(output, r.rr);
            break;
        }
    }

    /* Set up event listeners */
    m->destroy.notify = destroy_monitor;
    wl_signal_add(&output->events.destroy, &m->destroy);
    m->mode.notify = handle_output_mode;
    wl_signal_add(&output->events.mode, &m->mode);

    bool is_first_monitor = wl_list_empty(&mons);
    wl_list_insert(&mons, &m->link);

    if (is_first_monitor) {
        init_config(L);
        set_selected_monitor(m);
        if (server.default_layout.options.tag_names.length <= 0) {
            handle_error("tag_names is empty, loading default tag_names");
            reset_tag_names(&server.default_layout.options.tag_names);
        }

        create_workspaces(server.default_layout.options.tag_names, server.default_layout);
    }

    m->root = create_root(m);

    set_next_unoccupied_workspace(m, get_workspace(0));
    load_default_layout(L, m->ws[0]);
    copy_layout_from_selected_workspace();
    set_root_color(m->root, m->ws[0]->layout[0].options.root_color);

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

    wlr_output_enable(output, 1);

    if (!wlr_output_commit(output))
        return;
}

static void handle_output_frame(struct wl_listener *listener, void *data)
{
}

static void handle_output_damage_frame(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, damage_frame);

    if (!m->wlr_output->enabled) {
        return;
    }

    /* Check if we can scan-out the primary view. */
    bool needs_frame;
    pixman_region32_t damage;
    pixman_region32_init(&damage);
    if (!wlr_output_damage_attach_render(m->damage, &needs_frame, &damage))
        goto damage_finish;

    if (!needs_frame) {
        wlr_output_rollback(m->wlr_output);
        pixman_region32_fini(&damage);
        return;
    }

    render_frame(m, &damage);

damage_finish:
    pixman_region32_fini(&damage);
}

static void handle_output_mode(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, mode);
    if (!m)
        return;
    arrange_monitor(m);
}

void focusmon(int i)
{
    selected_monitor = dirtomon(i);
    struct workspace *ws = selected_monitor->ws[0];
    focus_top_container(ws, FOCUS_LIFT);
}

void destroy_monitor(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, destroy);

    set_workspace(m, NULL);
    destroy_root(m->root);
    wl_list_remove(&m->link);
}

void center_mouse_in_monitor(struct monitor *m)
{
    if (!m)
        return;

    int xcenter = m->geom.x + m->geom.width/2;
    int ycenter = m->geom.y + m->geom.height/2;
    wlr_cursor_warp(server.cursor.wlr_cursor, NULL, xcenter, ycenter);
}

void set_selected_monitor(struct monitor *m)
{
    assert(m);
    if (selected_monitor == m)
        return;

    selected_monitor = m;
    set_workspace(m, m->ws[0]);
    int x = server.cursor.wlr_cursor->x;
    int y = server.cursor.wlr_cursor->y;

    focus_container(xytocontainer(x, y), FOCUS_NOOP);
}

void push_selected_workspace(struct monitor *m, struct workspace *ws)
{
    if (!m || !ws)
        return;
    set_workspace(m, ws);
}

struct monitor *dirtomon(int dir)
{
    struct monitor *m;

    if (dir > 0) {
        if (selected_monitor->link.next == &mons)
            return wl_container_of(mons.next, m, link);
        return wl_container_of(selected_monitor->link.next, m, link);
    } else {
        if (selected_monitor->link.prev == &mons)
            return wl_container_of(mons.prev, m, link);
        return wl_container_of(selected_monitor->link.prev, m, link);
    }
}

struct monitor *output_to_monitor(struct wlr_output *output)
{
    struct monitor *m;
    bool found = false;
    wl_list_for_each(m, &mons, link) {
        if (m->wlr_output == output) {
            found = true;
            break;
        }
    }
    return found ? m : NULL;
}

struct monitor *xytomon(double x, double y)
{
    struct wlr_output *o = wlr_output_layout_output_at(server.output_layout, x, y);
    return o ? o->data : NULL;
}
