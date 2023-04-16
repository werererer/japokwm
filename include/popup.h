#ifndef POPUP_H
#define POPUP_H
#include <wayland-server.h>
#include <wlr/util/box.h>
#include <wlr/types/wlr_xdg_shell.h>
#include "client.h"

struct xdg_popup {
    struct wlr_xdg_popup *xdg;
    struct container *toplevel;
    struct wlr_box geom;
    struct monitor *m;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener commit;
    struct wl_listener new_popup;
};

struct xdg_popup *create_popup(struct monitor *m, struct wlr_xdg_popup *xdg_popup,
        struct wlr_box parent_geom, struct container* toplevel);
void popup_handle_destroy(struct wl_listener *listener, void *data);
void destroy_popups();
struct wlr_surface *get_popup_surface_under_cursor(struct cursor *cursor, double *sx, double *sy);
struct xdg_popup *get_latest_popup();
bool popups_exist();

void popup_set_x(struct xdg_popup *popup, int x);
void popup_set_y(struct xdg_popup *popup, int y);
void popup_set_width(struct xdg_popup *popup, int width);
void popup_set_height(struct xdg_popup *popup, int height);
#endif /* POPUP_H */
