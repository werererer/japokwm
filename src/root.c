#include "root.h"

#include <string.h>
#include <glib.h>

#include "client.h"
#include "container.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "tile/tileUtils.h"
#include "layer_shell.h"
#include "workspace.h"

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
    root->color = color;
}

void set_root_geom(struct root *root, struct wlr_box geom)
{
    root->geom = geom;
}

void root_damage_whole(struct root *root)
{
    if (!root)
        return;
    struct monitor *m = root->m;
    struct wlr_box geom = get_monitor_local_box(root->geom, m);
    wlr_output_damage_add_box(m->damage, &geom);
}

static bool is_bar_at_direction(struct container *con, enum wlr_direction direction)
{
    struct wlr_layer_surface_v1 *wlr_layer_surface = con->client->surface.layer;
    struct wlr_layer_surface_v1_state *state = &wlr_layer_surface->current;

    if (direction & WLR_DIRECTION_UP) {
        if (equals_anchor(&anchors.top, state->anchor)) {
            return true;
        }
    }
    if (direction & WLR_DIRECTION_DOWN) {
        if (equals_anchor(&anchors.bottom, state->anchor)) {
            return true;
        }
    }
    if (direction & WLR_DIRECTION_LEFT) {
        if (equals_anchor(&anchors.left, state->anchor)) {
            return true;
        }
    }
    if (direction & WLR_DIRECTION_RIGHT) {
        if (equals_anchor(&anchors.right, state->anchor)) {
            return true;
        }
    }
    return false;
}

void set_bars_visible(struct workspace *ws, bool visible, enum wlr_direction direction)
{
    ws->consider_layer_shell = visible;
    for (int i = 0; i < length_of_composed_list(server.layer_visual_stack_lists); i++) {
        struct container *con = get_in_composed_list(server.layer_visual_stack_lists, i);

        if (!container_is_bar(con))
            continue;
        if (!is_bar_at_direction(con, direction))
            continue;

        container_set_hidden_at_workspace(con, !visible, ws);
    }

    struct monitor *m = workspace_get_monitor(ws);
    arrange_layers(m);
}

bool get_bars_visible(struct workspace *ws)
{
    return ws->consider_layer_shell;
}

void toggle_bars_visible(struct workspace *ws, enum wlr_direction direction)
{
    set_bars_visible(ws, !get_bars_visible(ws), direction);
}
