#ifndef SEAT_H
#define SEAT_H

#include <wayland-server.h>
#include <wlr/types/wlr_pointer_constraints_v1.h>
#include <wlr/types/wlr_seat.h>

#include "input_manager.h"

struct seat {
    struct wlr_seat *wlr_seat;
    struct cursor *cursor;

    struct wl_listener request_set_selection;
    struct wl_listener request_set_primary_selection;

    bool allow_set_cursor;

    GPtrArray *devices;
};

struct seat_device {
    struct seat *seat;
    struct input_device *input_device;
    struct keyboard *keyboard;
	struct tablet *tablet;
};

struct pointer_constraint {
    struct cursor *cursor;
    struct wlr_pointer_constraint_v1 *wlr_constraint;

    struct wl_listener set_region;
    struct wl_listener destroy;
};

struct seat *create_seat(const char *seat_name);
void destroy_seat(struct seat *seat);

void seat_add_device(struct seat *seat, struct input_device *input_device);
void seat_configure_xcursor(struct seat *seat);
void seat_remove_device(struct seat *seat, struct input_device *input_device);

#endif /* SEAT_H */
