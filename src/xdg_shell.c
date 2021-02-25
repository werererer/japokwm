#include "xdg_shell.h"

#include <stdlib.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>

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
