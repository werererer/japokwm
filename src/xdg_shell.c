#include "xdg_shell.h"

#include <stdlib.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/util/edges.h>

#include "client.h"
#include "popup.h"
#include "utils/coreUtils.h"
#include "container.h"
#include "tile/tileUtils.h"

static void destroyxdeco(struct wl_listener *listener, void *data);
static void getxdecomode(struct wl_listener *listener, void *data);

void destroyxdeco(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_toplevel_decoration_v1 *wlr_deco = data;
    Decoration *d = wlr_deco->data;

    wl_list_remove(&d->destroy.link);
    wl_list_remove(&d->request_mode.link);
    free(d);
}

static void getxdecomode(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_toplevel_decoration_v1 *wlr_deco = data;
    wlr_xdg_toplevel_decoration_v1_set_mode(wlr_deco,
            WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}

void create_notify_xdg(struct wl_listener *listener, void *data)
{
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
     * client, either a toplevel (application window) or popup. */
    struct wlr_xdg_surface *xdg_surface = data;


    if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL)
        return;

    union surface_t surface;
    surface.xdg = xdg_surface;
    /* Allocate a Client for this surface */
    struct client *c = xdg_surface->data = create_client(XDG_SHELL, surface);

    /* Tell the client not to try anything fancy */
    wlr_xdg_toplevel_set_tiled(c->surface.xdg, WLR_EDGE_TOP |
            WLR_EDGE_BOTTOM | WLR_EDGE_LEFT | WLR_EDGE_RIGHT);

    /* Listen to the various events it can emit */
    LISTEN(&xdg_surface->events.map, &c->map, maprequest);
    LISTEN(&xdg_surface->surface->events.commit, &c->commit, commit_notify);
    LISTEN(&xdg_surface->events.unmap, &c->unmap, unmap_notify);
    LISTEN(&xdg_surface->events.destroy, &c->destroy, destroy_notify);

    LISTEN(&xdg_surface->toplevel->events.set_title, &c->set_title, client_handle_set_title);
    LISTEN(&xdg_surface->toplevel->events.set_app_id, &c->set_app_id, client_handle_set_app_id);

    LISTEN(&xdg_surface->events.new_popup, &c->new_popup, popup_handle_new_popup);

    create_container(c, selected_monitor, true);
}

void createxdeco(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_toplevel_decoration_v1 *wlr_deco = data;
    Decoration *d = wlr_deco->data = calloc(1, sizeof(*d));

    wl_signal_add(&wlr_deco->events.request_mode, &d->request_mode);
    d->request_mode.notify = getxdecomode;
    wl_signal_add(&wlr_deco->events.destroy, &d->destroy);
    d->destroy.notify = destroyxdeco;

    getxdecomode(&d->request_mode, wlr_deco);
}
