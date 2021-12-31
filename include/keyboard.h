#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <wayland-server.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/backend/multi.h>
#include <xkbcommon/xkbcommon.h>
#include <wlr/types/wlr_input_device.h>

#include "seat.h"

struct keyboard {
    struct seat *seat;
    struct seat_device *seat_device;

    int32_t repeat_rate;
    int32_t repeat_delay;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;

    struct wl_event_source *key_repeat_source;

    char *repeat_binding;
};

typedef uint32_t xkb_keysym_t;

void cleanupkeyboard(struct wl_listener *listener, void *data);
void create_keyboard(struct seat *seat, struct seat_device *device);
void destroy_keyboard(struct keyboard *kb);
void handle_key_event(struct wl_listener *listener, void *data);
void keypressmod(struct wl_listener *listener, void *data);

#endif /* KEYBOARD_H */
