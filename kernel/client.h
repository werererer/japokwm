#ifndef CLIENT
#define CLIENT
#include "coreUtils.h"

typedef struct {
    struct wl_list link;
    struct wl_list flink;
    struct wl_list slink;
    union {
        struct wlr_xdg_surface *xdg;
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
#ifdef XWAYLAND
    unsigned int type;
#endif
    int bw;
    unsigned int tags;
    int isfloating;
    uint32_t resize; /* configure serial of a pending resize */
} Client;

void applybounds(Client *c, struct wlr_box *bbox);
void applyrules(Client *c);

static struct wl_list clients; /* tiling order */
static struct wl_list fstack;  /* focus order */
static struct wl_list stack;   /* stacking z-order */
static struct wlr_output_layout *output_layout;
static struct wlr_box sgeom;
static struct wl_list mons;
static Monitor *selmon;

#endif
