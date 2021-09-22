#ifndef XWAYLAND_H
#define XWAYLAND_H

#include <wlr/xwayland.h>
#include <xcb/xproto.h>

struct client;
struct seat;

enum atom_name {
    NET_WM_WINDOW_TYPE_NORMAL,
    NET_WM_WINDOW_TYPE_DIALOG,
    NET_WM_WINDOW_TYPE_UTILITY,
    NET_WM_WINDOW_TYPE_TOOLBAR,
    NET_WM_WINDOW_TYPE_SPLASH,
    NET_WM_WINDOW_TYPE_MENU,
    NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
    NET_WM_WINDOW_TYPE_POPUP,
    NET_WM_WINDOW_TYPE_POPUP_MENU,
    NET_WM_WINDOW_TYPE_TOOLTIP,
    NET_WM_WINDOW_TYPE_NOTIFICATION,
    NET_WM_STATE_MODAL,
    ATOM_LAST,
};

struct xwayland {
    struct wlr_xwayland *wlr_xwayland;
    struct wlr_xcursor_manager *xcursor_manager;

    xcb_atom_t atoms[ATOM_LAST];
};

void create_notifyx11(struct wl_listener *listener, void *data);
void handle_xwayland_ready(struct wl_listener *listener, void *data);
void maprequestx11(struct wl_listener *listener, void *data);
void unmap_notifyx11(struct wl_listener *listener, void *data);
void destroy_notifyx11(struct wl_listener *listener, void *data);

bool xwayland_popups_exist();

bool x11_wants_floating(struct client *c);
bool x11_is_popup_menu(struct client *c);
void init_xwayland(struct wl_display *display, struct seat *seat);

#endif /* XWAYLAND_H */
