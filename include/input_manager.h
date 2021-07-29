#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <glib.h>
#include <wayland-server.h>
#include <wlr/types/wlr_input_inhibitor.h>
#include <wlr/types/wlr_keyboard_shortcuts_inhibit_v1.h>

struct input_device {
    struct wlr_input_device *wlr_device;
    char *identifier;

    struct wl_listener device_destroy;

    bool is_virtual;
};

struct input_manager {
    GPtrArray *devices;
    GPtrArray *seats;

    struct wlr_input_inhibit_manager *inhibit;
    struct wlr_keyboard_shortcuts_inhibit_manager_v1 *keyboard_shortcuts_inhibit;
    struct wlr_virtual_keyboard_manager_v1 *virtual_keyboard;
    struct wlr_virtual_pointer_manager_v1 *virtual_pointer;

    struct wl_listener new_input;
    struct wl_listener inhibit_activate;
    struct wl_listener inhibit_deactivate;
    struct wl_listener keyboard_shortcuts_inhibit_new_inhibitor;
    struct wl_listener new_virtual_keyboard;
    struct wl_listener new_virtual_pointer;
};

struct input_manager *create_input_manager();
void destroy_input_manager(struct input_manager *input_manager);

struct seat *input_manager_get_default_seat();
struct seat *input_manager_get_seat(const char *seat_name);
struct seat *input_manager_seat_from_wlr_seat(struct wlr_seat *wlr_seat);

char *input_device_get_identifier(struct wlr_input_device *device);

const char *input_device_get_type(struct input_device *device);

#endif /* INPUT_MANAGER_H */
