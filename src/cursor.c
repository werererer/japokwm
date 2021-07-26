#include "cursor.h"

#include "container.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "popup.h"
#include "keybinding.h"
#include <wlr/xcursor.h>
#include <wlr/types/wlr_relative_pointer_v1.h>
#include <wlr/util/region.h>

struct wl_listener request_set_cursor = {.notify = handle_set_cursor};

static struct container *grabc = NULL;
static int offsetx, offsety;

// TODO refactor this function
static void pointer_focus(struct wlr_surface *surface, double sx, double sy, uint32_t time);

static void pointer_focus(struct wlr_surface *surface, double sx, double sy, uint32_t time)
{
    /* Use top level surface if nothing more specific given */
    if (!surface)
        return;

    /* If surface is NULL, clear pointer focus */
    if (!surface) {
        wlr_seat_pointer_notify_clear_focus(server.seat);
        return;
    }

    /* If surface is already focused, only notify motion */
    if (surface == server.seat->pointer_state.focused_surface) {
        wlr_seat_pointer_notify_motion(server.seat, time, sx, sy);
        return;
    }
    /* Otherwise, let the client know that the mouse cursor has entered one
     * of its surfaces, and make keyboard focus follow if desired. */
    wlr_seat_pointer_notify_enter(server.seat, surface, sx, sy);
}

void axisnotify(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an axis event,
     * for example when you move the scroll wheel. */
    struct wlr_event_pointer_axis *event = data;
    /* Notify the client with pointer focus of the axis event. */
    wlr_seat_pointer_notify_axis(server.seat,
            event->time_msec, event->orientation, event->delta,
            event->delta_discrete, event->source);
}

void create_pointer(struct wlr_input_device *device)
{
    /* We don't do anything special with pointers. All of our pointer handling
     * is proxied through wlr_cursor. On another compositor, you might take this
     * opportunity to do libinput configuration on the device to set
     * acceleration, etc. */
    wlr_cursor_attach_input_device(server.cursor.wlr_cursor, device);
}

void cursorframe(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an frame
     * event. Frame events are sent after regular pointer events to group
     * multiple events together. For instance, two axis events may happen at the
     * same time, in which case a frame event won't be sent in between. */
    /* Notify the client with pointer focus of the frame event. */
    wlr_seat_pointer_notify_frame(server.seat);
}

static bool handle_move_resize(enum cursor_mode cursor_mode)
{
    struct wlr_cursor *cursor = server.cursor.wlr_cursor;
    switch (cursor_mode) {
        case CURSOR_MOVE:
            move_container(grabc, cursor, offsetx, offsety);
            return true;
            break;
        case CURSOR_RESIZE:
            resize_container(grabc, server.cursor.wlr_cursor, 0, 0);
            return true;
            break;
        default:
            break;
    }
    return false;
}

void motion_relative(struct wl_listener *listener, void *data)
{
    struct cursor *cursor = &server.cursor;
    struct wlr_event_pointer_motion *event = data;
    /* cursor_handle_activity_from_device(cursor, event->device); */

    motion_notify(cursor, event->time_msec, event->device, event->delta_x,
            event->delta_y, event->unaccel_dx, event->unaccel_dy);
}

void motion_absolute(struct wl_listener *listener, void *data)
{
    struct cursor *cursor = &server.cursor;
    struct wlr_event_pointer_motion_absolute *event = data;
    // TODO what does this in sway?
    /* cursor_handle_activity_from_device(cursor, event->device); */

    double lx, ly;
    wlr_cursor_absolute_to_layout_coords(cursor->wlr_cursor, event->device,
            event->x, event->y, &lx, &ly);

    double dx = lx - cursor->wlr_cursor->x;
    double dy = ly - cursor->wlr_cursor->y;

    motion_notify(cursor, event->time_msec, event->device, dx, dy, dx, dy);
}

static void focus_under_cursor(uint32_t time)
{
    int cursorx = server.cursor.wlr_cursor->x;
    int cursory = server.cursor.wlr_cursor->y;

    focus_monitor(xy_to_monitor(cursorx, cursory));

    /* If handled successfully return */
    if (handle_move_resize(server.cursor.cursor_mode))
        return;

    double sx = 0, sy = 0;
    struct wlr_surface *popup_surface = get_popup_surface_under_cursor(&sx, &sy);
    bool is_popup_under_cursor = popup_surface != NULL;

    struct wlr_surface *focus_surface = popup_surface;

    struct container *focus_con = xy_to_container(cursorx, cursory);
    if (!is_popup_under_cursor && focus_con) {
        focus_surface = wlr_surface_surface_at(get_wlrsurface(focus_con->client),
                absolute_x_to_container_relative(focus_con->geom, cursorx),
                absolute_y_to_container_relative(focus_con->geom, cursory),
                &sx, &sy);

        update_cursor(&server.cursor);
    }

    pointer_focus(focus_surface, sx, sy, time);

    if (focus_con) {
        struct workspace *ws = monitor_get_active_workspace(selected_monitor);
        if (ws->layout->options.sloppy_focus)
            focus_container(focus_con, FOCUS_NOOP);
    }
}


