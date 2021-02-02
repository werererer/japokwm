#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <wayland-util.h>

#include "container.h"
#include "popup.h"
#include "server.h"
#include "tile/tile.h"
#include "tile/tileUtils.h"
#include "utils/coreUtils.h"

//global variables
struct wl_list clients; /* tiling order */

struct wlr_surface *get_wlrsurface(struct client *c)
{
    if (!c)
        return NULL;
    switch (c->type) {
        case XDG_SHELL:
            return c->surface.xdg->surface;
            break;
        case LAYER_SHELL:
            return c->surface.layer->surface;
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            return c->surface.xwayland->surface;
        default:
            printf("wlr_surface is not supported: \n");
            return NULL;
    }
}

static void unfocus_client(struct client *c)
{
    if (!c)
        return;

    switch (c->type) {
        case XDG_SHELL:
            wlr_xdg_toplevel_set_activated(c->surface.xdg, false);
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            wlr_xwayland_surface_activate(c->surface.xwayland, false);
            break;
        default:
            break;
    }
}

void focus_client(struct client *old, struct client *c)
{
    printf("focus client: %p -> %p\n", old, c);
    struct wlr_keyboard *kb = wlr_seat_get_keyboard(server.seat);

    unfocus_client(old);
    /* Update wlroots' keyboard focus */
    if (!c) {
        /* With no client, all we have left is to clear focus */
        wlr_seat_keyboard_notify_clear_focus(server.seat);
        return;
    }

    /* Have a client, so focus its top-level wlr_surface */
    wlr_seat_keyboard_notify_enter(server.seat, get_wlrsurface(c), kb->keycodes,
            kb->num_keycodes, &kb->modifiers);

    /* Activate the new client */
    switch (c->type) {
        case XDG_SHELL:
            wlr_xdg_toplevel_set_activated(c->surface.xdg, true);
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            wlr_xwayland_surface_activate(c->surface.xwayland, true);
            break;
        default:
            break;
    }
}

void client_setsticky(struct client *c, bool sticky)
{
    c->sticky = sticky;
}

float calc_ratio(float width, float height)
{
    return height / width;
}

void reset_client_borders(int border_px)
{
    struct client *c;
    wl_list_for_each(c, &clients, link) {
        if (!existon(c->con, selected_monitor))
            continue;
        c->bw = border_px;
    }
}
