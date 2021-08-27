#include "input_manager.h"

#include <ctype.h>
#include <wlr/backend/libinput.h>
#include <wlr/util/log.h>

#include "cursor.h"
#include "keyboard.h"
#include "seat.h"
#include "server.h"
#include "stringop.h"
#include "utils/coreUtils.h"

#define DEFAULT_SEAT "seat0"

static bool device_is_touchpad(struct input_device *device)
{
    if (device->wlr_device->type != WLR_INPUT_DEVICE_POINTER ||
            !wlr_input_device_is_libinput(device->wlr_device)) {
        return false;
    }

    struct libinput_device *libinput_device =
        wlr_libinput_get_device_handle(device->wlr_device);

    return libinput_device_config_tap_get_finger_count(libinput_device) > 0;
}

const char *input_device_get_type(struct input_device *device)
{
    switch (device->wlr_device->type) {
    case WLR_INPUT_DEVICE_POINTER:
        if (device_is_touchpad(device)) {
            return "touchpad";
        } else {
            return "pointer";
        }
    case WLR_INPUT_DEVICE_KEYBOARD:
        return "keyboard";
    case WLR_INPUT_DEVICE_TOUCH:
        return "touch";
    case WLR_INPUT_DEVICE_TABLET_TOOL:
        return "tablet_tool";
    case WLR_INPUT_DEVICE_TABLET_PAD:
        return "tablet_pad";
    case WLR_INPUT_DEVICE_SWITCH:
        return "switch";
    }
    return "unknown";
}

/* static void apply_input_type_config(struct input_device *input_device) { */
/*     const char *device_type = input_device_get_type(input_device); */
/*     struct input_config *type_config = NULL; */
/*     for (int i = 0; i < config->input_type_configs->length; i++) { */
/*         struct input_config *ic = config->input_type_configs->items[i]; */
/*         if (strcmp(ic->identifier + 5, device_type) == 0) { */
/*             type_config = ic; */
/*             break; */
/*         } */
/*     } */
/*     if (type_config == NULL) { */
/*         return; */
/*     } */

/*     for (int i = 0; i < config->input_configs->length; i++) { */
/*         struct input_config *ic = config->input_configs->items[i]; */
/*         if (strcmp(input_device->identifier, ic->identifier) == 0) { */
/*             struct input_config *current = new_input_config(ic->identifier); */
/*             merge_input_config(current, type_config); */
/*             merge_input_config(current, ic); */

/*             current->input_type = device_type; */
/*             config->input_configs->items[i] = current; */
/*             free_input_config(ic); */
/*             ic = NULL; */

/*             break; */
/*         } */
/*     } */
/* } */

static struct input_device *input_device_from_wlr(
        struct wlr_input_device *device) {
    for (int i = 0; i < server.input_manager->devices->len; i++) {
        struct input_device *input_device =
            g_ptr_array_index(server.input_manager->devices, i);
        if (input_device->wlr_device == device) {
            return input_device;
        }
    }
    return NULL;
}

static void handle_device_destroy(struct wl_listener *listener, void *data) {
    struct wlr_input_device *device = data;

    struct input_device *input_device = input_device_from_wlr(device);

    if (!input_device) {
        return;
    }

    for (int i = 0; i < server.input_manager->seats->len; i++) {
        struct seat *seat = g_ptr_array_index(server.input_manager->seats, i);
        seat_remove_device(seat, input_device);
    }

    wl_list_remove(&input_device->device_destroy.link);
    g_ptr_array_remove(server.input_manager->devices, input_device);

    free(input_device->identifier);
    free(input_device);
}

static void handle_new_input(struct wl_listener *listener, void *data)
{
    struct input_manager *input_manager = wl_container_of(listener, input_manager, new_input);
    struct wlr_input_device *device = data;

    struct input_device *input_device = calloc(1, sizeof(struct input_device));
    device->data = input_device;

    input_device->wlr_device = device;
    input_device->identifier = input_device_get_identifier(device);
    g_ptr_array_add(input_manager->devices, input_device);

    /* apply_input_type_config(input_device); */

    /* sway_input_configure_libinput_device(input_device); */

    wl_signal_add(&device->events.destroy, &input_device->device_destroy);
    input_device->device_destroy.notify = handle_device_destroy;

    /* bool added = false; */
    for (int i = 0; i < input_manager->seats->len; i++) {
        struct seat *seat = g_ptr_array_index(input_manager->seats, i);

/*         struct seat_config *seat_config = seat_get_config(seat); */
/*         bool has_attachment = seat_config && */
/*             (seat_config_get_attachment(seat_config, input_device->identifier) || */
/*              seat_config_get_attachment(seat_config, "*")); */

        /* if (has_attachment) { */
            seat_add_device(seat, input_device);
            /* added = true; */
        /* } */
    }

    /* input_manager_verify_fallback_seat(); */
}

static void handle_inhibit_activate(struct wl_listener *listener, void *data)
{
}

static void handle_inhibit_deactivate(struct wl_listener *listener, void *data)
{
}

static void handle_keyboard_shortcuts_inhibit_new_inhibitor(
        struct wl_listener *listener, void *data)
{
}

