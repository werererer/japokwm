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

struct wl_list focus_stack;

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
    wl_list_init(&m->containers);
    wl_list_init(&m->stack);
    wl_list_init(&m->layer_stack);
    wl_list_init(&m->popups);

    struct client *c;
    wl_list_for_each(c, &clients, link) {
        create_container(c, m);
    }

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

void remove_container_from_monitor(struct monitor *m, struct container *con)
{
    printf("remove from container\n");
    if (m && con) {
        wl_list_remove(&con->slink);
        wl_list_remove(&con->flink);
    }
}

void focusmon(int i)
{
    selected_monitor = dirtomon(i);
    focus_top_container(selected_monitor, FOCUS_LIFT);
}

void destroy_monitor(struct wl_listener *listener, void *data)
{
    printf("destroy_monitor\n");
    struct wlr_output *wlr_output = data;
    struct monitor *m = wlr_output->data;
    destroy_root(m->root);

    wl_list_remove(&m->link);
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

static void set_layer_shell(struct container *con)
{
    con->geom.x = 0;
    con->geom.y = 0;
    if (con->client->surface.layer->current.desired_width)
        con->geom.width = con->client->surface.layer->current.desired_width;
    else
        con->geom.width = selected_monitor->wlr_output->width;

    if (con->client->surface.layer->current.desired_height)
        con->geom.height = con->client->surface.layer->current.desired_height;
    else
        con->geom.height = selected_monitor->wlr_output->height;
    /* wlr_layer_surface_v1_configure(con->client->surface.layer, con->geom.width, */
    /*         con->geom.height); */
    resize(con, con->geom, false);
}

// TODO: Reduce side effects
void set_root_area(struct monitor *m)
{
    m->root->w = m->geom;
    int maxWidth = 0, maxHeight = 0;
    struct container *con;
    wl_list_for_each(con, &m->layer_stack, llink) {
        set_layer_shell(con);
        // if desired_width/height == 0 they are fullscreen and have no effect
        maxWidth = MAX(maxWidth, con->client->surface.layer->current.desired_width);
        maxHeight = MAX(maxHeight, con->client->surface.layer->current.desired_height);
        // move the current window barely out of view
        if (!m->root->consider_layer_shell) {
            con->geom.x = -maxWidth;
            con->geom.y = -maxHeight;
        }
    }
    if (m->root->consider_layer_shell) {
        m->root->w.x += maxWidth;
        m->root->w.width -= maxWidth;
        m->root->w.y += maxHeight;
        m->root->w.height -= maxHeight;
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
    return get_workspace(m->focused_workspace[0]);
}

void push_selected_workspace(struct monitor *m, struct workspace *ws)
{
    if (!m || !ws)
        return;
    m->focused_workspace[1] = m->focused_workspace[0];
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
