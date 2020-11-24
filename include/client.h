#ifndef CLIENT
#define CLIENT
#include <wlr/types/wlr_layer_shell_v1.h>
#include <X11/Xlib.h>
#include <wlr/xwayland.h>

#include "tagset.h"
#include "parseConfig.h"

enum shell { XDG_SHELL, X11_MANAGED, X11_UNMANAGED, LAYER_SHELL }; /* client types */

struct client {
    struct wl_list link;
    struct wl_list flink;
    struct wl_list slink;
    struct wl_list llink;
    struct wl_list plink;
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
    struct wlr_box geom;  /* layout-relative, includes border */
    struct monitor *mon;
    enum shell type;
    struct tagset *tagset;
    int bw;
    int id;
    // if position -1 it is floating
    int position;
    char *title;
    bool floating;
    bool hidden;
    uint32_t resize; /* configure serial of a pending resize */
};

void applybounds(struct client *c, struct wlr_box bbox);

/* it ignores bool  hiding which visibleon doesn't */
bool existon(struct client *c, struct monitor *m);
bool visibleon(struct client *c, struct monitor *m);
bool hiddenon(struct client *c, struct monitor *m);
bool visible_on_tag(struct client *c, struct monitor *m, size_t focusedTag);
struct client *next_client();
struct client *selected_client();
struct client *get_client(int i);
/* get first visible client */
struct client *firstClient();
/* get last visible client */
struct client *last_client();
struct client *xytoclient(double x, double y);
struct wlr_surface *xytosurface(double x, double y);
/* add new client and focus and raise it */
void add_client_to_focusstack(struct client *c);
void add_client_to_stack(struct client *c);
void applyrules(struct client *c);
void focus_client(struct client *old, struct client *c, bool lift);
void focus_top_client(struct client *old, bool lift);
void lift_client(struct client *c);

extern struct wl_list clients; /* tiling order */
extern struct wl_list focus_stack;  /* focus order */
extern struct wl_list stack;   /* stacking z-order */
extern struct wl_list independents;
extern struct wl_list layerstack;   /* stacking z-order */
extern struct wlr_output_layout *output_layout;

struct wlr_surface *get_wlrsurface(struct client *c);
#endif
