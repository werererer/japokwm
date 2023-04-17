#include "render.h"

#include <wayland-server.h>
#include <wayland-server-core.h>

#include "container.h"
#include "client.h"
#include "tag.h"
#include "monitor.h"
#include "layout.h"
#include "list_sets/container_stack_set.h"
#include "root.h"

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

// TODO refactor the name it doesn't represent what this does perfectly
// returns the newly accquired hidden edges
static enum wlr_edges container_update_hidden_edges(struct container *con, struct wlr_box *borders, enum wlr_edges hidden_edges)
{
    struct monitor *m = container_get_monitor(con);

    enum wlr_edges containers_hidden_edges = WLR_EDGE_NONE;
    struct wlr_box con_geom = container_get_current_geom(con);
    // int border_width = container_get_border_width(con);
    // hide edges if needed
    if (hidden_edges & WLR_EDGE_LEFT) {
        if (con_geom.x == m->root->geom.x) {
            containers_hidden_edges |= WLR_EDGE_LEFT;
        }
    }
    if (hidden_edges & WLR_EDGE_RIGHT) {
        if (is_approx_equal(con_geom.x + con_geom.width, m->root->geom.x + m->root->geom.width, 3)) {
            containers_hidden_edges |= WLR_EDGE_RIGHT;
        }
    }
    if (hidden_edges & WLR_EDGE_TOP) {
        if (con_geom.y == m->root->geom.y) {
            containers_hidden_edges |= WLR_EDGE_TOP;
        }
    }
    if (hidden_edges & WLR_EDGE_BOTTOM) {
        if (is_approx_equal(con_geom.y + con_geom.height, m->root->geom.y + m->root->geom.height, 3)) {
            containers_hidden_edges |= WLR_EDGE_BOTTOM;
        }
    }

    container_set_hidden_edges(con, containers_hidden_edges);
    return containers_hidden_edges;
}

static void surface_handle_commit(struct wl_listener *listener, void *data) {
    struct scene_surface *surface = wl_container_of(listener, surface, commit);

    struct client *c = wlr_surface_get_client(surface->wlr);
    struct container *con = c->con;

    wlr_scene_node_set_enabled(&surface->scene_surface->node, true);

    if (!con->has_border) {
        for (int i = 0; i < BORDER_COUNT; i++) {
            struct wlr_scene_rect *border = surface->borders[i];
            wlr_scene_node_set_enabled(&border->node, false);
        }
        return;
    }

    struct wlr_box *borders = (struct wlr_box[4]) {
        container_get_current_border_geom(con, WLR_EDGE_TOP),
        container_get_current_border_geom(con, WLR_EDGE_BOTTOM),
        container_get_current_border_geom(con, WLR_EDGE_LEFT),
        container_get_current_border_geom(con, WLR_EDGE_RIGHT),
    };

    enum wlr_edges hidden_edges = WLR_EDGE_NONE;
    struct monitor *m = container_get_monitor(con);
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

    struct container *sel = monitor_get_focused_container(m);
    const struct color color = (con == sel) ? lt->options->focus_color :
    lt->options->border_color;

    for (int i = 0; i < BORDER_COUNT; i++) {
        struct wlr_scene_rect *border = surface->borders[i];
        struct wlr_box geom = borders[i];

        bool is_hidden = hidden_edges & (1 << i);
        // hide or show the border
        wlr_scene_node_set_enabled(&border->node, !is_hidden);
        if (is_hidden) {
            continue;
        }

        wlr_scene_node_set_position(&border->node, geom.x, geom.y);
        wlr_scene_rect_set_size(border, geom.width, geom.height);

        float border_color[4];
        color_to_wlr_color(border_color, color);
        wlr_scene_rect_set_color(border, border_color);
    }
}


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
