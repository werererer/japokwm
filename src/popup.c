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

static void popup_handle_new_subpopup(struct wl_listener *listener, void *data);
static struct xdg_popup *create_popup(struct monitor *m, struct wlr_xdg_popup *xdg_popup,
        struct wlr_box parent_geom, struct container* toplevel);
static void destroy_popup(struct xdg_popup *xdg_popup);
static void popup_handle_commit(struct wl_listener *listener, void *data);
static void popup_damage(struct xdg_popup *xdg_popup, bool whole);

static struct xdg_popup *create_popup(struct monitor *m, struct wlr_xdg_popup *xdg_popup,
        struct wlr_box parent_geom, struct container* toplevel)
{
    struct xdg_popup *popup = xdg_popup->base->data =
        calloc(1, sizeof(struct xdg_popup));
    popup->xdg = xdg_popup;
    popup->toplevel = toplevel;

    // unconstrain
    struct wlr_box box = popup->xdg->geometry;
    box.x = 0;
    box.y = 0;
    box.width = m->geom.width;
    box.height = m->geom.height;
    wlr_xdg_popup_unconstrain_from_box(popup->xdg, &box);

    // the root window may be resized. This must be adjusted
    popup->geom.x = popup->xdg->geometry.x + parent_geom.x;
    popup->geom.y = popup->xdg->geometry.y + parent_geom.y;
    popup->geom.width = popup->xdg->geometry.width;
    popup->geom.height = popup->xdg->geometry.height;
    popup->m = m;

    popup->commit.notify = popup_handle_commit;
    wl_signal_add(&popup->xdg->base->surface->events.commit, &popup->commit);
    popup->new_popup.notify = popup_handle_new_subpopup;
    wl_signal_add(&popup->xdg->base->events.new_popup, &popup->new_popup);
    popup->destroy.notify = popup_handle_destroy;
    wl_signal_add(&popup->xdg->base->events.destroy, &popup->destroy);
    return popup;
}

static void destroy_popup(struct xdg_popup *xdg_popup)
{
    free(xdg_popup);
    xdg_popup = NULL;
}

static void popup_handle_commit(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, commit);
    popup_damage(popup, false);
}

static void popup_damage(struct xdg_popup *xdg_popup, bool whole)
{
    struct wlr_xdg_popup *popup = xdg_popup->xdg;
    struct wlr_surface *surface = popup->base->surface;
    int popup_sx = popup->geometry.x - popup->base->geometry.x;
    int popup_sy = popup->geometry.y - popup->base->geometry.y;
    int ox = popup_sx, oy = popup_sy;
    struct monitor *m = xdg_popup->m;
    output_damage_surface(m, surface, ox, oy, whole);
}

void popup_handle_new_popup(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, new_popup);
    struct wlr_xdg_popup *xdg_popup = data;

    struct container *con = c->con;
    if (con->m != selected_monitor)
        return;
    struct xdg_popup *popup = create_popup(con->m, xdg_popup, con->geom, con);
    wl_list_insert(&popups, &popup->plink);
}

static void popup_handle_new_subpopup(struct wl_listener *listener, void *data)
{
    printf("new popup\n");
    struct xdg_popup *parent_popup =
        wl_container_of(listener, parent_popup, new_popup);
    struct wlr_xdg_popup *xdg_popup = data;

    struct xdg_popup *popup = create_popup(parent_popup->m, xdg_popup,
            parent_popup->geom, parent_popup->toplevel);
    wl_list_insert(&popups, &popup->plink);
}

void popup_handle_destroy(struct wl_listener *listener, void *data)
{
    printf("destroy popup\n");
    struct xdg_popup *popup = wl_container_of(listener, popup, destroy);
    wl_list_remove(&popup->plink);
    destroy_popup(popup);
}
