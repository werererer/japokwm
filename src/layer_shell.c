#include "layer_shell.h"

#include <stdlib.h>
#include <string.h>

#include "monitor.h"
#include "popup.h"
#include "server.h"
#include "container.h"
#include "tile/tileUtils.h"
#include "render/render.h"

/* void create_notify_layer_shell(struct wl_listener *listener, void *data) */
/* { */
/*     printf("create_notify_layer_shell\n"); */
/*     /1* This event is raised when wlr_xdg_shell receives a new xdg surface from a */
/*      * client, either a toplevel (application window) or popup. *1/ */
/*     struct wlr_layer_surface_v1 *layer_surface = data; */

/*     /1* Allocate a Client for this surface *1/ */
/*     union surface_t surface; */
/*     surface.layer = layer_surface; */
/*     struct client *c = layer_surface->data = create_client(LAYER_SHELL, surface); */

/*     if (!c->surface.layer->output) { */
/*         c->surface.layer->output = selected_monitor->wlr_output; */
/*     } */
/*     struct monitor *m = output_to_monitor(c->surface.layer->output); */
/*     wlr_layer_surface_v1_configure(c->surface.layer, m->geom.width, m->geom.height); */

/*     /1* Listen to the various events it can emit *1/ */
/*     c->map.notify = maprequest; */
/*     wl_signal_add(&layer_surface->events.map, &c->map); */
/*     c->unmap.notify = unmap_notify; */
/*     wl_signal_add(&layer_surface->events.unmap, &c->unmap); */
/*     c->destroy.notify = destroy_notify; */
/*     wl_signal_add(&layer_surface->events.destroy, &c->destroy); */

/*     /1* popups *1/ */
/*     c->new_popup.notify = popup_handle_new_popup; */
/*     wl_signal_add(&layer_surface->events.new_popup, &c->new_popup); */
/*     struct container *con = create_container(c, m, true); */
/*     printf("con: %p\n", con); */
/*     struct wlr_box geom = { */
/*         .x = 10, */
/*         .y = 10, */
/*         .width = 100, */
/*         .height = 100, */
/*     }; */
/*     resize(con, geom); */
/*     printf("x: %d\n", con->geom.x); */
/*     printf("y: %d\n", con->geom.y); */
/*     printf("width: %d\n", con->geom.width); */
/*     printf("height: %d\n", con->geom.height); */
/* } */

void create_notify_layer_shell(struct wl_listener *listener, void *data)
{
    struct wlr_layer_surface_v1 *wlr_layer_surface = data;

    if (!wlr_layer_surface->output) {
        wlr_layer_surface->output = selected_monitor->wlr_output;
    }

    union surface_t surface;
    surface.layer = wlr_layer_surface;
    struct client *client = create_client(LAYER_SHELL, surface);

    LISTEN(&wlr_layer_surface->events.destroy, &client->destroy,
            destroylayersurfacenotify);
    LISTEN(&wlr_layer_surface->events.map, &client->map,
            maplayersurfacenotify);
    LISTEN(&wlr_layer_surface->events.unmap, &client->unmap,
            unmaplayersurfacenotify);

    struct monitor *m = wlr_layer_surface->output->data;
    client->m = m;
    wlr_list_push(get_layer_list(wlr_layer_surface->client_pending.layer), client);

    // Temporarily set the layer's current state to client_pending
    // so that we can easily arrange it
    struct wlr_layer_surface_v1_state old_state = wlr_layer_surface->current;
    wlr_layer_surface->current = wlr_layer_surface->client_pending;
    arrangelayers(m);
    wlr_layer_surface->current = old_state;

    create_container(client, m, false);
}

void maplayersurfacenotify(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, map);
    wlr_surface_send_enter(get_wlrsurface(c), c->surface.layer->output);
    motion_notify(0);
}

void unmaplayersurface(struct client *c)
{
    struct container *sel_container = get_focused_container(selected_monitor);
    c->surface.layer->mapped = 0;
    if (get_wlrsurface(c) == server.seat->keyboard_state.focused_surface)
        focus_container(sel_container, FOCUS_NOOP);
    motion_notify(0);
}

