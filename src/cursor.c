#include "cursor.h"

#include <wlr/xcursor.h>
#include <wlr/types/wlr_relative_pointer_v1.h>
#include <wlr/util/region.h>

#include "container.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "popup.h"
#include "keybinding.h"
#include "seat.h"
#include "workspace.h"

static struct container *grabc = NULL;
static int offsetx, offsety;

// TODO refactor this function
static void pointer_focus(struct seat *seat, struct wlr_surface *surface, double sx, double sy, uint32_t time);
static void warp_to_constraint_cursor_hint(struct cursor *cursor);
static void update_cursor(struct cursor *cursor);

static void pointer_focus(struct seat *seat, struct wlr_surface *surface, double sx, double sy, uint32_t time)
{
    struct wlr_seat *wlr_seat = seat->wlr_seat;
    /* Use top level surface if nothing more specific given */

    /* If surface is NULL, clear pointer focus */
    if (!surface) {
        /* cursor_rebase(seat->cursor); */
        /* warp_to_constraint_cursor_hint(seat->cursor); */
        wlr_seat_pointer_notify_clear_focus(wlr_seat);
        return;
    }

    /* If surface is already focused, only notify motion */
    if (surface == seat->wlr_seat->pointer_state.focused_surface) {
        wlr_seat_pointer_notify_motion(wlr_seat, time, sx, sy);
        return;
    }

    cursor_constrain(seat->cursor, NULL);
    /* Otherwise, let the client know that the mouse cursor has entered one
     * of its surfaces, and make keyboard focus follow if desired. */
    wlr_seat_pointer_notify_enter(wlr_seat, surface, sx, sy);
}

static void handle_axis_notify(struct wl_listener *listener, void *data)
{
    struct cursor *cursor = wl_container_of(listener, cursor, axis);
    /* This event is forwarded by the cursor when a pointer emits an axis event,
     * for example when you move the scroll wheel. */
    struct wlr_event_pointer_axis *event = data;
    /* Notify the client with pointer focus of the axis event. */
    wlr_seat_pointer_notify_axis(cursor->seat->wlr_seat,
            event->time_msec, event->orientation, event->delta,
            event->delta_discrete, event->source);
}

static void cursor_update_image(struct cursor *cursor) {
    cursor_set_image(cursor, "left_ptr", NULL);
}