void motion_notify(struct cursor *cursor, uint32_t time_msec,
        struct wlr_input_device *device, double dx, double dy,
        double dx_unaccel, double dy_unaccel)
{
    wlr_relative_pointer_manager_v1_send_relative_motion(
            server.relative_pointer_mgr,
            server.seat, (uint64_t)time_msec * 1000,
            dx, dy, dx_unaccel, dy_unaccel);

    // Only apply pointer constraints to real pointer input.
    if (cursor->active_constraint && device->type == WLR_INPUT_DEVICE_POINTER) {
        struct wlr_surface *surface = NULL;
        double sx, sy;
        /* node_at_coords(cursor->seat, */
        /*         cursor->cursor->x, cursor->cursor->y, &surface, &sx, &sy); */

        if (cursor->active_constraint->surface != surface) {
            return;
        }

        double sx_confined, sy_confined;
        /* if (!wlr_region_confine(&cursor->confine, sx, sy, sx + dx, sy + dy, */
        /*             &sx_confined, &sy_confined)) { */
        /*     return; */
        /* } */

        dx = sx_confined - sx;
        dy = sy_confined - sy;
    }

    wlr_cursor_move(cursor->wlr_cursor, device, dx, dy);

/*     seatop_pointer_motion(cursor->seat, time_msec); */
}

void buttonpress(struct wl_listener *listener, void *data)
{
    struct wlr_event_pointer_button *event = data;

    switch (event->state) {
        case WLR_BUTTON_PRESSED:
            {
                /* Translate libinput to xkbcommon code */
                unsigned sym = event->button + 64985;

                /* get modifiers */
                struct wlr_keyboard *kb = wlr_seat_get_keyboard(server.seat);
                int mods = wlr_keyboard_get_modifiers(kb);

                handle_keybinding(mods, sym);
                break;
            }
        case WLR_BUTTON_RELEASED:
            /* If you released any buttons, we exit interactive move/resize
             * mode. */
            /* XXX should reset to the pointer focus's current setcursor */
            if (server.cursor.cursor_mode != CURSOR_NORMAL) {
                wlr_xcursor_manager_set_cursor_image(server.cursor_mgr, "left_ptr",
                        server.cursor.wlr_cursor);
                server.cursor.cursor_mode = CURSOR_NORMAL;
                /* Drop the window off on its new monitor */
                struct monitor *m = xy_to_monitor(server.cursor.wlr_cursor->x,
                        server.cursor.wlr_cursor->y);
                focus_monitor(m);
                return;
            }
            break;
    }
    /* If the event wasn't handled by the compositor, notify the client with
     * pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(server.seat, event->time_msec, event->button,
            event->state);
}

void move_resize(int ui)
{
    grabc = xy_to_container(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y);
    if (!grabc)
        return;
    if (grabc->client->type == LAYER_SHELL)
        return;

    struct wlr_cursor *cursor = server.cursor.wlr_cursor;
    struct monitor *m = container_get_monitor(grabc);
    struct layout *lt = get_layout_in_monitor(m);
    // all floating windows will be tiled. Thats why you can't make new windows
    // tiled
    if (lt->options.arrange_by_focus)
        return;

    /* Float the window and tell motion_notify to grab it */
    set_container_floating(grabc, fix_position, true);

    switch (server.cursor.cursor_mode = ui) {
        case CURSOR_MOVE:
            wlr_xcursor_manager_set_cursor_image(server.cursor_mgr, "fleur", cursor);
            wlr_seat_pointer_notify_clear_focus(server.seat);
            offsetx = absolute_x_to_container_relative(grabc->geom, cursor->x);
            offsety = absolute_y_to_container_relative(grabc->geom, cursor->y);
            break;
        case CURSOR_RESIZE:
            /* Doesn't work for X11 output - the next absolute motion event
             * returns the cursor to where it started */
            wlr_cursor_warp_closest(server.cursor.wlr_cursor, NULL,
                    grabc->geom.x + grabc->geom.width,
                    grabc->geom.y + grabc->geom.height);
            wlr_xcursor_manager_set_cursor_image(server.cursor_mgr,
                    "bottom_right_corner", server.cursor.wlr_cursor);
            wlr_seat_pointer_notify_clear_focus(server.seat);
            break;
        default:
            break;
    }
    arrange();
}

void handle_set_cursor(struct wl_listener *listener, void *data)
{
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    struct cursor *cursor = &server.cursor;

    /* This can be sent by any client, so we check to make sure this one is
     * actually has pointer focus first. If so, we can tell the cursor to
     * use the provided surface as the cursor image. It will set the
     * hardware cursor on the output that it's currently on and continue to
     * do so as the cursor moves between outputs. */
    if (event->seat_client != server.seat->pointer_state.focused_client) {
        return;
    }

    cursor->cursor_surface = event->surface;
    cursor->hotspot_x = event->hotspot_x;
    cursor->hotspot_y = event->hotspot_y;

    update_cursor(cursor);
}

void handle_new_virtual_pointer(struct wl_listener *listener, void *data)
{
    struct wlr_virtual_pointer_v1_new_pointer_event *event = data;
    struct wlr_virtual_pointer_v1 *pointer = event->new_pointer;
    struct wlr_input_device *device = &pointer->input_device;

    wlr_cursor_attach_input_device(server.cursor.wlr_cursor, device);
}

void update_cursor(struct cursor *cursor)
{
    /* If we're "grabbing" the server.cursor, don't use the client's image */
    /* XXX still need to save the provided surface to restore later */
    if (cursor->cursor_mode != CURSOR_NORMAL)
        return;

    if (!xy_to_container(cursor->wlr_cursor->x, cursor->wlr_cursor->y)) {
        wlr_xcursor_manager_set_cursor_image(server.cursor_mgr,
            "left_ptr", server.cursor.wlr_cursor);
        return;
    }

    if (!cursor->cursor_surface)
        return;

    if (!server.seat->pointer_state.focused_client)
        return;

    wlr_cursor_set_surface(cursor->wlr_cursor, cursor->cursor_surface,
            cursor->hotspot_x, cursor->hotspot_y);
}
