#ifndef MONITOR_H
#define MONITOR_H
#include <wayland-server.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_output_damage.h>

#include "container.h"
#include "workspace.h"
#include "root.h"
#include "tagset.h"

struct monitor {
    struct wlr_output *wlr_output;
    struct wlr_output_damage *damage;

    struct wl_listener mode;
    struct wl_listener frame;
    struct wl_listener damage_frame;
    struct wl_listener destroy;
    /* monitor area, layout-relative */
    struct wlr_box geom;
    struct root *root;
    float scale;
    struct tagset *tagset;
};

struct monrule {
    char *name;
    int lua_func_ref;
};

/* associated with stlink in container  */
extern struct wl_list sticky_stack;

void center_mouse_in_monitor(struct monitor *m);
void create_monitor(struct wl_listener *listener, void *data);
void destroy_monitor(struct wl_listener *listener, void *data);
void scale_monitor(struct monitor *m, float scale);
void focus_monitor(struct monitor *m);
void monitor_focus_tags(int ws_id, struct BitSet bitset);
void focus_tags(struct BitSet bitset);
void transform_monitor(struct monitor *m, enum wl_output_transform transform);
void update_monitor_geometries();

struct monitor *dirtomon(int dir);
struct monitor *output_to_monitor(struct wlr_output *output);
struct monitor *xy_to_monitor(double x, double y);
struct tagset *monitor_get_active_tagset(struct monitor *m);
struct workspace *monitor_get_active_workspace(struct monitor *m);
struct layout *get_layout_in_monitor(struct monitor *m);

extern struct monitor *selected_monitor;
#endif /* MONITOR_H */
