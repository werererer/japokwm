#include "popup.h"
#include "client.h"
#include <stdio.h>
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wayland-server.h>
#include "root.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "xdg-shell-protocol.h"
#include "tile/tileUtils.h"

struct wl_list popups;

static void popup_handle_new_subpopup(struct wl_listener *listener, void *data);

static struct xdg_popup *create_popup(struct wlr_xdg_popup *xdg_popup,
        struct wlr_box parent_geom, struct client* toplevel)
{
    struct xdg_popup *popup = xdg_popup->base->data =
        calloc(1, sizeof(struct xdg_popup));
    popup->xdg = xdg_popup;
    popup->toplevel = toplevel;

    // unconstrain
    struct wlr_box *box = &selected_monitor->m;
    box->x = -popup->toplevel->geom.x;
    box->y = -popup->toplevel->geom.y;
    wlr_xdg_popup_unconstrain_from_box(popup->xdg, box);

    // the root window may be resized. This must be adjusted
    popup->geom.x = popup->xdg->geometry.x + parent_geom.x;
    popup->geom.y = popup->xdg->geometry.y + parent_geom.y;
    popup->geom.width = popup->xdg->geometry.width;
    popup->geom.height = popup->xdg->geometry.height;

    popup->new_popup.notify = popup_handle_new_subpopup;
    wl_signal_add(&popup->xdg->base->events.new_popup, &popup->new_popup);
    popup->destroy.notify = popup_handle_destroy;
    wl_signal_add(&popup->xdg->base->events.destroy, &popup->destroy);
    return popup;
}

void popup_handle_new_popup(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, new_popup);
    struct wlr_xdg_popup *xdg_popup = data;

    struct xdg_popup *popup =
        create_popup(xdg_popup, get_absolute_box(selected_monitor->m, c->geom), c);

    wl_list_insert(&popups, &popup->link);
}

static void popup_handle_new_subpopup(struct wl_listener *listener, void *data)
{
    struct xdg_popup *parentPopup =
        wl_container_of(listener, parentPopup, new_popup);
    struct wlr_xdg_popup *xdg_popup = data;

    struct xdg_popup *popup =
        create_popup(xdg_popup, parentPopup->geom, parentPopup->toplevel);
    wl_list_insert(&popups, &popup->link);
}

void popup_handle_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, destroy);
    wl_list_remove(&popup->link);
    free(popup);
}
