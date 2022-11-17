#include "seat.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/util/log.h>

#include "monitor.h"
#include "container.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "keyboard.h"
#include "tablet.h"

static void handle_set_selection(struct wl_listener *listener, void *data)
{
    struct seat *seat = wl_container_of(listener, seat, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(seat->wlr_seat, event->source, event->serial);
}

void handle_set_primary_selection(struct wl_listener *listener, void *data)
{
    struct seat *seat = wl_container_of(listener, seat, request_set_primary_selection);
    struct wlr_seat_request_set_primary_selection_event *event = data;
    wlr_seat_set_primary_selection(seat->wlr_seat, event->source, event->serial);
}

struct seat *create_seat(const char *seat_name)
{
    struct seat *seat = calloc(1, sizeof(*seat));

    seat->wlr_seat = wlr_seat_create(server.wl_display, seat_name);
    seat->wlr_seat->data = seat;

    seat->cursor = create_cursor(seat);

    LISTEN(&seat->wlr_seat->events.request_set_selection,
            &seat->request_set_selection,
            handle_set_selection);
    LISTEN(&seat->wlr_seat->events.request_set_primary_selection,
            &seat->request_set_primary_selection,
            handle_set_primary_selection);

    seat->devices = g_ptr_array_new();

    return seat;
}

void destroy_seat(struct seat *seat)
{
    destroy_cursor(seat->cursor);

    g_ptr_array_unref(seat->devices);

    free(seat);
}

static struct seat_device *seat_get_device(struct seat *seat,
        struct input_device *input_device) {

    for (int i = 0; i < seat->devices->len; i++) {
        struct seat_device *seat_device = g_ptr_array_index(seat->devices, i);
        if (seat_device->input_device == input_device) {
            return seat_device;
        }
    }

    return NULL;
}

static void seat_device_destroy(struct seat_device *seat_device) {
    if (!seat_device) {
        return;
    }

    destroy_keyboard(seat_device->keyboard);
    wlr_cursor_detach_input_device(seat_device->seat->cursor->wlr_cursor,
        seat_device->input_device->wlr_device);
    g_ptr_array_remove(seat_device->seat->devices, seat_device);
    free(seat_device);
}

static void seat_update_capabilities(struct seat *seat) {
    uint32_t caps = 0;
    uint32_t previous_caps = seat->wlr_seat->capabilities;
    for (int i = 0; i < seat->devices->len; i++) {
        struct seat_device *seat_device = g_ptr_array_index(seat->devices, i);

        switch (seat_device->input_device->wlr_device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            caps |= WL_SEAT_CAPABILITY_KEYBOARD;
            break;
        case WLR_INPUT_DEVICE_POINTER:
            caps |= WL_SEAT_CAPABILITY_POINTER;
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            caps |= WL_SEAT_CAPABILITY_TOUCH;
            break;
        case WLR_INPUT_DEVICE_TABLET_TOOL:
            caps |= WL_SEAT_CAPABILITY_POINTER;
            break;
        case WLR_INPUT_DEVICE_SWITCH:
        case WLR_INPUT_DEVICE_TABLET_PAD:
            break;
        }
    }

    // Hide cursor if seat doesn't have pointer capability.
    // We must call cursor_set_image while the wlr_seat has the capabilities
    // otherwise it's a no op.
    if ((caps & WL_SEAT_CAPABILITY_POINTER) == 0) {
        cursor_set_image(seat->cursor, NULL, NULL);
        wlr_seat_set_capabilities(seat->wlr_seat, caps);
    } else {
        wlr_seat_set_capabilities(seat->wlr_seat, caps);
        if ((previous_caps & WL_SEAT_CAPABILITY_POINTER) == 0) {
            cursor_set_image(seat->cursor, "left_ptr", NULL);
        }
    }
}


void seat_remove_device(struct seat *seat, struct input_device *input_device)
{
    struct seat_device *seat_device = seat_get_device(seat, input_device);

    if (!seat_device) {
        return;
    }

    seat_device_destroy(seat_device);

    seat_update_capabilities(seat);
}

static bool xcursor_manager_is_named(const struct wlr_xcursor_manager *manager,
        const char *name) {
    return (!manager->name && !name) ||
        (name && manager->name && strcmp(name, manager->name) == 0);
}

void seat_configure_xcursor(struct seat *seat) {
    unsigned cursor_size = 24;
    const char *cursor_theme = NULL;

    if (seat == input_manager_get_default_seat()) {
        char cursor_size_fmt[16];
        snprintf(cursor_size_fmt, sizeof(cursor_size_fmt), "%u", cursor_size);
        setenv("XCURSOR_SIZE", cursor_size_fmt, 1);
        if (cursor_theme != NULL) {
            setenv("XCURSOR_THEME", cursor_theme, 1);
        }
    }

    /* Create xcursor manager if we don't have one already, or if the
     * theme has changed */
    if (!seat->cursor->xcursor_mgr ||
            !xcursor_manager_is_named(
                seat->cursor->xcursor_mgr, cursor_theme) ||
            seat->cursor->xcursor_mgr->size != cursor_size) {

        wlr_xcursor_manager_destroy(seat->cursor->xcursor_mgr);
        seat->cursor->xcursor_mgr =
            wlr_xcursor_manager_create(cursor_theme, cursor_size);
        if (!seat->cursor->xcursor_mgr) {
            printf("Cannot create XCursor manager for theme '%s'\n", cursor_theme);
        }
    }

    for (int i = 0; i < server.mons->len; ++i) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct wlr_output *output = m->wlr_output;
        bool result =
            wlr_xcursor_manager_load(seat->cursor->xcursor_mgr,
                output->scale);
        if (!result) {
            printf(
                "Cannot load xcursor theme for output '%s' with scale %f",
                output->name, output->scale);
        }
    }

    // Reset the cursor so that we apply it to outputs that just appeared
    cursor_set_image(seat->cursor, NULL, NULL);
    cursor_set_image(seat->cursor, "left_ptr", NULL);
    wlr_cursor_warp(seat->cursor->wlr_cursor, NULL, seat->cursor->wlr_cursor->x,
        seat->cursor->wlr_cursor->y);
}


