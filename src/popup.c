#include "popup.h"

#include <stdio.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "client.h"
#include "container.h"
#include "monitor.h"
#include "render/render.h"
#include "root.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "xdg-shell-protocol.h"

static void popup_handle_new_popup(struct wl_listener *listener, void *data);
static void popup_handle_new_subpopup(struct wl_listener *listener, void *data);
static void destroy_popup(struct xdg_popup *xdg_popup);
static void popup_handle_commit(struct wl_listener *listener, void *data);
static void popup_handle_map(struct wl_listener *listener, void *data);
static void popup_handle_unmap(struct wl_listener *listener, void *data);
static void popup_damage(struct xdg_popup *xdg_popup, bool whole);

struct xdg_popup *create_popup(struct monitor *m, struct wlr_xdg_popup *xdg_popup,
        void *parent, struct container* toplevel)
{
    struct xdg_popup *popup = xdg_popup->base->data = calloc(1, sizeof(*popup));
    popup->xdg = xdg_popup;
    popup->toplevel = toplevel;
    popup->parent = parent;

    // TODO: what does this exactly do?
    // struct wlr_box output_geom = monitor_get_active_geom(m);
    // wlr_xdg_popup_unconstrain_from_box(xdg_popup, &output_geom);

    // the root window may be resized. This must be adjusted
    popup->m = m;

    LISTEN(&popup->xdg->base->events.map, &popup->map, popup_handle_map);
    LISTEN(&popup->xdg->base->events.unmap, &popup->unmap, popup_handle_unmap);
    LISTEN(&popup->xdg->base->events.new_popup, &popup->new_popup, popup_handle_new_popup);
    LISTEN(&popup->xdg->base->events.destroy, &popup->destroy, popup_handle_destroy);

    g_ptr_array_insert(server.popups, 0, popup);
    return popup;
}

static void destroy_popup(struct xdg_popup *xdg_popup)
{
    wl_list_remove(&xdg_popup->map.link);
    wl_list_remove(&xdg_popup->unmap.link);
    wl_list_remove(&xdg_popup->destroy.link);
    wl_list_remove(&xdg_popup->new_popup.link);

    free(xdg_popup);
    xdg_popup = NULL;
}

static void popup_handle_commit(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, commit);
    popup_damage(popup, false);
}

static void popup_handle_map(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, map);

    struct wlr_xdg_popup *xdg_popup = popup->xdg;

    double sx, sy;
    wlr_xdg_popup_get_position(xdg_popup, &sx, &sy);

    int offset_x = 0;
    int offset_y = 0;

    for (struct xdg_popup *curr_popup = popup;
            curr_popup->parent != curr_popup->toplevel;
            curr_popup = curr_popup->parent) {
        struct xdg_popup *parent_popup = curr_popup->parent;
        double sx, sy;
        wlr_xdg_popup_get_position(parent_popup->xdg, &sx, &sy);
        offset_x += sx;
        offset_y += sy;
    }
    struct container *toplevel = popup->toplevel;
    struct wlr_box toplevel_geom = container_get_current_content_geom(toplevel);

    offset_x += toplevel_geom.x;
    offset_y += toplevel_geom.y;


    // the root window may be resized. This must be adjusted
    popup_set_x(popup, offset_x + sx);
    popup_set_y(popup, offset_y + sy);
    popup_set_width(popup, xdg_popup->current.geometry.width);
    popup_set_height(popup, xdg_popup->current.geometry.height);

    // TODO: either use this or use actual xdg constraints
    struct monitor *m = server_get_selected_monitor();
    struct wlr_box output_geom = monitor_get_active_geom(m);
    apply_bounds(&popup->geom, output_geom);

    popup->commit.notify = popup_handle_commit;
    wl_signal_add(&popup->xdg->base->surface->events.commit, &popup->commit);
}

static void popup_handle_unmap(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, unmap);
    wl_list_remove(&popup->commit.link);
    popup_damage(popup, true);
}

static void popup_damage(struct xdg_popup *xdg_popup, bool whole)
{
    if (!xdg_popup)
        return;

    struct wlr_xdg_popup *popup = xdg_popup->xdg;
    struct wlr_surface *surface = popup->base->surface;
    struct monitor *m = xdg_popup->m;

    output_damage_surface(m, surface, &xdg_popup->geom, whole);
}

static void popup_handle_new_popup(struct wl_listener *listener, void *data)
{
    struct xdg_popup *parent_popup =
        wl_container_of(listener, parent_popup, new_popup);
    struct wlr_xdg_popup *xdg_popup = data;

    create_popup(parent_popup->m, xdg_popup,
            parent_popup, parent_popup->toplevel);
}

void popup_handle_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, destroy);
    list_remove(server.popups, cmp_ptr, popup);

    destroy_popup(popup);
}

struct wlr_surface *get_popup_surface_under_cursor(struct cursor *cursor, double *sx, double *sy)
{
    int cursorx = cursor->wlr_cursor->x;
    int cursory = cursor->wlr_cursor->y;

    if (!popups_exist())
        return NULL;

    struct xdg_popup *popup = get_latest_popup();
    if (!popup)
        return NULL;
    struct container *con = popup->toplevel;
    if (!con)
        return NULL;

    struct wlr_surface *surface = NULL;
    struct wlr_box con_geom = container_get_current_geom(con);

    switch (con->client->type) {
        case XDG_SHELL:
            surface = wlr_xdg_surface_surface_at(
                    con->client->surface.xdg,
                    /* absolute mouse position to relative in regards to
                     * the client */
                    absolute_x_to_container_local(con_geom, cursorx),
                    absolute_y_to_container_local(con_geom, cursory),
                    sx, sy);
            break;
        case LAYER_SHELL:
            surface = wlr_layer_surface_v1_surface_at(
                    con->client->surface.layer,
                    absolute_x_to_container_local(con_geom, cursorx),
                    absolute_y_to_container_local(con_geom, cursory),
                    sx, sy);
            break;
        default:
            break;
    }
    return surface;
}

inline void destroy_popups()
{
    struct xdg_popup *popup = get_latest_popup();

    if (!popup)
        return;

    wlr_xdg_popup_destroy(popup->xdg);
}

inline struct xdg_popup *get_latest_popup()
{
    if (!popups_exist())
        return NULL;

    struct xdg_popup *popup = g_ptr_array_index(server.popups, 0);
    return popup;
}


inline bool popups_exist()
{
    return server.popups->len > 0;
}

void popup_set_x(struct xdg_popup *popup, int x)
{
    popup->geom.x = x;
}

void popup_set_y(struct xdg_popup *popup, int y)
{
    popup->geom.y = y;
}

void popup_set_width(struct xdg_popup *popup, int width)
{
    popup->geom.width = width;
}

void popup_set_height(struct xdg_popup *popup, int height)
{
    popup->geom.height = height;
}
