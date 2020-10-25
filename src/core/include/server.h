#ifndef SERVER_H
#define SERVER_H
#include <wayland-server.h>

/* enums */
enum cursorMode { CurNormal, CurMove, CurResize }; /* cursor */
struct server {
    struct wl_display *display;
    struct wlr_backend *backend;
    struct wlr_compositor *compositor;

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
