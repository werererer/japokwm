#include "xwayland.h"

#include <stdlib.h>
#include <wlr/util/log.h>

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

void create_notifyx11(struct wl_listener *listener, void *data)
{
    struct wlr_xwayland_surface *xwayland_surface = data;
    struct client *c;
    /* Allocate a Client for this surface */
    c = xwayland_surface->data = calloc(1, sizeof(struct client));
    c->surface.xwayland = xwayland_surface;
    // set default value will be overriden on maprequest
    c->type = X11_MANAGED;

    /* Listen to the various events it can emit */
    c->map.notify = maprequestx11;
    wl_signal_add(&xwayland_surface->events.map, &c->map);
    c->unmap.notify = unmapnotify;
    wl_signal_add(&xwayland_surface->events.unmap, &c->unmap);
    c->activate.notify = activatex11;
    wl_signal_add(&xwayland_surface->events.request_activate, &c->activate);
    c->destroy.notify = destroynotify;
    wl_signal_add(&xwayland_surface->events.destroy, &c->destroy);
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

void activatex11(struct wl_listener *listener, void *data)
{
       struct client *c = wl_container_of(listener, c, activate);

       /* Only "managed" windows can be activated */
       if (c->type == X11_MANAGED)
           wlr_xwayland_surface_activate(c->surface.xwayland, true);
}

void maprequestx11(struct wl_listener *listener, void *data)
{
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct client *c = wl_container_of(listener, c, map);
    struct wlr_xwayland_surface *xwayland_surface = c->surface.xwayland;
    struct monitor *m = selected_monitor;
    struct layout *lt = get_layout_on_monitor(m);

    c->commit.notify = commit_notify;
    wl_signal_add(&xwayland_surface->surface->events.commit, &c->commit);

    c->type = xwayland_surface->override_redirect ? X11_UNMANAGED : X11_MANAGED;
    c->ws_id = m->ws_ids[0];
    c->bw = lt->options.tile_border_px;

    struct container *con = create_container(c, m, true);

    struct wlr_box prefered_geom = (struct wlr_box) {
        .x = c->surface.xwayland->x, 
        .y = c->surface.xwayland->y,
        .width = c->surface.xwayland->width,
        .height = c->surface.xwayland->height,
    };
    struct wlr_xwayland_surface_size_hints *size_hints = 
        c->surface.xwayland->size_hints;
    if (size_hints) {
        prefered_geom = (struct wlr_box) {
            .x = size_hints->x,
            .y = size_hints->y,
            .width = size_hints->width,
            .height = size_hints->height,
        };
    }

    /* if (prefered_geom.width <= MIN_CONTAINER_WIDTH || */
    /*         prefered_geom.height <= MIN_CONTAINER_HEIGHT) { */
    /* } */

    switch (c->type) {
        case X11_MANAGED:
            {
                wl_list_insert(&clients, &c->link);
                printf("X11_MANAGED\n");

                con->on_top = false;
                if (wants_floating(con->client)) {
                    set_container_floating(con, true);
                    resize(con, prefered_geom);
                }
                break;
            }
        case X11_UNMANAGED:
            {
                wl_list_insert(&server.independents, &con->ilink);
                printf("X11_UNMANAGED\n");

                if (is_popup_menu(c) || xwayland_surface->parent) {
                    wl_list_remove(&con->flink);
                    wl_list_insert(&focused_container(m)->flink, &con->flink);
                } else {
                    con->on_top = true;
                    focus_container(con, FOCUS_NOOP);
                }

                con->has_border = false;
                lift_container(con);
                set_container_floating(con, true);
                resize(con, prefered_geom);
                break;
            }
        default:
            break;
    }
    arrange();
    apply_rules(con);
}
