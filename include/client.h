#ifndef CLIENT
#define CLIENT
#include <X11/Xlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/xwayland.h>

#include "parseConfig.h"

enum shell { XDG_SHELL, X11_MANAGED, X11_UNMANAGED, LAYER_SHELL }; /* client types */

struct client {
    /* clients */
    struct wl_list link;
    /* independents list */
    struct wl_list ilink;

    float ratio;
    /* containers containing this client */
    struct wl_list containers;
    union {
        struct wlr_xdg_surface *xdg;
        struct wlr_layer_surface_v1 *layer;
        struct wlr_xwayland_surface *xwayland;
    } surface;
    struct wl_listener activate;
    struct wl_listener commit;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener new_popup;
    int bw;

    enum shell type;
    int id;
    char *title;
    uint32_t resize; /* configure serial of a pending resize */
    struct workspace *ws;
};

/* it ignores bool  hiding which visibleon doesn't */
void focus_client(struct client *old, struct client *c);
float calc_ratio(float width, float height);

extern struct wl_list clients; /* tiling order */

struct wlr_surface *get_wlrsurface(struct client *c);
#endif
