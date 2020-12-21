#include "monitor.h"
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_list.h>
#include <stdlib.h>
#include <wlr/util/log.h>

#include "parseConfig.h"
#include "render/render.h"
#include "server.h"
#include "workspace.h"
#include "tile/tileUtils.h"
#include "ipc-server.h"

struct wl_list stack;
struct wl_list focus_stack;
struct wl_list containers;
struct wl_list layer_stack;
struct wl_list popups;

/* monitors */
static const struct mon_rule monrules[] = {
    /* name       mfact nmaster scale layout       rotate/reflect */
    /* example of a HiDPI laptop monitor:
    { "eDP-1",    0.5,  1,      2,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL },
    */
    /* defaults */
    { NULL, 0.55, 1, 1, &defaultLayout, WL_OUTPUT_TRANSFORM_NORMAL },
};

struct wl_list mons;
struct monitor *selected_monitor = NULL;

void create_monitor(struct wl_listener *listener, void *data)
{
    /* This event is raised by the backend when a new output (aka a display or
     * monitor) becomes available. */
    struct wlr_output *output = data;
    struct monitor *m;
    const struct mon_rule *r;

    /* The mode is a tuple of (width, height, refresh rate), and each
     * monitor supports only a specific set of modes. We just pick the
     * monitor's preferred mode; a more sophisticated compositor would let
     * the user configure it. */
    wlr_output_set_mode(output, wlr_output_preferred_mode(output));

    /* Allocates and configures monitor state using configured rules */
    m = output->data = calloc(1, sizeof(struct monitor));

    m->wlr_output = output;
    m->root = create_root();
    set_next_unoccupied_workspace(m, get_workspace(0));

    for (r = monrules; r < END(monrules); r++) {
        if (!r->name || strstr(output->name, r->name)) {
            m->mfact = r->mfact;
            m->nmaster = r->nmaster;
            wlr_output_set_scale(output, r->scale);
            wlr_xcursor_manager_load(server.cursorMgr, r->scale);
            set_selected_layout(m->ws, *r->lt);
            wlr_output_set_transform(output, r->rr);
            break;
        }
    }
    /* Set up event listeners */
    m->frame.notify = render_frame;
    wl_signal_add(&output->events.frame, &m->frame);
    m->destroy.notify = destroy_monitor;
    wl_signal_add(&output->events.destroy, &m->destroy);

    wl_list_insert(&mons, &m->link);

    wlr_output_enable(output, 1);
    if (!wlr_output_commit(output))
        return;

    /* Adds this to the output layout. The add_auto function arranges outputs
     * from left-to-right in the order they appear. A more sophisticated
     * compositor would let the user configure the arrangement of outputs in the
     * layout.
     *
     * The output layout utility automatically adds a wl_output global to the
     * display, which Wayland clients can see to find out information about the
     * output (such as DPI, scale factor, manufacturer, etc).
     */
    wlr_output_layout_add_auto(output_layout, output);
}

void focusmon(int i)
{
    selected_monitor = dirtomon(i);
    focus_top_container(selected_monitor, FOCUS_LIFT);
}

void destroy_monitor(struct wl_listener *listener, void *data)
{
    struct wlr_output *wlr_output = data;
    struct monitor *m = wlr_output->data;

    set_workspace(m, NULL);
    destroy_root(m->root);
    wl_list_remove(&m->link);

    /* m = wl_container_of(&mons.next, m, link); */
    /* set_selected_monitor(m); */
}

void set_selected_monitor(struct monitor *m)
{
    if (selected_monitor == m)
        return;

    selected_monitor = m;
    set_workspace(m, m->ws);
    int xcentre = m->geom.x + (float)m->geom.width/2;
    int ycentre = m->geom.y + (float)m->geom.height/2;
    wlr_cursor_warp(server.cursor, NULL, xcentre, ycentre);
    focus_container(m, xytocontainer(xcentre, ycentre), FOCUS_NOOP);
    arrange(LAYOUT_NOOP);
}

static void configure_layer_shell_container_geom(struct container *con, struct monitor *m)
{
    if (!con->client)
        return;
    if (con->client->type != LAYER_SHELL)
        return;

    con->geom.x = m->geom.x;
    con->geom.y = m->geom.y;
    if (con->client->surface.layer->current.desired_width)
        con->geom.width = con->client->surface.layer->current.desired_width;
    else
        con->geom.width = selected_monitor->wlr_output->width;

    if (con->client->surface.layer->current.desired_height)
        con->geom.height = con->client->surface.layer->current.desired_height;
    else
        con->geom.height = selected_monitor->wlr_output->height;
    resize(con, con->geom, false);
}

// TODO fix this to allow placement on all sides on screen
static struct wlr_box get_max_dimensions(struct monitor *m)
{
    struct wlr_box box;

    struct container *con;
    wl_list_for_each(con, &layer_stack, llink) {
        if (!visibleon(con, m))
            continue;

        // desired_width and desired_height are == 0 if nothing is desired
        int desired_width = con->client->surface.layer->current.desired_width;
        int desired_height = con->client->surface.layer->current.desired_height;

        box.width = MAX(box.width, desired_width);
        box.height = MAX(box.height, desired_height);
    }

    return box;
}

void set_root_area(struct monitor *m)
{
    m->root->geom = m->geom;

    struct wlr_box max_geom = get_max_dimensions(m);

    if (m->root->consider_layer_shell) {
        m->root->geom.x += max_geom.width;
        m->root->geom.width -= max_geom.width;
        m->root->geom.y += max_geom.height;
        m->root->geom.height -= max_geom.height;
    }

    struct container *con;
    wl_list_for_each(con, &layer_stack, llink) {
        if (!visibleon(con, m))
            continue;

        configure_layer_shell_container_geom(con, m);

        // move the current window barely out of view
        if (!m->root->consider_layer_shell) {
            con->geom.x = -max_geom.width;
            con->geom.y = -max_geom.height;
        }
    }
}

struct layout *selected_layout(struct monitor *m)
{
    if (!m)
        return 0;
    return &get_focused_workspace(m)->layout;
}

struct workspace *get_focused_workspace(struct monitor *m)
{
    return get_workspace(m->ws->id);
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

struct monitor *outputtomon(struct wlr_output *output)
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
    struct wlr_output *o = wlr_output_layout_output_at(output_layout, x, y);
    return o ? o->data : NULL;
}
