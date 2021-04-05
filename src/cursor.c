#include "cursor.h"

#include "container.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "popup.h"
#include <wlr/xcursor.h>

struct wl_listener request_set_cursor = {.notify = handle_set_cursor};

static struct container *grabc = NULL;
static int offsetx, offsety;

// TODO refactor this function
static void pointer_focus(struct container *con, struct wlr_surface *surface, double sx, double sy, uint32_t time);

static void pointer_focus(struct container *con, struct wlr_surface *surface, double sx, double sy, uint32_t time)
{
    /* Use top level surface if nothing more specific given */
    if (con && !surface)
        surface = get_wlrsurface(con->client);

    /* If surface is NULL, clear pointer focus */
    if (!surface) {
        wlr_seat_pointer_notify_clear_focus(server.seat);
        return;
    }

/*     /1* If surface is already focused, only notify motion *1/ */
/*     if (surface == server.seat->pointer_state.focused_surface) { */
/*         wlr_seat_pointer_notify_motion(server.seat, time, sx, sy); */
/*         return; */
/*     } */
/*     /1* Otherwise, let the client know that the mouse cursor has entered one */
/*      * of its surfaces, and make keyboard focus follow if desired. *1/ */
/*     wlr_seat_pointer_notify_enter(server.seat, surface, sx, sy); */

/*     if (!con) */
/*         return; */

/*     struct workspace *ws = get_workspace(&server.workspaces, con->m->ws_ids[0]); */
/*     if (ws->layout[0].options.sloppy_focus) */
/*         focus_container(con, FOCUS_NOOP); */
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
    /* This event is forwarded by the cursor when a pointer emits a _relative_
     * pointer motion event (i.e. a delta) */
    struct wlr_event_pointer_motion *event = data;
    /* The cursor doesn't move unless we tell it to. The cursor automatically
     * handles constraining the motion to the output layout, as well as any
     * special configuration applied for the specific input device which
     * generated the event. You can pass NULL for the device if you want to move
     * the cursor around without any input. */
    wlr_cursor_move(server.cursor.wlr_cursor, event->device, event->delta_x, event->delta_y);
    motion_notify(event->time_msec);
}

void motion_absolute(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an _absolute_
     * motion event, from 0..1 on Each axis. This happens, for example, when
     * wlroots is running under a Wayland window rather than KMS+DRM, and you
     * move the mouse over the Windows. You could enter the window from any edge,
     * so we have to warp the mouse there. There is also some hardware which
     * emits these events. */
    struct wlr_event_pointer_motion_absolute *event = data;
    wlr_cursor_warp_absolute(server.cursor.wlr_cursor, event->device, event->x, event->y);
    motion_notify(event->time_msec);
}

void motion_notify(uint32_t time)
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
                absolute_x_to_container_relative(focus_con, cursorx),
                absolute_y_to_container_relative(focus_con, cursory),
                &sx, &sy);

        if (popups_exist())
            destroy_popups();

        update_cursor(&server.cursor);
    }

    pointer_focus(focus_con, focus_surface, sx, sy, time);
}

void move_resize(int ui)
{
    grabc = xy_to_container(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y);
    if (!grabc)
        return;
    if (grabc->client->type == LAYER_SHELL)
        return;

    struct wlr_cursor *cursor = server.cursor.wlr_cursor;
    struct monitor *m = grabc->m;

    /* Float the window and tell motion_notify to grab it */
    set_container_floating(grabc, true);
    update_container_stack_positions(m);

    struct layout *lt = get_layout_in_monitor(m);
    if (lt->options.arrange_by_focus) {
        return;
    }

    switch (server.cursor.cursor_mode = ui) {
        case CURSOR_MOVE:
            wlr_xcursor_manager_set_cursor_image(server.cursor_mgr, "fleur", cursor);
            wlr_seat_pointer_notify_clear_focus(server.seat);
            offsetx = absolute_x_to_container_relative(grabc, cursor->x);
            offsety = absolute_y_to_container_relative(grabc, cursor->y);
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
