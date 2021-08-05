#ifndef CURSOR_H
#define CURSOR_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_pointer_constraints_v1.h>

#include "seat.h"

enum cursor_mode {
    CURSOR_NORMAL,
    CURSOR_MOVE,
    CURSOR_RESIZE
};

struct cursor {
    struct seat *seat;
    struct wlr_surface *image_surface;
    const char *image;

    struct wlr_pointer_constraint_v1 *active_constraint;

    struct wl_listener motion;
    struct wl_listener motion_absolute;
    struct wl_listener button;
    struct wl_listener axis;
    struct wl_listener frame;

    struct wl_listener image_surface_destroy;

    struct wl_listener request_set_cursor;

    struct wl_listener constraint_commit;

    struct wl_event_source *hide_source;

    struct wlr_xcursor_manager *xcursor_mgr;

    enum cursor_mode cursor_mode;
    struct wlr_surface *cursor_surface;
    struct wlr_cursor *wlr_cursor;
    int hotspot_x;
    int hotspot_y;
    struct wl_client *image_client;
    bool active_confine_requires_warp;
    bool hidden;

    pixman_region32_t confine; // invalid if active_constraint == NULL
};

struct cursor *create_cursor(struct seat *seat);
void destroy_cursor(struct cursor *cursor);

void cursor_rebase(struct cursor *cursor);
void cursor_set_image(struct cursor *cursor, const char *image, struct wl_client *client);
void handle_cursor_button(struct wl_listener *listener, void *data);
void create_pointer(struct seat *seat, struct seat_device *seat_device);
void handle_cursor_frame(struct wl_listener *listener, void *data);
/* This event is raised by the seat when a client provides a cursor image */
void handle_set_cursor(struct wl_listener *listener, void *data);
void cursor_constrain(struct cursor *cursor, struct wlr_pointer_constraint_v1 *constraint);
void handle_new_pointer_constraint(struct wl_listener *listener, void *data);
void cursor_update_image(struct cursor *cursor);

void focus_under_cursor(struct cursor *cursor, uint32_t time);
void cursor_handle_activity_from_device(struct cursor *cursor, struct wlr_input_device *device);
void handle_motion_relative(struct wl_listener *listener, void *data);
void handle_motion_absolute(struct wl_listener *listener, void *data);
void motion_notify(struct cursor *cursor, uint32_t time_msec,
        struct wlr_input_device *device, double dx, double dy,
        double dx_unaccel, double dy_unaccel);
/* reload the surface stored in cursor */
void update_cursor(struct cursor *cursor);
void move_resize(struct cursor *cursor, int ui);
void cursor_set_image_surface(struct cursor *cursor,
        struct wlr_surface *surface, int32_t hotspot_x, int32_t hotspot_y,
        struct wl_client *client);

struct wlr_surface *xt_to_surface(double x, double y);

extern struct wl_listener request_set_cursor;

#endif /* CURSOR_H */