void unmaplayersurfacenotify(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, unmap);
    unmaplayersurface(c);
    container_damage_whole(c->con);
}

void destroylayersurfacenotify(struct wl_listener *listener, void *data)
{
    printf("destroy layersurface\n");
    struct client *c = wl_container_of(listener, c, destroy);

    if (c->surface.layer->mapped)
        unmaplayersurface(c);
    remove_in_composed_list(&server.layer_visual_stack_lists, cmp_ptr, c->con);
    destroy_container(c->con);
    wl_list_remove(&c->destroy.link);
    wl_list_remove(&c->map.link);
    wl_list_remove(&c->unmap.link);
    if (c->surface.layer->output) {
        struct monitor *m = c->surface.layer->output->data;
        if (m)
            arrangelayers(m);
        c->surface.layer->output = NULL;
    }
    free(c);
}

void commitlayersurfacenotify(struct wl_listener *listener, void *data)
{
    printf("commit\n");
    struct client *c = wl_container_of(listener, c, commit);
    struct wlr_layer_surface_v1 *wlr_layer_surface = c->surface.layer;
    struct wlr_output *wlr_output = wlr_layer_surface->output;

    if (!wlr_output)
        return;

    struct monitor *m = wlr_output->data;
    arrangelayers(m);
    struct container *con = c->con;
    container_damage_part(con);

    if (c->surface.layer->current.layer != wlr_layer_surface->current.layer) {
        remove_in_composed_list(&server.layer_visual_stack_lists, cmp_ptr, con);
        wlr_list_insert(get_layer_list(wlr_layer_surface->current.layer), 0, con);
    }
    arrange();
}

struct wlr_list *get_layer_list(enum zwlr_layer_shell_v1_layer layer)
{
    struct wlr_list *new_list = NULL;
    switch (layer) {
        case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
            new_list = server.layer_visual_stack_lists.items[3];
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
            new_list = server.layer_visual_stack_lists.items[2];
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
            new_list = server.layer_visual_stack_lists.items[1];
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
            new_list = server.layer_visual_stack_lists.items[0];
            break;
    }
    return new_list;
}

void arrangelayers(struct monitor *m)
{
    struct wlr_box usable_area = m->geom;
    uint32_t layers_above_shell[] = {
        ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
        ZWLR_LAYER_SHELL_V1_LAYER_TOP,
    };
    struct wlr_keyboard *kb = wlr_seat_get_keyboard(server.seat);

    // Arrange exclusive surfaces from top->bottom

    arrangelayer(m, &server.layer_visual_stack_overlay,
            &usable_area, 1);
    arrangelayer(m, &server.layer_visual_stack_top,
            &usable_area, 1);
    arrangelayer(m, &server.layer_visual_stack_bottom,
            &usable_area, 1);
    arrangelayer(m, &server.layer_visual_stack_background,
            &usable_area, 1);

    if (memcmp(&usable_area, &m->root->geom, sizeof(struct wlr_box))) {
        m->root->geom = usable_area;
        arrange(m);
    }

    // Arrange non-exlusive surfaces from top->bottom
    arrangelayer(m, &server.layer_visual_stack_overlay,
            &usable_area, 0);
    arrangelayer(m, &server.layer_visual_stack_top,
            &usable_area, 0);
    arrangelayer(m, &server.layer_visual_stack_bottom,
            &usable_area, 0);
    arrangelayer(m, &server.layer_visual_stack_background,
            &usable_area, 0);

    // Find topmost keyboard interactive layer, if such a layer exists
    for (size_t i = 0; i < LENGTH(layers_above_shell); i++) {
        struct wlr_list *new_list = get_layer_list(layers_above_shell[i]);
        int len = new_list->length;
        for (int j = len-1; j >= 0; j--) {
            struct client *layersurface = new_list->items[j];
            if (layersurface->surface.layer->current.keyboard_interactive &&
                    layersurface->surface.layer->mapped) {
                // Deactivate the focused client.
                // TODO fix this
                focus_container(NULL, FOCUS_NOOP);
                wlr_seat_keyboard_notify_enter(server.seat,
                        get_wlrsurface(layersurface),
                        kb->keycodes, kb->num_keycodes,
                        &kb->modifiers);
                return;
            }
        }
    }
}


