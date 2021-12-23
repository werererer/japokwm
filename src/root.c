#include "root.h"

#include <string.h>
#include <glib.h>

#include "client.h"
#include "container.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "tile/tileUtils.h"
#include "layer_shell.h"
#include "tag.h"

struct anchor {
    uint32_t singular_anchor;
    uint32_t anchor_triplet;
};

struct anchors {
    struct anchor top;
    struct anchor bottom;
    struct anchor left;
    struct anchor right;
};

static const struct anchors anchors = {
    .top = {
        .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP,
        .anchor_triplet =
            ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
    },
    .bottom = {
        .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
        .anchor_triplet =
            ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
    },
    .left = {
        .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT,
        .anchor_triplet =
            ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
    },
    .right = {
        .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
        .anchor_triplet =
            ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
    },
};


struct root *create_root(struct monitor *m, struct wlr_box geom)
{
    struct root *root = calloc(1, sizeof(*root));
    root->m = m;
    float color[4] = {0.0, 0.0, 0.0, 1.0};
    root->root_rect = wlr_scene_rect_create(&server.scene->node, m->geom.width, m->geom.height, color);

    set_root_geom(root, geom);
    return root;
}

void destroy_root(struct root *root)
{
    free(root);
}

static bool equals_anchor(const struct anchor *anchor, uint32_t anchor_value)
{
    bool is_anchor_triplet = anchor->anchor_triplet == anchor_value;
    bool is_singular_anchor = anchor->singular_anchor == anchor_value;
    return is_anchor_triplet || is_singular_anchor;
}

void set_root_color(struct root *root, struct color color)
{
    float root_color[4];
    color_to_wlr_color(root_color, color);
    wlr_scene_rect_set_color(root->root_rect, root_color);
    root->color = color;
}

void set_root_geom(struct root *root, struct wlr_box geom)
{
    root->geom = geom;

    wlr_scene_rect_set_size(root->root_rect, geom.width, geom.height);
}

void root_damage_whole(struct root *root)
{
    if (!root)
        return;
    struct monitor *m = root->m;
    struct wlr_box geom = get_monitor_local_box(root->geom, m);
    wlr_output_damage_add_box(m->damage, &geom);
}

static enum wlr_edges bar_get_direction(struct container *con)
{
    struct wlr_layer_surface_v1 *wlr_layer_surface = con->client->surface.layer;
    struct wlr_layer_surface_v1_state *state = &wlr_layer_surface->current;
    enum wlr_edges anchor = state->anchor;

    if (equals_anchor(&anchors.top, anchor)) {
        return WLR_EDGE_TOP;
    }
    if (equals_anchor(&anchors.bottom, anchor)) {
        return true;
        return WLR_EDGE_BOTTOM;
    }
    if (equals_anchor(&anchors.left, anchor)) {
        return WLR_EDGE_LEFT;
    }
    if (equals_anchor(&anchors.right, anchor)) {
        return WLR_EDGE_RIGHT;
    }
    return WLR_EDGE_NONE;
}

void bars_update_visiblitiy(struct tag *tag)
{
    enum wlr_edges visible_edges = tag->visible_bar_edges;
    for (int i = 0; i < length_of_composed_list(server.layer_visual_stack_lists); i++) {
        struct container *con = get_in_composed_list(server.layer_visual_stack_lists, i);

        if (!container_is_bar(con))
            continue;

        enum wlr_edges bar_dir = bar_get_direction(con);
        bool is_visible = visible_edges & bar_dir;

        container_set_hidden_at_tag(con, !is_visible, tag);
    }

    struct monitor *m = tag_get_monitor(tag);
    arrange_layers(m);
}

void toggle_bars_visible(struct tag *tag, enum wlr_edges direction)
{
    tag->visible_bar_edges ^= direction;
    bars_update_visiblitiy(tag);
}

void set_bars_visible(struct tag *tag, enum wlr_edges direction)
{
    tag->visible_bar_edges = direction;
    bars_update_visiblitiy(tag);
}
