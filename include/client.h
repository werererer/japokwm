#ifndef CLIENT
#define CLIENT
#include <X11/Xlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/xwayland.h>

#include "parseConfig.h"
#include "tagset.h"
#include "monitor.h"

enum shell { XDG_SHELL, X11_MANAGED, X11_UNMANAGED, LAYER_SHELL }; /* client types */

struct client {
    /* clients */
    struct wl_list link;
    /* layer shell based clients */
    struct wl_list llink;

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
    struct tagset *tagset;
    int id;
    char *title;
    uint32_t resize; /* configure serial of a pending resize */
};

/* it ignores bool  hiding which visibleon doesn't */
void focus_client(struct client *old, struct client *c);
bool visibleon_tag(struct client *c, struct monitor *m, size_t focusedTag);

extern struct wl_list clients; /* tiling order */
extern struct wl_list independents;
extern struct wl_list layerstack;   /* stacking z-order */
extern struct wlr_output_layout *output_layout;

struct wlr_surface *get_wlrsurface(struct client *c);
#endif