static void handle_rebase(struct seat *seat, uint32_t time_msec)
{
    struct cursor *cursor = seat->cursor;
    struct wlr_surface *surface = NULL;
    double sx = 0.0, sy = 0.0;

    if (surface) {
        wlr_seat_pointer_notify_enter(seat->wlr_seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(seat->wlr_seat, time_msec, sx, sy);
    } else {
        cursor_update_image(cursor);
        wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
    }
    cursor_update_image(cursor);
}

static uint32_t get_current_time_msec(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

void cursor_rebase(struct cursor *cursor)
{
    uint32_t time_msec = get_current_time_msec();
    handle_rebase(cursor->seat, time_msec);
}

static void handle_image_surface_destroy(struct wl_listener *listener,
        void *data) {
    struct cursor *cursor = wl_container_of(listener, cursor, image_surface_destroy);
    cursor_set_image(cursor, NULL, cursor->image_client);
    cursor_rebase(cursor);
}

static void cursor_hide(struct cursor *cursor) {
    wlr_cursor_set_image(cursor->wlr_cursor, NULL, 0, 0, 0, 0, 0, 0);
    cursor->hidden = true;
    wlr_seat_pointer_notify_clear_focus(cursor->seat->wlr_seat);
}

void cursor_unhide(struct cursor *cursor) {
    if (!cursor->hidden) {
        return;
    }

    cursor->hidden = false;
    if (cursor->image_surface) {
        cursor_set_image_surface(cursor,
                cursor->image_surface,
                cursor->hotspot_x,
                cursor->hotspot_y,
                cursor->image_client);
    } else {
        const char *image = cursor->image;
        cursor->image = NULL;
        cursor_set_image(cursor, image, cursor->image_client);
    }
    cursor_rebase(cursor);
    wl_event_source_timer_update(cursor->hide_source, 0);
}

static int hide_notify(void *data) {
    struct cursor *cursor = data;
    cursor_hide(cursor);
    return 1;
}

struct cursor *create_cursor(struct seat *seat)
{
    struct cursor *cursor = calloc(1, sizeof(struct cursor));

    struct wlr_cursor *wlr_cursor = wlr_cursor_create();
    cursor->wlr_cursor = wlr_cursor;
    wlr_cursor->data = cursor;

    cursor->seat = seat;

    wlr_cursor_attach_output_layout(wlr_cursor, server.output_layout);

    cursor->hide_source = wl_event_loop_add_timer(server.wl_event_loop,
            hide_notify, cursor);

    cursor->xcursor_mgr = wlr_xcursor_manager_create(NULL, 24);

    /*
     * wlr_cursor *only* displays an image on screen. It does not move around
     * when the pointer moves. However, we can attach input devices to it, and
     * it will generate aggregate events for all of them. In these events, we
     * can choose how we want to process them, forwarding them to clients and
     * moving the cursor around. More detail on this process is described in my
     * input handling blog post:
     *
     * https://drewdevault.com/2018/07/17/Input-handling-in-wlroots.html
     *
     * And more comments are sprinkled throughout the notify functions above.
     */
    LISTEN(&wlr_cursor->events.axis, &cursor->axis, handle_axis_notify);
    LISTEN(&wlr_cursor->events.button, &cursor->button, handle_cursor_button);
    LISTEN(&wlr_cursor->events.frame, &cursor->frame, handle_cursor_frame);
    LISTEN(&wlr_cursor->events.motion, &cursor->motion, handle_motion_relative);
    LISTEN(&wlr_cursor->events.motion_absolute, &cursor->motion_absolute, handle_motion_absolute);

    wl_list_init(&cursor->image_surface_destroy.link);
    cursor->image_surface_destroy.notify = handle_image_surface_destroy;

    LISTEN(&seat->wlr_seat->events.request_set_cursor, &cursor->request_set_cursor, handle_set_cursor);

    wl_list_init(&cursor->constraint_commit.link);

    return cursor;
}

void destroy_cursor(struct cursor *cursor)
{
    wl_list_remove(&cursor->axis.link);
    wl_list_remove(&cursor->button.link);
    wl_list_remove(&cursor->frame.link);
    wl_list_remove(&cursor->motion.link);
    wl_list_remove(&cursor->motion_absolute.link);

    wl_list_remove(&cursor->request_set_cursor.link);
    wl_list_remove(&cursor->image_surface_destroy.link);

    wl_list_remove(&cursor->constraint_commit.link);

    free(cursor);
}

static void set_image_surface(struct cursor *cursor,
        struct wlr_surface *surface) {
    wl_list_remove(&cursor->image_surface_destroy.link);
    cursor->image_surface = surface;
    if (surface) {
        wl_signal_add(&surface->events.destroy, &cursor->image_surface_destroy);
    } else {
        wl_list_init(&cursor->image_surface_destroy.link);
    }
}

void cursor_set_image(struct cursor *cursor, const char *image, struct wl_client *client)
{
    if (!(cursor->seat->wlr_seat->capabilities & WL_SEAT_CAPABILITY_POINTER)) {
        return;
    }

    const char *current_image = cursor->image;
    set_image_surface(cursor, NULL);
    cursor->image = image;
    cursor->hotspot_x = cursor->hotspot_y = 0;
    cursor->image_client = client;

    if (cursor->hidden) {
        return;
    }

    if (!image) {
        wlr_cursor_set_image(cursor->wlr_cursor, NULL, 0, 0, 0, 0, 0, 0);
    } else if (!current_image || strcmp(current_image, image) != 0) {
        wlr_xcursor_manager_set_cursor_image(cursor->xcursor_mgr, image,
                cursor->wlr_cursor);
    }
}

void create_pointer(struct seat *seat, struct seat_device *seat_device)
{
    /* We don't do anything special with pointers. All of our pointer handling
     * is proxied through wlr_cursor. On another compositor, you might take this
     * opportunity to do libinput configuration on the device to set
     * acceleration, etc. */

    struct wlr_input_device *device = seat_device->input_device->wlr_device;
    wlr_cursor_attach_input_device(seat->cursor->wlr_cursor, device);
}

void handle_cursor_frame(struct wl_listener *listener, void *data)
{
    struct cursor *cursor = wl_container_of(listener, cursor, frame);
    /* This event is forwarded by the cursor when a pointer emits an frame
     * event. Frame events are sent after regular pointer events to group
     * multiple events together. For instance, two axis events may happen at the
     * same time, in which case a frame event won't be sent in between. */
    /* Notify the client with pointer focus of the frame event. */
    wlr_seat_pointer_notify_frame(cursor->seat->wlr_seat);
}

static bool handle_move_resize(struct cursor *cursor)
{
    enum cursor_mode cursor_mode = cursor->cursor_mode;
    struct wlr_cursor *wlr_cursor = cursor->wlr_cursor;
    switch (cursor_mode) {
        case CURSOR_MOVE:
            move_container(grabc, wlr_cursor, offsetx, offsety);
            return true;
            break;
        case CURSOR_RESIZE:
            resize_container(grabc, wlr_cursor, 0, 0);
            return true;
            break;
        default:
            break;
    }
    return false;
}

void cursor_handle_activity_from_device(struct cursor *cursor, struct wlr_input_device *device)
{
    cursor_unhide(cursor);
}

void handle_motion_relative(struct wl_listener *listener, void *data)
{
    struct cursor *cursor = wl_container_of(listener, cursor, motion);
    struct wlr_event_pointer_motion *event = data;
    cursor_handle_activity_from_device(cursor, event->device);

    motion_notify(cursor, event->time_msec, event->device, event->delta_x,
            event->delta_y, event->unaccel_dx, event->unaccel_dy);
}

void handle_motion_absolute(struct wl_listener *listener, void *data)
{
    struct cursor *cursor = wl_container_of(listener, cursor, motion_absolute);
    struct wlr_event_pointer_motion_absolute *event = data;
    cursor_handle_activity_from_device(cursor, event->device);

    double lx, ly;
    wlr_cursor_absolute_to_layout_coords(cursor->wlr_cursor, event->device,
            event->x, event->y, &lx, &ly);

    double dx = lx - cursor->wlr_cursor->x;
    double dy = ly - cursor->wlr_cursor->y;

    motion_notify(cursor, event->time_msec, event->device, dx, dy, dx, dy);
}

void focus_under_cursor(struct cursor *cursor, uint32_t time)
{
    int cursorx = cursor->wlr_cursor->x;
    int cursory = cursor->wlr_cursor->y;

    debug_print("mon: %p\n", xy_to_monitor(cursorx, cursory));
    focus_monitor(xy_to_monitor(cursorx, cursory));

    /* If handled successfully return */
    if (handle_move_resize(cursor))
        return;

    double sx = 0, sy = 0;
    struct wlr_surface *popup_surface = get_popup_surface_under_cursor(cursor, &sx, &sy);
    bool is_popup_under_cursor = popup_surface != NULL;

    struct wlr_surface *final_focus_surface = popup_surface;

    struct container *focus_con = xy_to_container(cursorx, cursory);
    if (!is_popup_under_cursor && focus_con) {
        final_focus_surface = wlr_surface_surface_at(get_wlrsurface(focus_con->client),
                absolute_x_to_container_relative(container_get_geom(focus_con), cursorx),
                absolute_y_to_container_relative(container_get_geom(focus_con), cursory),
                &sx, &sy);
    }

    pointer_focus(cursor->seat, final_focus_surface, sx, sy, time);

    if (focus_con) {
        struct workspace *ws = monitor_get_active_workspace(selected_monitor);
        if (ws->layout->options.sloppy_focus &&
                !popups_exist() && !xwayland_popups_exist()) {
            focus_container(focus_con);
        }
    }
}

void motion_notify(struct cursor *cursor, uint32_t time_msec,
        struct wlr_input_device *device, double dx, double dy,
        double dx_unaccel, double dy_unaccel)
{
    struct wlr_seat *wlr_seat = cursor->seat->wlr_seat;
    wlr_relative_pointer_manager_v1_send_relative_motion(
            server.relative_pointer_mgr,
            wlr_seat, (uint64_t)time_msec * 1000,
            dx, dy, dx_unaccel, dy_unaccel);

    if (!cursor->active_constraint) {
        struct container *sel_con = get_focused_container(selected_monitor);
        if (sel_con) {
            cursor->active_constraint = wlr_pointer_constraints_v1_constraint_for_surface(
                    server.pointer_constraints, get_wlrsurface(sel_con->client), wlr_seat);
        }
    }

    // Only apply pointer constraints to real pointer input.
    if (cursor->active_constraint && device->type == WLR_INPUT_DEVICE_POINTER) {
        struct container *con = xy_to_container(cursor->wlr_cursor->x, cursor->wlr_cursor->y);
        struct wlr_box *con_geom = container_get_geom(con);
        double sx;
        double sy;
        if (con) {
            sx = cursor->wlr_cursor->x - con_geom->x;
            sy = cursor->wlr_cursor->y - con_geom->y;
        } else {
            sx = cursor->wlr_cursor->x;
            sy = cursor->wlr_cursor->y;
        }

        double sx_confined, sy_confined;
        if (wlr_region_confine(&cursor->confine, sx, sy, sx + dx, sy + dy,
                    &sx_confined, &sy_confined)) {
            dx = sx_confined - sx;
            dy = sy_confined - sy;
        }
    }

    wlr_cursor_move(cursor->wlr_cursor, device, dx, dy);

    focus_under_cursor(cursor, 0);
}

void handle_cursor_button(struct wl_listener *listener, void *data)
{
    struct cursor *cursor = wl_container_of(listener, cursor, button);
    struct wlr_event_pointer_button *event = data;

    switch (event->state) {
        case WLR_BUTTON_PRESSED:
            {
                /* Translate libinput to xkbcommon code */
                unsigned sym = event->button + 64985;

                /* get modifiers */
                struct seat *seat = input_manager_get_default_seat();
                struct wlr_keyboard *kb = wlr_seat_get_keyboard(seat->wlr_seat);
                int mods = wlr_keyboard_get_modifiers(kb);

                handle_keybinding(mods, sym);
                break;
            }
        case WLR_BUTTON_RELEASED:
            /* If you released any buttons, we exit interactive move/resize
             * mode. */
            /* XXX should reset to the pointer focus's current setcursor */
            if (cursor->cursor_mode != CURSOR_NORMAL) {
                wlr_xcursor_manager_set_cursor_image(cursor->xcursor_mgr,
                        "left_ptr", cursor->wlr_cursor);
                cursor->cursor_mode = CURSOR_NORMAL;
                /* Drop the window off on its new monitor */
                struct monitor *m = xy_to_monitor(cursor->wlr_cursor->x,
                        cursor->wlr_cursor->y);
                focus_monitor(m);
                return;
            }
            break;
    }

    if (cursor->cursor_mode == CURSOR_RESIZE) {
        return;
    }
    if (cursor->cursor_mode == CURSOR_MOVE) {
        return;
    }

    /* If the event wasn't handled by the compositor, notify the client with
     * pointer focus that a button press has occurred */
    struct wlr_seat *wlr_seat = cursor->seat->wlr_seat;
    wlr_seat_pointer_notify_button(wlr_seat, event->time_msec, event->button,
            event->state);
}

void move_resize(struct cursor *cursor, int ui)
{
    grabc = xy_to_container(cursor->wlr_cursor->x, cursor->wlr_cursor->y);
    if (!grabc)
        return;
    if (grabc->client->type == LAYER_SHELL)
        return;

    struct monitor *m = container_get_monitor(grabc);
    struct layout *lt = get_layout_in_monitor(m);
    // all floating windows will be tiled. Thats why you can't make new windows
    // tiled
    if (lt->options.arrange_by_focus)
        return;

    /* Float the window and tell motion_notify to grab it */
    set_container_floating(grabc, container_fix_position, true);

    struct wlr_box *grabc_geom = container_get_geom(grabc);
    struct wlr_cursor *wlr_cursor = cursor->wlr_cursor;
    switch (cursor->cursor_mode = ui) {
        case CURSOR_MOVE:
            wlr_xcursor_manager_set_cursor_image(cursor->xcursor_mgr, "fleur", wlr_cursor);
            offsetx = absolute_x_to_container_relative(grabc_geom, wlr_cursor->x);
            offsety = absolute_y_to_container_relative(grabc_geom, wlr_cursor->y);
            break;
        case CURSOR_RESIZE:
            /* Doesn't work for X11 output - the next absolute motion event
             * returns the cursor to where it started */
            wlr_cursor_warp_closest(wlr_cursor, NULL,
                    grabc_geom->x + grabc_geom->width,
                    grabc_geom->y + grabc_geom->height);
            wlr_xcursor_manager_set_cursor_image(cursor->xcursor_mgr,
                    "bottom_right_corner", wlr_cursor);
            break;
        default:
            break;
    }
    arrange();
}

void cursor_set_image_surface(struct cursor *cursor,
        struct wlr_surface *surface, int32_t hotspot_x, int32_t hotspot_y,
        struct wl_client *client) {
    if (!(cursor->seat->wlr_seat->capabilities & WL_SEAT_CAPABILITY_POINTER)) {
        return;
    }

    set_image_surface(cursor, surface);
    cursor->image = NULL;
    cursor->hotspot_x = hotspot_x;
    cursor->hotspot_y = hotspot_y;
    cursor->image_client = client;

    if (cursor->hidden) {
        return;
    }

    wlr_cursor_set_surface(cursor->wlr_cursor, surface, hotspot_x, hotspot_y);
}


void handle_set_cursor(struct wl_listener *listener, void *data)
{
    struct cursor *cursor = wl_container_of(listener, cursor, request_set_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;

    /* This can be sent by any client, so we check to make sure this one is
     * actually has pointer focus first. If so, we can tell the cursor to
     * use the provided surface as the cursor image. It will set the
     * hardware cursor on the output that it's currently on and continue to
     * do so as the cursor moves between outputs. */
    if (event->seat_client != cursor->seat->wlr_seat->pointer_state.focused_client) {
        return;
    }

    cursor->cursor_surface = event->surface;
    cursor->hotspot_x = event->hotspot_x;
    cursor->hotspot_y = event->hotspot_y;

    update_cursor(cursor);
}

static void handle_pointer_constraint_set_region(struct wl_listener *listener,
        void *data) {
    struct pointer_constraint *sway_constraint =
        wl_container_of(listener, sway_constraint, set_region);
    struct cursor *cursor = sway_constraint->cursor;

    cursor->active_confine_requires_warp = true;
}

static void warp_to_constraint_cursor_hint(struct cursor *cursor)
{
    struct wlr_pointer_constraint_v1 *constraint = cursor->active_constraint;

    if (constraint->current.committed &
            WLR_POINTER_CONSTRAINT_V1_STATE_CURSOR_HINT) {
        double sx = constraint->current.cursor_hint.x;
        double sy = constraint->current.cursor_hint.y;

        /* struct sway_view *view = view_from_wlr_surface(constraint->surface); */
        /* struct sway_container *con = view->container; */

        struct monitor *m = selected_monitor;
        struct container *con = get_focused_container(m);
        struct wlr_box *con_geom = container_get_geom(con);
        double lx = sx + con_geom->x - m->geom.x;
        double ly = sy + con_geom->x - m->geom.y;

        wlr_cursor_warp(cursor->wlr_cursor, NULL, lx, ly);

        // Warp the pointer as well, so that on the next pointer rebase we don't
        // send an unexpected synthetic motion event to clients.
        wlr_seat_pointer_warp(constraint->seat, sx, sy);
    }
}

void handle_constraint_destroy(struct wl_listener *listener, void *data) {
    struct pointer_constraint *pointer_constraint =
        wl_container_of(listener, pointer_constraint, destroy);
    struct wlr_pointer_constraint_v1 *wlr_constraint = data;
    struct cursor *cursor = pointer_constraint->cursor;

    wl_list_remove(&pointer_constraint->set_region.link);
    wl_list_remove(&pointer_constraint->destroy.link);

    if (cursor->active_constraint == wlr_constraint) {
        warp_to_constraint_cursor_hint(cursor);

        if (cursor->constraint_commit.link.next != NULL) {
            wl_list_remove(&cursor->constraint_commit.link);
        }
        wl_list_init(&cursor->constraint_commit.link);
        cursor->active_constraint = NULL;
    }

    free(pointer_constraint);
}

static void check_constraint_region(struct cursor *cursor) {
    struct wlr_pointer_constraint_v1 *constraint = cursor->active_constraint;
    pixman_region32_t *region = &constraint->region;
    struct container *con = get_focused_container(selected_monitor);
    if (cursor->active_confine_requires_warp && con) {
        cursor->active_confine_requires_warp = false;
        struct wlr_box *con_geom = container_get_geom(con);

        double sx = cursor->wlr_cursor->x - con_geom->x;
        double sy = cursor->wlr_cursor->y - con_geom->x;

        if (!pixman_region32_contains_point(region,
                floor(sx), floor(sy), NULL)) {
            int nboxes;
            pixman_box32_t *boxes = pixman_region32_rectangles(region, &nboxes);
            if (nboxes > 0) {
                double sx = (boxes[0].x1 + boxes[0].x2) / 2.;
                double sy = (boxes[0].y1 + boxes[0].y2) / 2.;

                wlr_cursor_warp_closest(cursor->wlr_cursor, NULL,
                    sx + con_geom->x,
                    sy + con_geom->x);

                cursor_rebase(cursor);
            }
        }
    }

    // A locked pointer will result in an empty region, thus disallowing all movement
    if (constraint->type == WLR_POINTER_CONSTRAINT_V1_CONFINED) {
        pixman_region32_copy(&cursor->confine, region);
    } else {
        pixman_region32_clear(&cursor->confine);
    }
}

static void handle_constraint_commit(struct wl_listener *listener,
        void *data) {
    struct cursor *cursor = wl_container_of(listener, cursor, constraint_commit);
    /* struct wlr_pointer_constraint_v1 *constraint = cursor->active_constraint; */
    /* assert(constraint->surface == data); */

    cursor_hide(cursor);
    check_constraint_region(cursor);
}

void cursor_constrain(struct cursor *cursor, struct wlr_pointer_constraint_v1 *constraint) {
    if (cursor->active_constraint == constraint) {
        return;
    }

    wl_list_remove(&cursor->constraint_commit.link);
    if (cursor->active_constraint) {
        if (!constraint) {
            warp_to_constraint_cursor_hint(cursor);
        }
        wlr_pointer_constraint_v1_send_deactivated(cursor->active_constraint);
    }

    cursor->active_constraint = constraint;

    if (constraint == NULL) {
        wl_list_init(&cursor->constraint_commit.link);
        return;
    }

    cursor->active_confine_requires_warp = true;

    // FIXME: Big hack, stolen from wlr_pointer_constraints_v1.c:121.
    // This is necessary because the focus may be set before the surface
    // has finished committing, which means that warping won't work properly,
    // since this code will be run *after* the focus has been set.
    // That is why we duplicate the code here.
    if (pixman_region32_not_empty(&constraint->current.region)) {
        pixman_region32_intersect(&constraint->region,
            &constraint->surface->input_region, &constraint->current.region);
    } else {
        pixman_region32_copy(&constraint->region,
            &constraint->surface->input_region);
    }

    check_constraint_region(cursor);

    wlr_pointer_constraint_v1_send_activated(constraint);

    LISTEN(&constraint->surface->events.commit, &cursor->constraint_commit, handle_constraint_commit);
}

void handle_new_pointer_constraint(struct wl_listener *listener, void *data)
{
    struct wlr_pointer_constraint_v1 *wlr_constraint = data;
    struct seat *seat = wlr_constraint->seat->data;

    struct pointer_constraint *constraint =
        calloc(1, sizeof(struct pointer_constraint));
    constraint->cursor = seat->cursor;
    constraint->wlr_constraint = wlr_constraint;

    constraint->set_region.notify = handle_pointer_constraint_set_region;
    wl_signal_add(&wlr_constraint->events.set_region, &constraint->set_region);

    constraint->destroy.notify = handle_constraint_destroy;
    wl_signal_add(&wlr_constraint->events.destroy, &constraint->destroy);

    struct container *con = get_focused_container(selected_monitor);
    if (con) {
        struct wlr_surface *surface = get_wlrsurface(con->client);
        if (surface == wlr_constraint->surface) {
            cursor_constrain(seat->cursor, wlr_constraint);
        }
    }
}

static void update_cursor(struct cursor *cursor)
{
    /* If we're "grabbing" the server.cursor, don't use the client's image */
    /* XXX still need to save the provided surface to restore later */
    if (cursor->cursor_mode != CURSOR_NORMAL)
        return;

    if (!xy_to_container(cursor->wlr_cursor->x, cursor->wlr_cursor->y)) {
        wlr_xcursor_manager_set_cursor_image(cursor->xcursor_mgr,
            "left_ptr", cursor->wlr_cursor);
        return;
    }

    if (!cursor->seat->wlr_seat->pointer_state.focused_client)
        return;

    wlr_cursor_set_surface(cursor->wlr_cursor, cursor->cursor_surface,
            cursor->hotspot_x, cursor->hotspot_y);
}
