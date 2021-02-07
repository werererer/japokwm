#ifndef CURSOR_H
#define CURSOR_H

#include "lib/actions/actions.h"
#include <wayland-server-core.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_seat.h>

enum cursor_mode {
    CURSOR_NORMAL,
    CURSOR_MOVE,
    CURSOR_RESIZE
};

struct cursor {
    struct wl_listener request_set_cursor;
    struct wl_listener image_surface_destroy;

    enum cursor_mode cursor_mode;
    struct wlr_surface *cursor_surface;
    struct wlr_cursor *wlr_cursor;
    int hotspot_x;
    int hotspot_y;
};

void axisnotify(struct wl_listener *listener, void *data);
void create_pointer(struct wlr_input_device *device);
void cursorframe(struct wl_listener *listener, void *data);
/* This event is raised by the seat when a client provides a cursor image */
void handle_set_cursor(struct wl_listener *listener, void *data);

void motion_relative(struct wl_listener *listener, void *data);
void motion_absolute(struct wl_listener *listener, void *data);
void motion_notify(uint32_t time);
/* reload the surface stored in cursor */
void update_cursor(struct cursor *cursor);
void move_resize(int ui);
void cursor_set_image_surface(struct cursor *cursor,
        struct wlr_surface *surface, int32_t hotspot_x, int32_t hotspot_y,
        struct wl_client *client);

extern struct wl_listener request_set_cursor;

#endif /* CURSOR_H */