static void seat_configure_pointer(struct seat *seat,
        struct seat_device *seat_device) {
    create_pointer(seat, seat_device);
    if ((seat->wlr_seat->capabilities & WL_SEAT_CAPABILITY_POINTER) == 0) {
        seat_configure_xcursor(seat);
    }
}

static void seat_configure_keyboard(struct seat *seat,
        struct seat_device *seat_device) {
    if (!seat_device->keyboard) {
        create_keyboard(seat, seat_device);
    }
    /* sway_keyboard_configure(seat_device->keyboard); */
    wlr_seat_set_keyboard(seat->wlr_seat, seat_device->input_device->wlr_device);
    /* struct sway_node *focus = seat_get_focus(seat); */
    /* if (focus && node_is_view(focus)) { */
    /*     // force notify reenter to pick up the new configuration */
    /*     wlr_seat_keyboard_notify_clear_focus(seat->wlr_seat); */
    /*     seat_keyboard_notify_enter(seat, focus->sway_container->view->surface); */
    /* } */
}

static void seat_configure_tablet_tool(struct seat *seat,
        struct seat_device *device) {
    if (!device->tablet) {
        device->tablet = tablet_create(seat, device);
    }
    configure_tablet(device->tablet);
    // TODO we only need this
    printf("attach tablet\n");
    wlr_cursor_attach_input_device(seat->cursor->wlr_cursor,
        device->input_device->wlr_device);
    // seat_apply_input_config(seat, sway_device);
}

static void seat_configure_tablet_pad(struct seat *seat,
        struct seat_device *device) {
    if (!device->tablet_pad) {
        device->tablet_pad = tablet_pad_create(seat, device);
    }
    configure_tablet_pad(device->tablet_pad);
}


void seat_configure_device(struct seat *seat,
        struct input_device *input_device) {
    struct seat_device *seat_device = seat_get_device(seat, input_device);
    if (!seat_device) {
        return;
    }

    switch (input_device->wlr_device->type) {
        case WLR_INPUT_DEVICE_POINTER:
            seat_configure_pointer(seat, seat_device);
            break;
        case WLR_INPUT_DEVICE_KEYBOARD:
            seat_configure_keyboard(seat, seat_device);
            break;
        case WLR_INPUT_DEVICE_TABLET_TOOL:
            seat_configure_tablet_tool(seat, seat_device);
            break;
        case WLR_INPUT_DEVICE_TABLET_PAD:
            seat_configure_tablet_pad(seat, seat_device);
            break;
        default:
            break;
    }
}

void seat_add_device(struct seat *seat, struct input_device *input_device) {
    if (seat_get_device(seat, input_device)) {
        seat_configure_device(seat, input_device);
        return;
    }

    struct seat_device *seat_device =
        calloc(1, sizeof(*seat_device));
    if (!seat_device) {
        printf("could not allocate seat device");
        return;
    }

    seat_device->seat = seat;
    seat_device->input_device = input_device;
    g_ptr_array_add(seat->devices, seat_device);

    seat_configure_device(seat, input_device);

    seat_update_capabilities(seat);
}