static void handle_new_virtual_keyboard(struct wl_listener *listener, void *data)
{

}

struct seat *input_manager_seat_from_wlr_seat(struct wlr_seat *wlr_seat) {
    for (int i = 0; i < server.input_manager->seats->len; i++) {
        struct seat *seat = g_ptr_array_index(server.input_manager->seats, i);
        if (seat->wlr_seat == wlr_seat) {
            return seat;
        }
    }

    return NULL;
}


static void handle_new_virtual_pointer(struct wl_listener *listener, void *data)
{
    struct input_manager *input_manager = wl_container_of(listener, input_manager, new_virtual_pointer);

    struct wlr_virtual_pointer_v1_new_pointer_event *event = data;
    struct wlr_virtual_pointer_v1 *pointer = event->new_pointer;
    struct wlr_input_device *device = &pointer->input_device;

    struct seat *seat = event->suggested_seat ?
        input_manager_seat_from_wlr_seat(event->suggested_seat) :
        input_manager_get_default_seat();

    struct input_device *input_device = calloc(1, sizeof(struct input_device));
    device->data = input_device;

    input_device->is_virtual = true;
    input_device->wlr_device = device;
    input_device->identifier = input_device_get_identifier(device);
    g_ptr_array_add(input_manager->devices, &input_device);

    wl_signal_add(&device->events.destroy, &input_device->device_destroy);
    input_device->device_destroy.notify = handle_device_destroy;

    seat_add_device(seat, input_device);

    if (event->suggested_output) {
        wlr_cursor_map_input_to_output(seat->cursor->wlr_cursor, device,
            event->suggested_output);
    }
}

struct input_manager *create_input_manager()
{
    struct input_manager *input_manager = calloc(1, sizeof(struct input_manager));

    input_manager->devices = g_ptr_array_new();
    input_manager->seats = g_ptr_array_new();

    LISTEN(&server.backend->events.new_input, &input_manager->new_input,
            handle_new_input);

    input_manager->inhibit = wlr_input_inhibit_manager_create(server.wl_display);
    LISTEN(&input_manager->inhibit->events.activate, &input_manager->inhibit_activate,
            handle_inhibit_activate);
    LISTEN(&input_manager->inhibit->events.deactivate, &input_manager->inhibit_deactivate,
            handle_inhibit_deactivate);

    input_manager->keyboard_shortcuts_inhibit =
        wlr_keyboard_shortcuts_inhibit_v1_create(server.wl_display);
    LISTEN(&input_manager->keyboard_shortcuts_inhibit->events.new_inhibitor,
            &input_manager->keyboard_shortcuts_inhibit_new_inhibitor,
            handle_keyboard_shortcuts_inhibit_new_inhibitor);

    input_manager->virtual_pointer = wlr_virtual_pointer_manager_v1_create(server.wl_display);
    LISTEN(&input_manager->virtual_pointer->events.new_virtual_pointer,
            &input_manager->new_virtual_pointer, handle_new_virtual_pointer);

    input_manager->virtual_keyboard = wlr_virtual_keyboard_manager_v1_create(server.wl_display);
    LISTEN(&input_manager->virtual_keyboard->events.new_virtual_keyboard,
            &input_manager->new_virtual_keyboard, handle_new_virtual_keyboard);

    return input_manager;
}

void destroy_input_manager(struct input_manager *input_manager)
{
    wl_list_remove(&input_manager->new_input.link);
    wl_list_remove(&input_manager->inhibit_activate.link);
    wl_list_remove(&input_manager->inhibit_deactivate.link);
    wl_list_remove(&input_manager->keyboard_shortcuts_inhibit_new_inhibitor.link);
    wl_list_remove(&input_manager->new_virtual_keyboard.link);
    wl_list_remove(&input_manager->new_virtual_pointer.link);
    free(input_manager);
}

struct seat *input_manager_get_default_seat()
{
    return input_manager_get_seat(DEFAULT_SEAT);
}

struct seat *input_manager_get_seat(const char *seat_name)
{
    for (int i = 0; i < server.input_manager->seats->len; i++) {
        struct seat *seat = g_ptr_array_index(server.input_manager->seats, i);
        if (strcmp(seat->wlr_seat->name, seat_name) == 0) {
            return seat;
        }
    }

    return NULL;
}

char *input_device_get_identifier(struct wlr_input_device *device)
{
    int vendor = device->vendor;
    int product = device->product;
    char *name = strdup(device->name ? device->name : "");
    strip_whitespace(name);

    char *p = name;
    for (; *p; ++p) {
        // There are in fact input devices with unprintable characters in its name
        if (*p == ' ' || !isprint(*p)) {
            *p = '_';
        }
    }

    const char *fmt = "%d:%d:%s";
    int len = snprintf(NULL, 0, fmt, vendor, product, name) + 1;
    char *identifier = malloc(len);
    if (!identifier) {
        printf("Unable to allocate unique input device name\n");
        return NULL;
    }

    snprintf(identifier, len, fmt, vendor, product, name);
    free(name);
    return identifier;
}