void arrangelayer(struct monitor *m, struct wlr_list *list, struct wlr_box *usable_area, int exclusive)
{
    struct wlr_box full_area = m->geom;

    for (int i = 0; i < list->length; i++) {
        struct container *con = list->items[i];
        struct wlr_layer_surface_v1 *wlr_layer_surface = con->client->surface.layer;
        struct wlr_layer_surface_v1_state *state = &wlr_layer_surface->current;
        struct wlr_box bounds;
        struct wlr_box box = {
            .width = state->desired_width,
            .height = state->desired_height
        };
        const uint32_t both_horiz = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
            | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
        const uint32_t both_vert = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
            | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;

        if (exclusive != (state->exclusive_zone > 0))
            continue;

        bounds = state->exclusive_zone == -1 ? full_area : *usable_area;

        // Horizontal axis
        if ((state->anchor & both_horiz) && box.width == 0) {
            box.x = bounds.x;
            box.width = bounds.width;
        } else if ((state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT)) {
            box.x = bounds.x;
        } else if ((state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT)) {
            box.x = bounds.x + (bounds.width - box.width);
        } else {
            box.x = bounds.x + ((bounds.width / 2) - (box.width / 2));
        }
        // Vertical axis
        if ((state->anchor & both_vert) && box.height == 0) {
            box.y = bounds.y;
            box.height = bounds.height;
        } else if ((state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP)) {
            box.y = bounds.y;
        } else if ((state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM)) {
            box.y = bounds.y + (bounds.height - box.height);
        } else {
            box.y = bounds.y + ((bounds.height / 2) - (box.height / 2));
        }
        // Margin
        if ((state->anchor & both_horiz) == both_horiz) {
            box.x += state->margin.left;
            box.width -= state->margin.left + state->margin.right;
        } else if ((state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT)) {
            box.x += state->margin.left;
        } else if ((state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT)) {
            box.x -= state->margin.right;
        }
        if ((state->anchor & both_vert) == both_vert) {
            box.y += state->margin.top;
            box.height -= state->margin.top + state->margin.bottom;
        } else if ((state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP)) {
            box.y += state->margin.top;
        } else if ((state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM)) {
            box.y -= state->margin.bottom;
        }
        if (box.width < 0 || box.height < 0) {
            wlr_layer_surface_v1_close(wlr_layer_surface);
            continue;
        }
        con->geom = box;

        if (state->exclusive_zone > 0)
            apply_exclusive(usable_area, state->anchor, state->exclusive_zone,
                    state->margin.top, state->margin.right,
                    state->margin.bottom, state->margin.left);
        wlr_layer_surface_v1_configure(wlr_layer_surface, box.width, box.height);
    }
}

void apply_exclusive(struct wlr_box *usable_area,
        uint32_t anchor, int32_t exclusive,
        int32_t margin_top, int32_t margin_right,
        int32_t margin_bottom, int32_t margin_left) {
    struct edge edges[] = {
        { // Top
            .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP,
            .anchor_triplet = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP,
            .positive_axis = &usable_area->y,
            .negative_axis = &usable_area->height,
            .margin = margin_top,
        },
        { // Bottom
            .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
            .anchor_triplet = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
            .positive_axis = NULL,
            .negative_axis = &usable_area->height,
            .margin = margin_bottom,
        },
        { // Left
            .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT,
            .anchor_triplet = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
            .positive_axis = &usable_area->x,
            .negative_axis = &usable_area->width,
            .margin = margin_left,
        },
        { // Right
            .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
            .anchor_triplet = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
            .positive_axis = NULL,
            .negative_axis = &usable_area->width,
            .margin = margin_right,
        }
    };
    for (size_t i = 0; i < LENGTH(edges); i++) {
        if ((anchor == edges[i].singular_anchor || anchor == edges[i].anchor_triplet)
                && exclusive + edges[i].margin > 0) {
            if (edges[i].positive_axis)
                *edges[i].positive_axis += exclusive + edges[i].margin;
            if (edges[i].negative_axis)
                *edges[i].negative_axis -= exclusive + edges[i].margin;
            break;
        }
    }
}
