#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <wayland-util.h>
#include <string.h>
#include <assert.h>

#include "container.h"
#include "popup.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "ipc-server.h"
#include "tag.h"
#include "scratchpad.h"
#include "monitor.h"
#include "tagset.h"
#include "rules/rule.h"
#include "server.h"

struct client *create_client(enum shell shell_type, union surface_t surface)
{
    struct client *c = calloc(1, sizeof(*c));

    c->sticky_tags = bitset_create_with_data(c);
    c->type = shell_type;
    c->surface = surface;

    return c;
}

void destroy_client(struct client *c)
{
    bitset_destroy(c->sticky_tags);
    free(c);
}

struct wlr_surface *get_base_wlrsurface(struct client *c)
{
    if (!c)
        return NULL;

    struct wlr_surface *ret_surface;
    switch (c->type) {
        case X11_MANAGED:
        case X11_UNMANAGED:
            {
                struct wlr_xwayland_surface *xwayland_surface = c->surface.xwayland;
                while (xwayland_surface->parent)
                    xwayland_surface = xwayland_surface->parent;
                ret_surface = xwayland_surface->surface;
                break;
            }
        case XDG_SHELL:
            ret_surface = get_wlrsurface(c);
            break;
        case LAYER_SHELL:
            ret_surface = get_wlrsurface(c);
            break;
    }
    return ret_surface;
}

struct wlr_surface *get_wlrsurface(struct client *c)
{
    if (!c)
        return NULL;
    switch (c->type) {
        case XDG_SHELL:
            return c->surface.xdg->surface;
            break;
        case LAYER_SHELL:
            return c->surface.layer->surface;
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            return c->surface.xwayland->surface;
        default:
            return NULL;
    }
}

static void unfocus_client(struct client *c)
{
    if (!c)
        return;

    switch (c->type) {
        case XDG_SHELL:
            wlr_xdg_toplevel_set_activated(c->surface.xdg, false);
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            {
                // unfocus x11 parent surface
                struct wlr_xwayland_surface *xwayland_surface = c->surface.xwayland;
                while (xwayland_surface->parent) {
                    xwayland_surface = xwayland_surface->parent;
                }
                wlr_xwayland_surface_activate(xwayland_surface, false);
            }
            break;
        default:
            break;
    }
}

void focus_surface(struct seat *seat, struct wlr_surface *surface)
{
    assert(surface != NULL);

    struct wlr_seat *wlr_seat = seat->wlr_seat;

    struct wlr_keyboard *kb = wlr_seat_get_keyboard(wlr_seat);
    // wlr_seat_get_keyboard can actually fail
    if (!kb)
        return;

    /* Have a client, so focus its top-level wlr_surface */
    wlr_seat_keyboard_notify_enter(wlr_seat, surface, kb->keycodes,
            kb->num_keycodes, &kb->modifiers);
}

void focus_client(struct seat *seat, struct client *old, struct client *c)
{
    struct wlr_surface *new_surface = get_base_wlrsurface(c);

    if (old) {
        struct wlr_surface *old_surface = get_wlrsurface(old);
        if (old_surface != new_surface) {
            cursor_constrain(seat->cursor, NULL);
            unfocus_client(old);
            struct container *old_con = old->con;
            container_damage_borders(old_con);
        }
    }

    /* Update wlroots'c keyboard focus */
    if (!c) {
        /* struct wlr_seat *wlr_seat = seat->wlr_seat; */
        wlr_seat_keyboard_notify_clear_focus(seat->wlr_seat);
        return;
    }
    // the surface shall only be focused if the container is visible on the
    // current monitor. Else the focus jumps left and right if we have multiple
    // montiors.
    if (!container_viewable_on_monitor(server_get_selected_monitor(), c->con)) {
        return;
    }

    /* Update wlroots'c keyboard focus */
    focus_surface(seat, get_wlrsurface(c));

    struct container *con = c->con;
    struct monitor *m = container_get_monitor(con);
    container_damage_borders_at_monitor(con, m);

    /* Activate the new client */
    switch (c->type) {
        case XDG_SHELL:
            wlr_xdg_toplevel_set_activated(c->surface.xdg, true);
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            wlr_xwayland_surface_activate(c->surface.xwayland, true);
            break;
        default:
            break;
    }
}

