#ifndef MONITOR_H
#define MONITOR_H
#include <wayland-server.h>
#include <wlr/types/wlr_box.h>

#include "tagset.h"

struct monitor {
    /* mons(monitors) list */
    struct wl_list link;

    /* associated with link in container */
    struct wl_list containers;
    /* associated with slink in container */
    struct wl_list stack;
    /* associated with flink in container */
    struct wl_list focus_stack;
    /* associated with llink in container */
    struct wl_list layer_stack;

    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener destroy;
    /* monitor area, layout-relative */
    struct wlr_box m;
    struct tagset *tagset;
    double mfact;
    int nmaster;
};

void create_monitor(struct wl_listener *listener, void *data);
void destroy_monitor(struct wl_listener *listener, void *data);
void focusmon(int i);
void set_monitor(struct monitor *m);

struct monitor *dirtomon(int dir);
struct monitor *xytomon(double x, double y);

extern struct wl_list mons;
extern struct monitor *selected_monitor;
#endif /* MONITOR_H */
