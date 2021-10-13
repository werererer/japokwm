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
struct xdg_popup *create_popup(struct monitor *m, struct wlr_xdg_popup *xdg_popup,
        struct wlr_box *parent_geom, struct container* toplevel);
static void destroy_popup(struct xdg_popup *xdg_popup);
static void popup_handle_commit(struct wl_listener *listener, void *data);
static void popup_handle_map(struct wl_listener *listener, void *data);
static void popup_handle_unmap(struct wl_listener *listener, void *data);
static void popup_damage(struct xdg_popup *xdg_popup, bool whole);

struct xdg_popup *create_popup(struct monitor *m, struct wlr_xdg_popup *xdg_popup,
        struct wlr_box *parent_geom, struct container* toplevel)
{
    struct xdg_popup *popup = xdg_popup->base->data = calloc(1, sizeof(*popup));
    popup->xdg = xdg_popup;
    popup->toplevel = toplevel;

    struct wlr_box box = popup->xdg->geometry;
    box.x = 0;
    box.y = 0;
    box.width = m->geom.width;
    box.height = m->geom.height;

    wlr_xdg_popup_unconstrain_from_box(popup->xdg, &box);
    // the root window may be resized. This must be adjusted
    popup->geom.x = popup->xdg->geometry.x + parent_geom->x;
    popup->geom.y = popup->xdg->geometry.y + parent_geom->y;
    popup->geom.width = popup->xdg->geometry.width;
    popup->geom.height = popup->xdg->geometry.height;
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
    popup_damage(popup, true);

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
            &parent_popup->geom, parent_popup->toplevel);
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
    struct wlr_box *con_geom = container_get_current_geom(con);
    switch (con->client->type) {
        case XDG_SHELL:
            surface = wlr_xdg_surface_surface_at(
                    con->client->surface.xdg,
                    /* absolute mouse position to relative in regards to
                     * the client */
                    absolute_x_to_container_relative(con_geom, cursorx),
                    absolute_y_to_container_relative(con_geom, cursory),
                    sx, sy);
            break;
        case LAYER_SHELL:
            surface = wlr_layer_surface_v1_surface_at(
                    con->client->surface.layer,
                    absolute_x_to_container_relative(con_geom, cursorx),
                    absolute_y_to_container_relative(con_geom, cursory),
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

    wlr_xdg_popup_destroy(popup->xdg->base);
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
