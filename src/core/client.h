#ifndef CLIENT
#define CLIENT
#include "coreUtils.h"
#include "parseConfig.h"
#include <wlr/types/wlr_layer_shell_v1.h>

typedef struct {
    struct wl_list link;
    struct wl_list flink;
    struct wl_list slink;
    union {
        struct wlr_xdg_surface *xdg;
        struct wlr_layer_surface_v1 *layer;
#ifdef XWAYLAND
        struct wlr_xwayland_surface *xwayland;
#endif
    } surface;
#ifdef XWAYLAND
    struct wl_listener activate;
#endif
    struct wl_listener commit;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wlr_box geom;  /* layout-relative, includes border */
    Monitor *mon;
    unsigned int type;
    int bw;
    unsigned int tags;
    int isfloating;
    uint32_t resize; /* configure serial of a pending resize */
} Client;

void applybounds(Client *c, struct wlr_box bbox);
void applyrules(Client *c);
Client *selClient();

extern struct wl_list clients; /* tiling order */
extern struct wl_list focus_stack;  /* focus order */
extern struct wl_list stack;   /* stacking z-order */
extern struct wlr_output_layout *output_layout;
extern struct wlr_box sgeom;
extern struct wl_list mons;
extern Monitor *selMon;

struct wlr_surface *getWlrSurface(Client *c);
#endif
