#ifndef MONITOR_H
#define MONITOR_H
#include <wayland-server.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_output_damage.h>

#include "server.h"
#include "bitset/bitset.h"

struct cursor;

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

    int tag_id;
};

struct monrule {
    char *name;
    int lua_func_ref;
};

struct tag;
void center_cursor_in_monitor(struct cursor *cursor, struct monitor *m);
void create_monitor(struct wl_listener *listener, void *data);
void create_output(struct wlr_backend *backend, void *data);
void handle_destroy_monitor(struct wl_listener *listener, void *data);
void scale_monitor(struct monitor *m, float scale);
void focus_monitor(struct monitor *m);
void focus_tags(struct BitSet bitset);
void transform_monitor(struct monitor *m, enum wl_output_transform transform);
void update_monitor_geometries();

void monitor_set_selected_tag(struct monitor *m, struct tag *tag);

void handle_output_mgr_apply(struct wl_listener *listener, void *data);
void handle_output_mgr_test(struct wl_listener *listener, void *data);
void handle_output_mgr_apply_test(struct wlr_output_configuration_v1 *config, bool test);

BitSet *monitor_get_tags(struct monitor *m);

struct monitor *output_to_monitor(struct wlr_output *output);
struct monitor *xy_to_monitor(double x, double y);
struct tag *monitor_get_active_tag(struct monitor *m);
struct layout *get_layout_in_monitor(struct monitor *m);
struct root *monitor_get_active_root(struct monitor *m);
struct wlr_box monitor_get_active_geom(struct monitor *m);
#endif /* MONITOR_H */
