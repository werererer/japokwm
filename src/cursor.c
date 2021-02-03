#include "cursor.h"

#include "container.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "popup.h"

struct wl_listener request_set_cursor = {.notify = handle_set_cursor};

static struct container *grabc = NULL;
static int dx, dy; /* client-relative */

static void pointer_focus(struct container *con, struct wlr_surface *surface, double sx, double sy, uint32_t time);

static void pointer_focus(struct container *con, struct wlr_surface *surface, double sx, double sy, uint32_t time)
{
    /* Use top level surface if nothing more specific given */
    if (con && !surface)
        surface = get_wlrsurface(con->client);

    /* If surface is NULL, clear pointer focus */
    if (!surface) {
        printf("clear focus\n");
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

    if (con->m->ws[0]->layout[0].options.sloppy_focus)
        focus_container(con, FOCUS_NOOP);
}

static bool handle_move_resize(enum cursor_mode cursor_mode)
{
    bool ret_val = false;
    switch (cursor_mode) {
        case CURSOR_MOVE:
            resize_container(grabc, dx, dy);
            ret_val = true;
            break;
        case CURSOR_RESIZE:
            move_container(grabc, grabc->geom.x, grabc->geom.y);
            ret_val = true;
            break;
        default:
            break;
    }
    return ret_val;
}

// TODO optimize this function
void motionnotify(uint32_t time)
{
    int cursorx = server.cursor.wlr_cursor->x;
    int cursory = server.cursor.wlr_cursor->y;

    set_selected_monitor(xytomon(cursorx, cursory));

    double sx = 0, sy = 0;
    struct wlr_surface *surface = NULL;

    set_selected_monitor(xytomon(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y));

    /* If handled successfully return */
    if (handle_move_resize(server.cursor.cursor_mode))
        return;

    bool is_popup = false;
    struct monitor *m = selected_monitor;
    struct container *con = focused_container(m);

    if (!con)
        return;

    is_popup = !wl_list_empty(&con->client->surface.xdg->popups);
    if (is_popup){
        switch (con->client->type) {
            case XDG_SHELL:
                surface = wlr_xdg_surface_surface_at(
                        con->client->surface.xdg,
                        /* absolute mouse position to relative in regards to
                         * the client */
                        server.cursor.wlr_cursor->x - con->geom.x,
                        server.cursor.wlr_cursor->y - con->geom.y,
                        &sx, &sy);
                break;
            case LAYER_SHELL:
                    surface = wlr_layer_surface_v1_surface_at(
                            con->client->surface.layer,
                            server.cursor.wlr_cursor->x - con->geom.x,
                            server.cursor.wlr_cursor->y - con->geom.y,
                            &sx, &sy);
                    break;
                    default:
                    break;
        }
    }

    // if surface and subsurface exist
    if (!surface) {
        is_popup = false;
    } else if (surface == focused_container(m)->client->surface.xdg->surface) {
        struct container *con = xytocontainer(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y);
        if (con) {
            is_popup = is_popup && surface == con->client->surface.xdg->surface;
        }
    }

    if (!surface && !is_popup) {
        if (!wl_list_empty(&popups)) {
            struct xdg_popup *popup = wl_container_of(popups.next, popup, plink);
            wlr_xdg_popup_destroy(popup->xdg->base);
        }
        surface = wlr_surface_surface_at(get_wlrsurface(con->client),
                server.cursor.wlr_cursor->x - con->geom.x,
                server.cursor.wlr_cursor->y - con->geom.y, &sx, &sy);
    }

    update_cursor(&server.cursor);

    // if there is no popup use the selected client's surface
    if (!is_popup) {
        con = xytocontainer(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y);
        if (con) {
            surface = get_wlrsurface(con->client);
        }
    }

    printf("pointer focus container: %p\n", con);
    if (con) {
        pointer_focus(con, surface, sx, sy, time);
    }
}

void move_resize(int ui)
{
    grabc = xytocontainer(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y);
    if (!grabc)
        return;

    /* Float the window and tell motionnotify to grab it */
    set_container_floating(grabc, true);
    switch (server.cursor.cursor_mode = ui) {
        case CURSOR_MOVE:
            dx = server.cursor.wlr_cursor->x - grabc->geom.x;
            dy = server.cursor.wlr_cursor->y - grabc->geom.y;
            wlr_xcursor_manager_set_cursor_image(server.cursor_mgr, "fleur", server.cursor.wlr_cursor);
            wlr_seat_pointer_notify_clear_focus(server.seat);
            arrange();
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
            arrange();
            break;
        default:
            break;
    }
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

    if (!server.seat->pointer_state.focused_client) {
        return;
    }

    if (!xytocontainer(cursor->wlr_cursor->x, cursor->wlr_cursor->y)) {
        wlr_xcursor_manager_set_cursor_image(server.cursor_mgr,
            "left_ptr", server.cursor.wlr_cursor);
        return;
    }

    wlr_cursor_set_surface(cursor->wlr_cursor, cursor->cursor_surface,
            cursor->hotspot_x, cursor->hotspot_y);
}
