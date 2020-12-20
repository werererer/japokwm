#ifndef MONITOR_H
#define MONITOR_H
#include <wayland-server.h>
#include <wlr/types/wlr_box.h>

#include "workspace.h"
#include "root.h"

struct monitor {
    /* mons(monitors) list */
    struct wl_list link;

    /* associated with link in container */
    struct wl_list containers;
    /* associated with slink in container */
    struct wl_list stack;
    /* associated with llink in container */
    struct wl_list layer_stack;
    /* associated with plink in container  */
    struct wl_list popups;

    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener destroy;
    /* monitor area, layout-relative */
    struct wlr_box geom;
    struct root *root;
    double mfact;
    int nmaster;
    struct workspace *ws;
    unsigned int focused_workspace[2];
};

/* associated with flink in container */
extern struct wl_list focus_stack;

void create_monitor(struct wl_listener *listener, void *data);
void destroy_monitor(struct wl_listener *listener, void *data);
void focusmon(int i);
void set_selected_monitor(struct monitor *m);
/* set the are where windows can be placed in respect to layershell based
 * programs which occupie space
 * The resulting root area is relative to outputs*/
void set_root_area(struct monitor *m);

struct layout *selected_layout(struct monitor *m);
/* *
 * selTag[1] = selTag[0] then
 * selTag[0] = new value
 * */
void push_selected_workspace(struct monitor *m, struct workspace *ws);
struct workspace *get_focused_workspace(struct monitor *m);

struct monitor *dirtomon(int dir);
struct monitor *outputtomon(struct wlr_output *output);
struct monitor *xytomon(double x, double y);

extern struct wl_list mons;
extern struct monitor *selected_monitor;
#endif /* MONITOR_H */
