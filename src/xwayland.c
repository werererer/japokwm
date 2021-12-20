#include "xwayland.h"

#include <stdlib.h>
#include <wlr/util/log.h>

#include "client.h"
#include "container.h"
#include "monitor.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "seat.h"
#include "tag.h"
#include "list_sets/focus_stack_set.h"
#include "tagset.h"

#if JAPOKWM_HAS_XWAYLAND
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
#endif

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
    LISTEN(&xwayland_surface->events.set_class, &c->set_app_id, client_handle_set_app_id);
    LISTEN(&xwayland_surface->events.request_activate, &c->activate, activatex11);

    create_container(c, server_get_selected_monitor(), true);
}

void destroy_notifyx11(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, destroy);


    struct container *con = c->con;
    if (con->is_xwayland_popup) {
        g_ptr_array_remove(server.xwayland_popups, con);
    }
    destroy_container(c->con);

    wl_list_remove(&c->map.link);
    wl_list_remove(&c->unmap.link);
    wl_list_remove(&c->destroy.link);
    wl_list_remove(&c->set_title.link);
    wl_list_remove(&c->set_app_id.link);

    destroy_client(c);
}

void handle_xwayland_ready(struct wl_listener *listener, void *data)
{
#if JAPOKWM_HAS_XWAYLAND
    struct server *server =
        wl_container_of(listener, server, xwayland_ready);
    struct xwayland *xwayland = &server->xwayland;

    xcb_connection_t *xcb_conn = xcb_connect(NULL, NULL);
    int err = xcb_connection_has_error(xcb_conn);
    if (err) {
        debug_print("XCB connect failed: %d\n", err);
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
            debug_print("could not resolve atom %s, X11 error code %d\n",
                atom_map[i], error->error_code);
            free(error);
            break;
        }
    }

    xcb_disconnect(xcb_conn);
#endif
}

void unmap_notifyx11(struct wl_listener *listener, void *data)
{
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct client *c = wl_container_of(listener, c, unmap);

    wl_list_remove(&c->commit.link);

    struct container *con = c->con;
    container_damage_whole(c->con);
    remove_container_from_tile(con);

    arrange();
    focus_most_recent_container();
}

void maprequestx11(struct wl_listener *listener, void *data)
{
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct client *c = wl_container_of(listener, c, map);
    struct wlr_xwayland_surface *xwayland_surface = c->surface.xwayland;
    struct monitor *m = server_get_selected_monitor();

    c->type = xwayland_surface->override_redirect ? X11_UNMANAGED : X11_MANAGED;

    struct container *con = c->con;
    con->ws_id = m->ws_id;

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
                con->on_top = false;
                if (x11_wants_floating(con->client)) {
                    container_set_floating(con, container_fix_position, true);
                    container_set_floating_geom(con, &prefered_geom);
                }
                break;
            }
        case X11_UNMANAGED:
            {
                con->is_unmanaged = true;
                c->is_independent = true;

                debug_print("is unmanaged\n");
                struct tag *ws = monitor_get_active_workspace(m);
                if (x11_is_popup_menu(c) || xwayland_surface->parent) {
                    debug_print("is popup\n");
                    remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
                    g_ptr_array_insert(ws->focus_set->focus_stack_normal, 0, con);

                    con->is_xwayland_popup = true;
                    g_ptr_array_add(server.xwayland_popups, con);
                } else {
                    con->on_top = true;
                    focus_container(con);
                }

                con->has_border = false;
                lift_container(con);
                container_set_floating(con, NULL, true);
                container_set_floating_geom(con, &prefered_geom);
                break;
            }
        default:
            break;
    }

    arrange();
    struct container *sel = monitor_get_focused_container(m);
    focus_container(sel);
    struct seat *seat = input_manager_get_default_seat();
    wlr_xcursor_manager_set_cursor_image(seat->cursor->xcursor_mgr,
            "left_ptr", seat->cursor->wlr_cursor);
}

bool xwayland_popups_exist()
{
    return server.xwayland_popups->len > 0;
}

bool x11_wants_floating(struct client *c)
{
    if (c->type != X11_MANAGED && c->type != X11_UNMANAGED)
        return false;

    struct wlr_xwayland_surface *surface = c->surface.xwayland;
    if (surface->modal)
        return true;

#if JAPOKWM_HAS_XWAYLAND
    struct xwayland xwayland = server.xwayland;
    for (size_t i = 0; i < surface->window_type_len; ++i) {
        xcb_atom_t type = surface->window_type[i];
        if (type == xwayland.atoms[NET_WM_WINDOW_TYPE_DIALOG] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_UTILITY] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_TOOLBAR] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_POPUP_MENU] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_SPLASH]) {
            return true;
        }
    }
#endif

    struct wlr_xwayland_surface_size_hints *size_hints = surface->size_hints;
    if (size_hints != NULL &&
            size_hints->min_width > 0 && size_hints->min_height > 0 &&
            (size_hints->max_width == size_hints->min_width ||
            size_hints->max_height == size_hints->min_height)) {
        return true;
    }

    return false;
}

bool x11_is_popup_menu(struct client *c)
{
#if JAPOKWM_HAS_XWAYLAND
    struct wlr_xwayland_surface *surface = c->surface.xwayland;
    struct xwayland xwayland = server.xwayland;
    for (size_t i = 0; i < surface->window_type_len; ++i) {
        xcb_atom_t type = surface->window_type[i];
        if (type == xwayland.atoms[NET_WM_WINDOW_TYPE_POPUP_MENU] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_POPUP] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_MENU] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_NORMAL]) {
            return true;
        }
    }
#endif
    return false;
}


void init_xwayland(struct wl_display *display, struct seat *seat)
{
#if JAPOKWM_HAS_XWAYLAND
    /*
     * Initialise the XWayland X server.
     * It will be started when the first X client is started.
     */
    server.xwayland.wlr_xwayland = wlr_xwayland_create(server.wl_display,
            server.compositor, true);
    if (server.xwayland.wlr_xwayland) {
        LISTEN(&server.xwayland.wlr_xwayland->events.ready, &server.xwayland_ready, handle_xwayland_ready);
        LISTEN(&server.xwayland.wlr_xwayland->events.new_surface, &server.new_xwayland_surface, create_notifyx11);
        wlr_xwayland_set_seat(server.xwayland.wlr_xwayland, seat->wlr_seat);

        setenv("DISPLAY", server.xwayland.wlr_xwayland->display_name, true);
    } else {
        printf("failed to setup XWayland X server, continuing without it");
        unsetenv("DISPLAY");
    }
#endif
}
