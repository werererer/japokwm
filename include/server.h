#ifndef SERVER_H
#define SERVER_H
#include <wayland-server.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/xcursor.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include "xwayland.h"

/* enums */
enum cursorMode { CurNormal, CURSOR_MOVE, CURSOR_RESIZE }; /* cursor */
struct server {
    /* associated with ilink in client */
    struct wl_list independents;

    struct wl_display *display;
    struct wl_event_loop *wl_event_loop;
    struct wlr_backend *backend;
    struct wlr_compositor *compositor;
    struct wlr_seat *seat;

    struct xwayland xwayland;
    struct wl_listener xwayland_ready;

    struct wlr_xdg_shell *xdgShell;
    struct wlr_layer_shell_v1 *layerShell;
    struct wlr_xdg_decoration_manager_v1 *xdecoMgr;

    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursorMgr;

    struct wl_list keyboards;
    enum cursorMode cursorMode;
};

extern struct server server;
#endif /* SERVER_H */
