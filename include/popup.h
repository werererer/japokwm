#ifndef POPUP_H
#define POPUP_H
#include <wayland-server.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_xdg_shell.h>
#include "client.h"

struct xdg_popup {
    struct wlr_xdg_popup *xdg;
    struct container *toplevel;
    struct wlr_box geom;
    struct wl_listener new_popup;
    struct wl_listener map;
    struct wl_listener destroy;
    struct wl_list plink;
    struct monitor *m;
};

void popup_handle_destroy(struct wl_listener *listener, void *data);
void popup_handle_new_popup(struct wl_listener *listener, void *data);
#endif /* POPUP_H */
