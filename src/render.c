#include "render.h"

#include <wayland-server.h>
#include <wayland-server-core.h>

#include "container.h"
#include "client.h"
#include "tag.h"
#include "monitor.h"
#include "layout.h"

static void surface_handle_destroy(struct wl_listener *listener, void *data)
{
    struct scene_surface *surface = wl_container_of(listener, surface, destroy);
    wlr_scene_node_destroy(&surface->scene_surface->buffer->node);
    for (int i = 0; i < BORDER_COUNT; i++) {
        struct wlr_scene_rect *border = surface->borders[i];
        wlr_scene_node_destroy(&border->node);
    }
    wl_list_remove(&surface->destroy.link);
    free(surface);
}

static void surface_handle_commit(struct wl_listener *listener, void *data) {
    struct scene_surface *surface = wl_container_of(listener, surface, commit);

    struct client *c = wlr_surface_get_client(surface->wlr);
    struct container *con = c->con;

    if (!con->has_border) {
        for (int i = 0; i < BORDER_COUNT; i++) {
            struct wlr_scene_rect *border = surface->borders[i];
            wlr_scene_node_set_enabled(&border->node, false);
        }
    }

    enum wlr_edges hidden_edges = WLR_EDGE_NONE;
    struct tag *tag = monitor_get_active_tag(m);
    struct layout *lt = tag_get_layout(tag);
    if (lt->options->smart_hidden_edges) {
        if (tag->visible_con_set->tiled_containers->len <= 1) {
            hidden_edges = container_update_hidden_edges(con, borders,
            lt->options->hidden_edges);
        }
    } else {
        hidden_edges = container_update_hidden_edges(con, borders,
        lt->options->hidden_edges);
    }


    struct wlr_box *borders = (struct wlr_box[4]) {
        container_get_current_border_geom(con, WLR_EDGE_TOP),
        container_get_current_border_geom(con, WLR_EDGE_BOTTOM),
        container_get_current_border_geom(con, WLR_EDGE_LEFT),
        container_get_current_border_geom(con, WLR_EDGE_RIGHT),
    };

    struct monitor *m = container_get_monitor(con);
    struct container *sel = monitor_get_focused_container(m);
    const struct color color = (con == sel) ? lt->options->focus_color :
    lt->options->border_color;

    for (int i = 0; i < BORDER_COUNT; i++) {
        struct wlr_scene_rect *border = surface->borders[i];
        struct wlr_box geom = borders[i];

        wlr_scene_node_set_position(&border->node, geom.x, geom.y);
        wlr_scene_rect_set_size(border, geom.width, geom.height);

        float border_color[4];
        color_to_wlr_color(border_color, color);
        wlr_scene_rect_set_color(border, border_color);
    }
}

// static void render_borders(struct container *con, struct monitor *m,
// pixman_region32_t *output_damage)
// {
//     // TODO: reimplement me
//     if (!con->has_border)
//         return;
//
//     enum wlr_edges hidden_edges = WLR_EDGE_NONE;
//     struct tag *tag = monitor_get_active_tag(m);
//     struct layout *lt = tag_get_layout(tag);
//     if (lt->options->smart_hidden_edges) {
//         if (tag->visible_con_set->tiled_containers->len <= 1) {
//             hidden_edges = container_update_hidden_edges(con, borders,
//             lt->options->hidden_edges);
//         }
//     } else {
//         hidden_edges = container_update_hidden_edges(con, borders,
//         lt->options->hidden_edges);
//     }
//
//     /* Draw window borders */
//     struct container *sel = monitor_get_focused_container(m);
//     const struct color color = (con == sel) ? lt->options->focus_color :
//     lt->options->border_color;
//     for (int i = 0; i < 4; i++) {
//         if ((hidden_edges & (1 << i)) == 0) {
//             struct wlr_box border = borders[i];
//             double ox = border.x;
//             double oy = border.y;
//             wlr_output_layout_output_coords(server.output_layout,
//             m->wlr_output, &ox, &oy); struct wlr_box obox = {
//                 .x = ox,
//                 .y = oy,
//                 .width = border.width,
//                 .height = border.height,
//             };
//             scale_box(&obox, m->wlr_output->scale);
//             render_rect(m, output_damage, &obox, color);
//         }
//     }
// }

void server_handle_new_surface(struct wl_listener *listener, void *data)
{
    printf("create notify new surface\n");
    struct server *server = wl_container_of(listener, server, new_surface);
    struct wlr_surface *wlr_surface = data;
    // wlr_surface->role_data

    struct scene_surface *surface = calloc(1, sizeof(struct scene_surface));
    surface->wlr = wlr_surface;
    surface->commit.notify = surface_handle_commit;
    wl_signal_add(&wlr_surface->events.commit, &surface->commit);
    surface->destroy.notify = surface_handle_destroy;
    wl_signal_add(&wlr_surface->events.destroy, &surface->destroy);

    surface->scene_surface =
        wlr_scene_surface_create(&server->scene->tree, wlr_surface);

    for (int i = 0; i < BORDER_COUNT; i++) {
        surface->borders[i] =
            wlr_scene_rect_create(&server->scene->tree,
                    0, 0, (float[4]){ 1.0f, 0.0f, 0.0f, 1 });
    }

    wlr_surface->data = surface;
}

struct client *wlr_surface_get_client(struct wlr_surface *wlr_surface)
{
    return wlr_surface->data;
}
