#ifndef MONITOR_H
#define MONITOR_H
#include <wayland-server.h>
#include <wlr/types/wlr_box.h>

#include "tagset.h"

struct monitor {
    struct wl_list link;
    struct wlr_output *output;
    struct wl_listener frame;
    struct wl_listener destroy;
    /* monitor area, layout-relative */
    struct wlr_box m;
    struct tagset *tagset;
    double mfact;
    int nmaster;
};

void create_monitor(struct wl_listener *listener, void *data);
void cleanupMonitor(struct wl_listener *listener, void *data);
void set_monitor(struct monitor *m);
struct monitor *xytomon(double x, double y);

extern struct wl_list mons;
extern struct monitor *selected_monitor;
#endif /* MONITOR_H */
