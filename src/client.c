#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <wayland-util.h>
#include <string.h>
#include <assert.h>

#include "container.h"
#include "popup.h"
#include "server.h"
#include "tile/tile.h"
#include "tile/tileUtils.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "ipc-server.h"

struct client *create_client(enum shell shell_type, union surface_t surface)
{
    struct client *c = calloc(1, sizeof(struct client));

    c->type = shell_type;
    c->surface = surface;

    return c;
}

void destroy_client(struct client *c)
{
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
                while (xwayland_surface->parent)
                    xwayland_surface = xwayland_surface->parent;
                wlr_xwayland_surface_activate(xwayland_surface, false);
            }
            break;
        default:
            break;
    }
}

void focus_client(struct client *old, struct client *c)
{
    struct wlr_keyboard *kb = wlr_seat_get_keyboard(server.seat);

    struct wlr_surface *old_surface = get_base_wlrsurface(old);
    struct wlr_surface *new_surface = get_base_wlrsurface(c);
    if (old_surface != new_surface) {
        unfocus_client(old);
    }

    /* Update wlroots'c keyboard focus */
    if (!c) {
        /* With no client, all we have left is to clear focus */
        wlr_seat_keyboard_notify_clear_focus(server.seat);
        return;
    }

    /* Have a client, so focus its top-level wlr_surface */
    wlr_seat_keyboard_notify_enter(server.seat, get_wlrsurface(c), kb->keycodes,
            kb->num_keycodes, &kb->modifiers);

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

void client_setsticky(struct client *c, bool sticky)
{
    c->sticky = sticky;
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
            wlr_layer_surface_v1_close(c->surface.layer);
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            wlr_xwayland_surface_close(c->surface.xwayland);
            break;
    }
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
            title = "test";
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            title = c->surface.xwayland->title;
            break;
    }
    if (!title)
        title = "broken";

    c->title = title;
    ipc_event_window();
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
            app_id = "test";
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            app_id = c->surface.xwayland->class;
            break;
    }
    if (!app_id)
        app_id = "broken";

    c->app_id = app_id;
}

void reset_tiled_client_borders(int border_px)
{
    for (int i = 0; i < server.normal_clients.length; i++) {
        struct client *c = server.normal_clients.items[i];
        struct tagset *tagset = selected_monitor->tagset;
        if (!exist_on(tagset, c->con))
            continue;
        if (c->con->floating)
            continue;
        c->bw = border_px;
    }
}

void reset_floating_client_borders(int border_px)
{
    for (int i = 0; i < server.normal_clients.length; i++) {
        struct client *c = server.normal_clients.items[i];
        struct tagset *tagset = selected_monitor->tagset;
        if (!exist_on(tagset, c->con))
            continue;
        if (!c->con->floating)
            continue;
        c->bw = border_px;
    }
}

bool wants_floating(struct client *c)
{
    if (c->type != X11_MANAGED && c->type != X11_UNMANAGED)
        return false;

    struct wlr_xwayland_surface *surface = c->surface.xwayland;
    struct xwayland xwayland = server.xwayland;

    if (surface->modal)
        return true;

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

    struct wlr_xwayland_surface_size_hints *size_hints = surface->size_hints;
    if (size_hints != NULL &&
            size_hints->min_width > 0 && size_hints->min_height > 0 &&
            (size_hints->max_width == size_hints->min_width ||
            size_hints->max_height == size_hints->min_height)) {
        return true;
    }

    return false;
}

bool is_popup_menu(struct client *c)
{
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
    return false;
}

void destroy_notify(struct wl_listener *listener, void *data)
{
    /* Called when the surface is destroyed and should never be shown again. */
    struct client *c = wl_container_of(listener, c, destroy);

    wl_list_remove(&c->map.link);
    wl_list_remove(&c->unmap.link);
    wl_list_remove(&c->destroy.link);

    switch (c->type) {
        case LAYER_SHELL:
            wl_list_remove(&c->new_popup.link);
            break;
        case XDG_SHELL:
            wl_list_remove(&c->new_popup.link);
            wl_list_remove(&c->set_title.link);
            wl_list_remove(&c->set_app_id.link);
            break;
        case X11_MANAGED:
            wl_list_remove(&c->set_title.link);
        default:
            break;
    }

    destroy_client(c);
}

void maprequest(struct wl_listener *listener, void *data)
{
    printf("map\n");
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct client *c = wl_container_of(listener, c, map);

    struct monitor *m = selected_monitor;
    if (c->type == LAYER_SHELL) {
        m = output_to_monitor(c->surface.layer->output);
    }
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = ws->layout;

    c->ws_id = ws->id;
    c->bw = lt->options.tile_border_px;

    switch (c->type) {
        case LAYER_SHELL:
            wlr_list_push(&server.non_tiled_clients, c);
            break;
        default:
            wlr_list_push(&server.normal_clients, c);
    }
    if (c->type != LAYER_SHELL) {
        create_container(c, m, true);
    }

    arrange();
    focus_most_recent_container(m->tagset, FOCUS_NOOP);
}

void unmap_notify(struct wl_listener *listener, void *data)
{
    printf("unmap_notify\n");
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct client *c = wl_container_of(listener, c, unmap);

    container_damage_whole(c->con);
    destroy_container(c->con);
    c->con = NULL;

    remove_in_composed_list(&server.client_lists, cmp_ptr, c);

    arrange();
    struct monitor *m = selected_monitor;
    focus_most_recent_container(m->tagset, FOCUS_NOOP);
    printf("unmap_notify end\n");
}