void container_move_sticky_containers_current_tag(struct container *con)
{
    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    container_move_sticky_containers(con, tag->id);
}

void container_move_sticky_containers(struct container *con, int tag_id)
{
    // // TODO: refactor this function
    // struct tag *ws = get_tag(tag_id);
    // if (!bitset_test(con->client->sticky_tags, ws->id)) {
    //     if (bitset_any(con->client->sticky_tags)) {
    //         container_set_just_tag_id(con, tag_id);
    //         arrange();
    //         focus_most_recent_container();
    //         ipc_event_tag();
    //     } else {
    //         move_to_scratchpad(con, 0);
    //         return;
    //     }
    //     return;
    // }
    // if (con->on_scratchpad) {
    //     return;
    // }
    //
    // if (tag_sticky_contains_client(ws, con->client)) {
    //     container_set_just_tag_id(con, ws->id);
    // } else if (bitset_none(con->client->sticky_tags)) {
    //     move_to_scratchpad(con, 0);
    // }
}

void client_setsticky(struct client *c, BitSet *tags)
{
    bitset_assign_bitset(&c->sticky_tags, tags);
    struct container *con = c->con;
    container_move_sticky_containers_current_tag(con);
    ipc_event_tag();
}

float calc_ratio(float width, float height)
{
    return height / width;
}

void kill_client(struct client *c)
{
    if (!c)
        return;

    switch (c->type) {
        case XDG_SHELL:
            wlr_xdg_toplevel_send_close(c->surface.xdg);
            break;
        case LAYER_SHELL:
            wlr_layer_surface_v1_destroy(c->surface.layer);
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            wlr_xwayland_surface_close(c->surface.xwayland);
            break;
    }
}

void client_handle_new_popup(struct wl_listener *listener, void *data)
{
    struct client *client = wl_container_of(listener, client, new_popup);
    struct wlr_xdg_popup *xdg_popup = data;

    struct container *con = client->con;
    struct monitor *m = container_get_monitor(con);
    create_popup(m, xdg_popup, container_get_current_geom(con), con);
}

void client_handle_set_title(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, set_title);
    const char *title;
    /* rule matching */
    switch (c->type) {
        case XDG_SHELL:
            title = c->surface.xdg->toplevel->title;
            break;
        case LAYER_SHELL:
            title = "";
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            title = c->surface.xwayland->title;
            break;
    }
    if (!title)
        title = "broken";

    c->title = title;

    struct tag *tag = container_get_tag(c->con);
    if (tag) {
        struct layout *lt = tag_get_layout(tag);
        apply_rules(lt->options->rules, c->con);
    }
}

void client_handle_set_app_id(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, set_app_id);
    const char *app_id;
    /* rule matching */
    switch (c->type) {
        case XDG_SHELL:
            if (c->surface.xdg->toplevel->app_id)
                app_id = c->surface.xdg->toplevel->app_id;
            break;
        case LAYER_SHELL:
            app_id = "";
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            app_id = c->surface.xwayland->class;
            break;
    }
    if (!app_id)
        app_id = "broken";

    c->app_id = app_id;

    struct tag *tag = container_get_tag(c->con);
    if (tag) {
        struct layout *lt = tag_get_layout(tag);
        apply_rules(lt->options->rules, c->con);
    }
}

void reset_floating_client_borders(int border_px)
{
    struct monitor *m = server_get_selected_monitor();
    for (int i = 0; i < server.container_stack->len; i++) {
        struct container *con = g_ptr_array_index(server.container_stack, i);
        if (container_is_tiled(con)) {
            continue;
        }
        if (!tagset_exist_on(m, con)) {
            continue;
        }
        container_set_border_width(con, direction_value_uniform(border_px));
        container_damage_whole(con);
    }
}
