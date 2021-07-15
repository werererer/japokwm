#include "root.h"

#include <string.h>

#include "client.h"
#include "container.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "tile/tileUtils.h"
#include "layer_shell.h"

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
    struct root *root = calloc(1, sizeof(struct root));
    root->consider_layer_shell = true;
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

static struct wlr_box fit_root_area(struct root *root)
{
    struct wlr_box d_box = root->m->geom;
    struct wlr_box box = root->geom;

    for (int i = 0; i < length_of_composed_list(server.layer_visual_stack_lists); i++) {
        struct container *con = get_in_composed_list(server.layer_visual_stack_lists, i);

        /* struct tagset *tagset = monitor_get_active_tagset(root->m); */
        /* if (!exist_on(tagset, con)) */
        /*     continue; */

        struct wlr_layer_surface_v1_state *current = &con->client->surface.layer->current;
        int anchor = current->anchor;

        // desired_width and desired_height are == 0 if nothing is desired
        int desired_width = current->desired_width;
        int desired_height = current->desired_height;

        // resize the root area
        if (equals_anchor(&anchors.top, anchor)) {
            box.x = d_box.x;
            int diff_height = d_box.y - (d_box.y - box.y);
            box.y = MAX(diff_height, desired_height);
            box.height = d_box.height - box.y;
        }
        if (equals_anchor(&anchors.bottom, anchor)) {
            int diff_height = (d_box.y + d_box.height) - (box.y + box.height);
            box.height = d_box.height - MAX(diff_height, desired_height);
        }
        if (equals_anchor(&anchors.left, anchor)) {
            int diff_width = box.x - d_box.x;
            box.x = MAX(diff_width, desired_width);
            box.width = d_box.width - box.x;
        }
        if (equals_anchor(&anchors.right, anchor)) {
            int diff_width = (d_box.x + d_box.width) - (box.x + box.width);
            box.width = d_box.width - MAX(diff_width, desired_width);
        }
    }

    return box;
}

/* static void configure_layer_shell_container_geom(struct container *con, struct wlr_box ref) */
/* { */
/*     if (!con->client) */
/*         return; */
/*     if (con->client->type != LAYER_SHELL) */
/*         return; */

/*     struct monitor *m = container_get_monitor(con); */
/*     int desired_width = con->client->surface.layer->current.desired_width; */
/*     int desired_height = con->client->surface.layer->current.desired_height; */

/*     struct wlr_box geom = { */
/*         .x = ref.x + m->geom.x, */
/*         .y = ref.y + m->geom.y, */
/*         .width = desired_width != 0 ? desired_width : m->geom.width, */
/*         .height = desired_height != 0 ? desired_height : m->geom.height, */
/*     }; */

/*     resize(con, geom); */
/* } */

void set_root_color(struct root *root, float color[static 4])
{
    memcpy(root->color, color, sizeof(float)*4);
}

void set_root_geom(struct root *root, struct wlr_box geom)
{
    root->geom = geom;

    if (root->consider_layer_shell) {
        root->geom = fit_root_area(root);
    }
}

void root_damage_whole(struct root *root)
{
    if (!root)
        return;
    struct monitor *m = root->m;
    struct wlr_box geom = get_monitor_local_box(root->geom, m);
    wlr_output_damage_add_box(m->damage, &geom);
}

void set_bars_visible(struct monitor *m, bool visible)
{
    m->root->consider_layer_shell = visible;
    wlr_output_damage_add_whole(m->damage);
}

bool get_bars_visible(struct monitor *m)
{
    return m->root->consider_layer_shell;
}

void toggle_bars_visible(struct monitor *m)
{
    set_bars_visible(m, !get_bars_visible(m));
}
