#include "layer_shell.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "monitor.h"
#include "popup.h"
#include "server.h"
#include "container.h"
#include "tile/tileUtils.h"
#include "render/render.h"
#include "input_manager.h"
#include "root.h"
#include "tagset.h"
#include "subsurface.h"

void create_notify_layer_shell(struct wl_listener *listener, void *data)
{
    struct wlr_layer_surface_v1 *wlr_layer_surface = data;

    if (!wlr_layer_surface->output) {
        struct monitor *m = server_get_selected_monitor();
        wlr_layer_surface->output = m->wlr_output;
    }

    union surface_t surface;
    surface.layer = wlr_layer_surface;
    struct client *client = create_client(LAYER_SHELL, surface);

    LISTEN(&wlr_layer_surface->surface->events.commit, &client->commit, commit_layer_surface_notify);
    LISTEN(&wlr_layer_surface->events.map, &client->map, map_layer_surface_notify);
    LISTEN(&wlr_layer_surface->events.unmap, &client->unmap, unmap_layer_surface_notify);
    LISTEN(&wlr_layer_surface->events.destroy, &client->destroy, destroy_layer_surface_notify);
    LISTEN(&wlr_layer_surface->events.new_popup, &client->new_popup, client_handle_new_popup);
    LISTEN(&wlr_layer_surface->surface->events.new_subsurface, &client->new_subsurface, handle_new_subsurface);

    struct monitor *m = wlr_layer_surface->output->data;
    client->m = m;
    struct container *con = create_container(client, m, false);

    add_container_to_tile(con);

    // Temporarily set the layer's current state to client_pending
    // so that we can easily arrange it
    struct wlr_layer_surface_v1_state old_state = wlr_layer_surface->current;
    wlr_layer_surface->current = wlr_layer_surface->pending;
    arrange_layers(m);
    wlr_layer_surface->current = old_state;
}

void map_layer_surface_notify(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, map);
    /* wlr_surface_send_enter(get_wlrsurface(c), c->surface.layer->output); */
    /* motion_notify(0); */
    debug_print("length of layer stack: %i\n", server.layer_visual_stack_bottom->len);
}

void unmap_layer_surface(struct client *c)
{
    /* struct container *sel_container = get_focused_container(selected_monitor); */
    /* c->surface.layer->mapped = 0; */
    /* if (get_wlrsurface(c) == server.seat->keyboard_state.focused_surface) */
    /*     focus_container(sel_container, FOCUS_NOOP); */
    /* motion_notify(0); */
}

void unmap_layer_surface_notify(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, unmap);
    unmap_layer_surface(c);
    container_damage_whole(c->con);
    arrange_layers(c->m);
}

void destroy_layer_surface_notify(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, destroy);

    if (c->surface.layer->mapped)
        unmap_layer_surface(c);
    remove_in_composed_list(server.layer_visual_stack_lists, cmp_ptr, c->con);

    wl_list_remove(&c->destroy.link);
    wl_list_remove(&c->map.link);
    wl_list_remove(&c->unmap.link);
    wl_list_remove(&c->new_popup.link);
    wl_list_remove(&c->commit.link);

    if (c->surface.layer->output) {
        struct monitor *m = c->surface.layer->output->data;
        arrange_layers(m);
        c->surface.layer->output = NULL;
    }

    remove_container_from_tile(c->con);

    destroy_container(c->con);
    destroy_client(c);
}

void commit_layer_surface_notify(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, commit);
    struct wlr_layer_surface_v1 *wlr_layer_surface = c->surface.layer;
    struct wlr_output *wlr_output = wlr_layer_surface->output;

    if (!wlr_output)
        return;

    struct monitor *m = wlr_output->data;
    struct container *con = c->con;
    container_damage_part(con);

    if (c->surface.layer->current.layer != wlr_layer_surface->current.layer) {
        remove_in_composed_list(server.layer_visual_stack_lists, cmp_ptr, con);
        g_ptr_array_insert(get_layer_list(m, wlr_layer_surface->current.layer), 0, con);
    }
}

