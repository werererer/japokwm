#include "cursor.h"

#include "container.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "popup.h"

struct wl_listener request_set_cursor = {.notify = handle_set_cursor};

static struct container *grabc;
static int grabcx, grabcy; /* client-relative */

static void pointer_focus(struct container *con, double sx, double sy, uint32_t time);

static void pointer_focus(struct container *con, double sx, double sy, uint32_t time)
{
    struct wlr_surface *surface = get_wlrsurface(con->client);
    printf("pointer_focus\n");
    /* Use top level surface if nothing more specific given */
    if (con && !surface)
        surface = get_wlrsurface(con->client);

    printf("works0\n");
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

// TODO optimize this function
/* void motionnotify(uint32_t time) */
/* { */
/*     double sx = 0, sy = 0; */

/*     set_selected_monitor(xytomon(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y)); */
/*     bool action = false; */
/*     struct wlr_box geom; */
/*     /1* If we are currently grabbing the mouse, handle and return *1/ */
/*     switch (server.cursor.cursor_mode) { */
/*         case CURSOR_MOVE: */
/*             action = true; */
/*             geom.x = server.cursor.wlr_cursor->x - grabcx; */
/*             geom.y = server.cursor.wlr_cursor->y - grabcy; */
/*             geom.width = grabc->geom.width; */
/*             geom.height = grabc->geom.height; */
/*             /1* Move the grabbed client to the new position. *1/ */
/*             resize(grabc, geom, false); */
/*             return; */
/*             break; */
/*         case CURSOR_RESIZE: */
/*             action = true; */
/*             geom.x = grabc->geom.x; */
/*             geom.y = grabc->geom.y; */
/*             geom.width = server.cursor.wlr_cursor->x - grabc->geom.x; */
/*             geom.height = server.cursor.wlr_cursor->y - grabc->geom.y; */
/*             resize(grabc, geom, false); */
/*             return; */
/*             break; */
/*         default: */
/*             break; */
/*     } */

/*     bool is_popup = false; */
/*     struct monitor *m = selected_monitor; */
/*     struct container *con = focused_container(m); */
/*     struct wlr_surface *surface = NULL; */
/*     if (con) { */
/*         switch (con->client->type) { */
/*             case XDG_SHELL: */
/*                 is_popup = !wl_list_empty(&con->client->surface.xdg->popups); */
/*                 if (is_popup) { */
/*                     surface = wlr_xdg_surface_surface_at( */
/*                             con->client->surface.xdg, */
/*                             /1* absolute mouse position to relative in regards to */
/*                              * the client *1/ */
/*                             server.cursor.wlr_cursor->x - con->geom.x, */
/*                             server.cursor.wlr_cursor->y - con->geom.y, */
/*                             &sx, &sy); */
/*                 } */
/*                 break; */
/*             case LAYER_SHELL: */
/*                 is_popup = !wl_list_empty(&con->client->surface.layer->popups); */
/*                 if (is_popup) { */
/*                     surface = wlr_layer_surface_v1_surface_at( */
/*                             con->client->surface.layer, */
/*                             server.cursor.wlr_cursor->x - con->geom.x, */
/*                             server.cursor.wlr_cursor->y - con->geom.y, */
/*                             &sx, &sy); */
/*                 } */
/*                 break; */
/*             default: */
/*                 break; */
/*         } */

/*         // if surface and subsurface exit */
/*         if (!surface) { */
/*             is_popup = false; */
/*         } else if (surface == focused_container(m)->client->surface.xdg->surface) { */
/*             struct container *con = xytocontainer(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y); */
/*             if (con) { */
/*                 is_popup = is_popup && surface == con->client->surface.xdg->surface; */
/*             } */
/*         } */

/*         if (!surface && !is_popup) { */
/*             if (!wl_list_empty(&popups)) { */
/*                 struct xdg_popup *popup = wl_container_of(popups.next, popup, plink); */
/*                 wlr_xdg_popup_destroy(popup->xdg->base); */
/*             } */
/*             surface = wlr_surface_surface_at(get_wlrsurface(con->client), */
/*                     server.cursor.wlr_cursor->x - con->geom.x, */
/*                     server.cursor.wlr_cursor->y - con->geom.y, &sx, &sy); */
/*         } */
/*     } */

/*     update_cursor(&server.cursor); */

/*     // if there is no popup use the selected client's surface */
/*     if (!is_popup) { */
/*         con = xytocontainer(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y); */
/*         if (con) { */
/*             surface = get_wlrsurface(con->client); */
/*         } */
/*     } */

/*     printf("pointer focus container: %p\n", con); */
/*     if (!action && con) { */
/*         pointer_focus(con, sx, sy, time); */
/*     } */
/* } */

void set_grabcontainer(struct container *con, int x, int y)
{
    grabc = con;
    grabcx = x;
    grabcy = y;
}

struct container *get_grabcontainer()
{
    return grabc;
}
