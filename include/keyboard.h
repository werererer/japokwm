#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <wayland-server.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/backend/multi.h>
#include <xkbcommon/xkbcommon.h>

void buttonpress(struct wl_listener *listener, void *data);
void cleanupkeyboard(struct wl_listener *listener, void *data);
void create_keyboard(struct wlr_input_device *device);
void keypress(struct wl_listener *listener, void *data);
void keypressmod(struct wl_listener *listener, void *data);

#endif /* KEYBOARD_H */
