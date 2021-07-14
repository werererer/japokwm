#include "xwayland.h"

#include <stdlib.h>
#include <wlr/util/log.h>

#include "clipboard.h"
#include "container.h"
#include "server.h"
#include "tile/tileUtils.h"

static const char *atom_map[ATOM_LAST] = {
    "_NET_WM_WINDOW_TYPE_NORMAL",
    "_NET_WM_WINDOW_TYPE_DIALOG",
    "_NET_WM_WINDOW_TYPE_UTILITY",
    "_NET_WM_WINDOW_TYPE_TOOLBAR",
    "_NET_WM_WINDOW_TYPE_SPLASH",
    "_NET_WM_WINDOW_TYPE_MENU",
    "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
    "_NET_WM_WINDOW_TYPE_POPUP",
    "_NET_WM_WINDOW_TYPE_POPUP_MENU",
    "_NET_WM_WINDOW_TYPE_TOOLTIP",
    "_NET_WM_WINDOW_TYPE_NOTIFICATION",
    "_NET_WM_STATE_MODAL",
};

static void activatex11(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, activate);

    /* Only "managed" windows can be activated */
    if (c->type == X11_MANAGED)
        wlr_xwayland_surface_activate(c->surface.xwayland, true);
}

void create_notifyx11(struct wl_listener *listener, void *data)
{
    struct wlr_xwayland_surface *xwayland_surface = data;
    /* Allocate a Client for this surface */

    union surface_t surface;
    surface.xwayland = xwayland_surface;
    struct client *c = xwayland_surface->data = create_client(X11_MANAGED, surface);
    // set default value will be overriden on maprequest

    /* Listen to the various events it can emit */
    LISTEN(&xwayland_surface->events.map, &c->map, maprequestx11);
    LISTEN(&xwayland_surface->events.unmap, &c->unmap, unmap_notifyx11);
    LISTEN(&xwayland_surface->events.destroy, &c->destroy, destroy_notifyx11);
    LISTEN(&xwayland_surface->events.set_title, &c->set_title, client_handle_set_title);
    LISTEN(&xwayland_surface->events.request_activate, &c->activate, activatex11);

    create_container(c, selected_monitor, true);
}

void destroy_notifyx11(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, destroy);

    destroy_container(c->con);
    c->con = NULL;

    wl_list_remove(&c->map.link);
    wl_list_remove(&c->unmap.link);
    wl_list_remove(&c->destroy.link);
    wl_list_remove(&c->set_title.link);

    destroy_client(c);
}

void handle_xwayland_ready(struct wl_listener *listener, void *data)
{
    struct server *server =
        wl_container_of(listener, server, xwayland_ready);
    struct xwayland *xwayland = &server->xwayland;

    xcb_connection_t *xcb_conn = xcb_connect(NULL, NULL);
    int err = xcb_connection_has_error(xcb_conn);
    if (err) {
        wlr_log(WLR_ERROR, "XCB connect failed: %d", err);
        return;
    }

    xcb_intern_atom_cookie_t cookies[ATOM_LAST];
    for (size_t i = 0; i < ATOM_LAST; i++) {
        cookies[i] =
            xcb_intern_atom(xcb_conn, 0, strlen(atom_map[i]), atom_map[i]);
    }
    for (size_t i = 0; i < ATOM_LAST; i++) {
        xcb_generic_error_t *error = NULL;
        xcb_intern_atom_reply_t *reply =
            xcb_intern_atom_reply(xcb_conn, cookies[i], &error);
        if (reply != NULL && error == NULL) {
            xwayland->atoms[i] = reply->atom;
        }
        free(reply);

        if (error != NULL) {
            wlr_log(WLR_ERROR, "could not resolve atom %s, X11 error code %d",
                atom_map[i], error->error_code);
            free(error);
            break;
        }
    }

    xcb_disconnect(xcb_conn);
}

void unmap_notifyx11(struct wl_listener *listener, void *data)
{
    printf("unmap_notify\n");
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct client *c = wl_container_of(listener, c, unmap);

    wl_list_remove(&c->commit.link);

    struct container *con = c->con;
    container_damage_whole(c->con);
    remove_container_from_tile(con);

    remove_in_composed_list(&server.client_lists, cmp_ptr, c);

    arrange();
    struct monitor *m = selected_monitor;
    focus_most_recent_container(m->tagset, FOCUS_NOOP);
    printf("unmap_notify end\n");
}

void maprequestx11(struct wl_listener *listener, void *data)
{
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct client *c = wl_container_of(listener, c, map);
    struct wlr_xwayland_surface *xwayland_surface = c->surface.xwayland;
    struct monitor *m = selected_monitor;
    struct layout *lt = get_layout_in_monitor(m);

    c->type = xwayland_surface->override_redirect ? X11_UNMANAGED : X11_MANAGED;
    c->ws_id = m->tagset->selected_ws_id;
    c->bw = lt->options.tile_border_px;

    struct container *con = c->con;
    add_container_to_tile(con);
    LISTEN(&xwayland_surface->surface->events.commit, &c->commit, commit_notify);

    struct wlr_box prefered_geom = (struct wlr_box) {
        .x = c->surface.xwayland->x, 
        .y = c->surface.xwayland->y,
        .width = c->surface.xwayland->width,
        .height = c->surface.xwayland->height,
    };

    struct wlr_xwayland_surface_size_hints *size_hints = 
        c->surface.xwayland->size_hints;
    if (size_hints) {
        if (size_hints->width > MIN_CONTAINER_WIDTH && size_hints->height > MIN_CONTAINER_WIDTH) {
            prefered_geom = (struct wlr_box) {
                .x = size_hints->x,
                .y = size_hints->y,
                .width = size_hints->width,
                .height = size_hints->height,
            };
        }
    }

    struct wlr_box tmp;
    if (!wlr_box_intersection(&tmp, &m->geom, &prefered_geom)) {
        struct monitor *xym = xy_to_monitor(prefered_geom.x, prefered_geom.y);
        if (xym) {
            struct wlr_fbox rel_geom = get_relative_box(prefered_geom, xym->geom);
            prefered_geom = get_absolute_box(rel_geom, m->geom);
        }
    }

    switch (c->type) {
        case X11_MANAGED:
            {
                wlr_list_push(&server.normal_clients, c);

                con->on_top = false;
                if (wants_floating(con->client)) {
                    set_container_floating(con, fix_position, true);
                    resize(con, prefered_geom);
                }
                break;
            }
        case X11_UNMANAGED:
            {
                wlr_list_push(&server.independent_clients, c);

                struct workspace *ws = monitor_get_active_workspace(m);
                if (is_popup_menu(c) || xwayland_surface->parent) {
                    remove_in_composed_list(&ws->list_set->focus_stack_lists, cmp_ptr, con);
                    wlr_list_insert(&ws->list_set->focus_stack_normal, 1, con);
                } else {
                    con->on_top = true;
                    focus_container(con, FOCUS_NOOP);
                }

                con->has_border = false;
                lift_container(con);
                set_container_floating(con, NULL, true);
                resize(con, prefered_geom);
                break;
            }
        default:
            break;
    }
    arrange();
    struct container *sel = get_focused_container(m); 
    focus_container(sel, FOCUS_NOOP);
    wlr_xcursor_manager_set_cursor_image(server.cursor_mgr,
            "left_ptr", server.cursor.wlr_cursor);
}