bool layer_shell_is_bar(struct container *con)
{
    assert(con->client->type == LAYER_SHELL);


    struct wlr_layer_surface_v1 *wlr_layer_surface = con->client->surface.layer;
    struct wlr_layer_surface_v1_state *state = &wlr_layer_surface->current;

    bool is_exclusive = state->exclusive_zone >= 0;
    bool is_anchord_on_three_edges = cross_sum(state->anchor, 2) == 3;
    bool is_anchord_on_one_edge = cross_sum(state->anchor, 2) == 1;

    return is_exclusive && (is_anchord_on_one_edge || is_anchord_on_three_edges);
}

GPtrArray *get_layer_list(struct monitor *m, enum zwlr_layer_shell_v1_layer layer)
{
    GPtrArray *layer_list = NULL;
    switch (layer) {
        case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
            layer_list = g_ptr_array_index(server.layer_visual_stack_lists, 3);
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
            layer_list = g_ptr_array_index(server.layer_visual_stack_lists, 2);
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
            layer_list = g_ptr_array_index(server.layer_visual_stack_lists, 1);
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
            layer_list = g_ptr_array_index(server.layer_visual_stack_lists, 0);
            break;
    }
    return layer_list;
}

void arrange_layers(struct monitor *m)
{
    printf("arrange layers\n");
    if (!m)
        return;

    struct wlr_box usable_area = m->geom;
    uint32_t layers_above_shell[] = {
        ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
        ZWLR_LAYER_SHELL_V1_LAYER_TOP,
    };

    // Arrange exclusive surfaces from top->bottom
    arrangelayer(m, server.layer_visual_stack_overlay, &usable_area, true);
    arrangelayer(m, server.layer_visual_stack_top, &usable_area, true);
    arrangelayer(m, server.layer_visual_stack_bottom, &usable_area, true);
    arrangelayer(m, server.layer_visual_stack_background, &usable_area, true);

    set_root_geom(m->root, usable_area);
    arrange();

    // Arrange non-exlusive surfaces from top->bottom
    arrangelayer(m, server.layer_visual_stack_overlay, &usable_area, false);
    arrangelayer(m, server.layer_visual_stack_top, &usable_area, false);
    arrangelayer(m, server.layer_visual_stack_bottom, &usable_area, false);
    arrangelayer(m, server.layer_visual_stack_background, &usable_area, false);

    struct seat *seat = input_manager_get_default_seat();
    struct wlr_keyboard *kb = wlr_seat_get_keyboard(seat->wlr_seat);
    // Find topmost keyboard interactive layer, if such a layer exists
    for (size_t i = 0; i < LENGTH(layers_above_shell); i++) {
        GPtrArray *layer_list = get_layer_list(m, layers_above_shell[i]);
        for (int j = layer_list->len-1; j >= 0; j--) {
            struct container *con = g_ptr_array_index(layer_list, j);
            struct client *c = con->client;
            struct wlr_layer_surface_v1 *layer_surface = c->surface.layer;
            if (layer_surface->current.keyboard_interactive && layer_surface->mapped) {
                // Deactivate the focused client.
                // TODO fix this NULL is not supported in focus_container
                tag_this_focus_container(NULL);
                wlr_seat_keyboard_notify_enter(seat->wlr_seat,
                        get_wlrsurface(c),
                        kb->keycodes, kb->num_keycodes,
                        &kb->modifiers);
                return;
            }
        }
    }
}

void arrangelayer(struct monitor *m, GPtrArray *array, struct wlr_box *usable_area, bool exclusive)
{
    struct wlr_box full_area = m->geom;

    for (int i = 0; i < array->len; i++) {
        struct container *con = g_ptr_array_index(array, i);

        if (!tagset_visible_on(m, con))
            continue;

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

        bool is_exclusive = (state->exclusive_zone > 0);
        if (exclusive != is_exclusive)
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
            wlr_layer_surface_v1_destroy(wlr_layer_surface);
            continue;
        }
        // TODO: is that correct?
        container_set_current_geom(con, box);

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
