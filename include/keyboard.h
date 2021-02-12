#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <wayland-server.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/backend/multi.h>
#include <xkbcommon/xkbcommon.h>

struct keyboard {
    struct wl_list link;
    struct wlr_input_device *device;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

typedef uint32_t xkb_keysym_t;

void buttonpress(struct wl_listener *listener, void *data);
void cleanupkeyboard(struct wl_listener *listener, void *data);
void create_keyboard(struct wlr_input_device *device);
void keypress(struct wl_listener *listener, void *data);
void keypressmod(struct wl_listener *listener, void *data);

#endif /* KEYBOARD_H */
