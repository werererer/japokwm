#ifndef MONITOR_H
#define MONITOR_H
#include <wayland-server.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_output_damage.h>

#include "container.h"
#include "workspace.h"
#include "root.h"

struct monitor {
    /* mons(monitors) list */
    struct wl_list link;

    struct wlr_output *wlr_output;
    struct wlr_output_damage *damage;

    struct wl_listener mode;
    struct wl_listener damage_frame;
    struct wl_listener destroy;
    /* monitor area, layout-relative */
    struct wlr_box geom;
    struct root *root;
    double mfact;
    struct workspace *ws;
};

/* associated with slink in container */
extern struct wl_list stack;
/* associated with flink in container */
extern struct wl_list focus_stack;
/* associated with link in container */
extern struct wl_list containers;
/* associated with llink in container */
extern struct wl_list layer_stack;
/* associated with plink in container  */
extern struct wl_list popups;


void center_mouse_in_monitor(struct monitor *m);
void create_monitor(struct wl_listener *listener, void *data);
void destroy_monitor(struct wl_listener *listener, void *data);
void focusmon(int i);
void set_selected_monitor(struct monitor *m);
void load_layout(lua_State *L, struct monitor *m, const char *layout_name);
void load_default_layout(lua_State *L, struct monitor *m);

/* *
 * selTag[1] = selTag[0] then
 * selTag[0] = new value
 * */
void push_selected_workspace(struct monitor *m, struct workspace *ws);

struct monitor *dirtomon(int dir);
struct monitor *outputtomon(struct wlr_output *output);
struct monitor *xytomon(double x, double y);

extern struct wl_list mons;
extern struct monitor *selected_monitor;
#endif /* MONITOR_H */
