#ifndef CLIENT
#define CLIENT
#include <wlr/types/wlr_layer_shell_v1.h>
#include <X11/Xlib.h>
#include <wlr/xwayland.h>

#include "parseConfig.h"
#include "tagset.h"
#include "utils/coreUtils.h"

enum shell { XDGShell, X11Managed, X11Unmanaged, LayerShell }; /* client types */

struct client {
    struct wl_list link;
    struct wl_list flink;
    struct wl_list slink;
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
    struct wlr_box geom;  /* layout-relative, includes border */
    struct monitor *mon;
    enum shell type;
    struct tagset tagset;
    int bw;
    bool floating;
    uint32_t resize; /* configure serial of a pending resize */
};

void clientCreate(struct client *c);
void clientDestroy(struct client *c);

void applybounds(struct client *c, struct wlr_box bbox);
void applyrules(struct client *c);
struct client *selClient();
bool visibleon(struct client *c, struct monitor *m);

extern struct wl_list clients; /* tiling order */
extern struct wl_list focus_stack;  /* focus order */
extern struct wl_list stack;   /* stacking z-order */
extern struct wlr_output_layout *output_layout;
extern struct wlr_box sgeom;

struct wlr_surface *getWlrSurface(struct client *c);
#endif
