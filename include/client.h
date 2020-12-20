#ifndef CLIENT
#define CLIENT
#include <X11/Xlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/xwayland.h>

#include "parseConfig.h"
#include "workspaceset.h"
#include "monitor.h"

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
    size_t focused_workspace[2];
};

/* it ignores bool  hiding which visibleon doesn't */
void focus_client(struct client *old, struct client *c);
float calc_ratio(float width, float height);
bool visibleon_workspace(struct client *c, size_t focusedTag);

extern struct wl_list clients; /* tiling order */
extern struct wlr_output_layout *output_layout;

struct wlr_surface *get_wlrsurface(struct client *c);
#endif
