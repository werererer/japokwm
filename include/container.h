#ifndef CONTAINER_H
#define CONTAINER_H

#include <lua.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_cursor.h>

#include "client.h"
#include "options.h"

enum focus_actions {
    FOCUS_NOOP,
    FOCUS_LIFT,
};

struct container {
    /* monitor containers */
    struct wl_list mlink;
    /* container stack */
    struct wl_list slink;
    /* container focus_stack */
    struct wl_list flink;
    /* layer shell based clients */
    struct wl_list llink;
    /* independents list */
    struct wl_list ilink;
    /* sticky containers */
    struct wl_list stlink;
    /* scratchpad */
    struct wl_list scratchpad_link;

    /* layout-relative, includes border */
    struct wlr_box geom;
    struct wlr_box prev_geom;
    struct wlr_box prev_floating_geom;
    struct client *client;

    struct monitor *m;
    struct monitor *prev_m;
    bool floating;
    bool focusable;
    bool has_border;
    bool hidden;
    bool on_scratchpad;
    bool on_top;
    // if position -1 it is floating
    bool geom_was_changed;
    int focus_position;
    int position;
    // height = ratio * width
    float ratio;
};

struct container *create_container(struct client *c, struct monitor *m, bool has_border);
void destroy_container(struct container *con);

struct container *get_container(int i);
struct container *get_relative_container(int ws_id, struct container *con, int i);
struct container *get_relative_hidden_container(int ws_id, int i);
struct container *get_relative_hidden_container_in_focus_stack(int ws_id, int i);
struct container *get_focused_container(struct monitor *m);
struct container *xy_to_container(double x, double y);
struct container *container_position_to_container(int ws_id, int position);
struct container *container_position_to_hidden_container(int ws_id, int position);
struct container *container_focus_position_to_container(int ws_id, int position);
struct container *get_relative_focus_container(int ws_id, struct container *con, int i);

struct wlr_box get_center_box(struct wlr_box ref);
struct wlr_box get_centered_box(struct wlr_box box, struct wlr_box ref);
struct wlr_box get_absolute_box(struct wlr_fbox ref, struct wlr_box box);
struct wlr_fbox get_relative_box(struct wlr_box box, struct wlr_box ref);
struct wlr_fbox lua_togeometry(lua_State *L);

void add_container_to_containers(struct container *con, int i);
void apply_bounds(struct container *con, struct wlr_box bbox);
void apply_rules(struct container *con);
void container_damage_part(struct container *con);
void container_damage_whole(struct container *con);
void focus_container(struct container *con, enum focus_actions a);
void focus_most_recent_container(int ws_id, enum focus_actions a);
void focus_on_stack(int i);
void focus_on_hidden_stack(int i);
/* Find the topmost visible client (if any) at point (x, y), including
 * borders. This relies on stack being ordered from top to bottom. */
void lift_container(struct container *con);
void repush(int pos, int pos2);
void set_container_floating(struct container *con, bool floating);
void set_container_geom(struct container *con, struct wlr_box geom);
void set_container_workspace(struct container *con, int ws_id);
void set_container_monitor(struct container *con, struct monitor *m);
void swap_container_positions(struct container *con1, struct container *con2);
void swap_container_properties(struct container *con1, struct container *con2);
void swap_container_focus_positions(struct container *con1, struct container *con2);
void resize_container(struct container *con, struct wlr_cursor *cursor, int dx, int dy);
void move_container(struct container *con, struct wlr_cursor *cursor, int offsetx, int offsety);

// TODO check if those 2 functions even work
int container_relative_x_to_absolute(struct container *con, int lx);
int container_relative_y_to_absolute(struct container *con, int ly);
int absolute_x_to_container_relative(struct container *con, int x);
int absolute_y_to_container_relative(struct container *con, int y);

bool is_resize_not_in_limit(struct wlr_fbox *geom, struct resize_constraints *resize_constraints);
#endif /* CONTAINER_H */
