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
#include "container.h"

struct wl_list popups;

static void popup_handle_new_subpopup(struct wl_listener *listener, void *data);
static struct xdg_popup *create_popup(struct wlr_xdg_popup *xdg_popup,
        struct wlr_box parent_geom, struct container* toplevel);
static void destroy_popup(struct xdg_popup *xdg_popup);

static struct xdg_popup *create_popup(struct wlr_xdg_popup *xdg_popup,
        struct wlr_box parent_geom, struct container* toplevel)
{
    struct xdg_popup *popup = xdg_popup->base->data =
        calloc(1, sizeof(struct xdg_popup));
    popup->xdg = xdg_popup;
    popup->toplevel = toplevel;

    // unconstrain
    struct wlr_box *box = &selected_monitor->geom;
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

static void destroy_popup(struct xdg_popup *xdg_popup)
{
    free(xdg_popup);
    xdg_popup = NULL;
}

void popup_handle_new_popup(struct wl_listener *listener, void *data)
{
    printf("new popup\n");
    struct client *c = wl_container_of(listener, c, new_popup);
    struct wlr_xdg_popup *xdg_popup = data;

    struct monitor *m;
    wl_list_for_each(m, &mons, link) {
        struct container *con;
        wl_list_for_each(con, &m->stack, slink) {
            if (con->client != c)
                continue;
            struct xdg_popup *popup = create_popup(xdg_popup, con->geom, con);
            wl_list_insert(&popups, &popup->plink);
        }
    }
}

static void popup_handle_new_subpopup(struct wl_listener *listener, void *data)
{
    printf("new popup\n");
    struct xdg_popup *parentPopup =
        wl_container_of(listener, parentPopup, new_popup);
    struct wlr_xdg_popup *xdg_popup = data;

    struct xdg_popup *popup =
        create_popup(xdg_popup, parentPopup->geom, parentPopup->toplevel);
    wl_list_insert(&popups, &popup->plink);
}

void popup_handle_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, destroy);
    wl_list_remove(&popup->plink);
    destroy_popup(popup);
}
