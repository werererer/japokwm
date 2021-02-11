#include "root.h"
#include "client.h"
#include "container.h"
#include "utils/coreUtils.h"
#include "tile/tileUtils.h"
#include "parseConfig.h"

struct root *create_root(struct monitor *m)
{
    struct root *root = calloc(1, sizeof(struct root));
    root->consider_layer_shell = true;
    root->m = m;
    return root;
}

void destroy_root(struct root *root)
{
    free(root);
}

// TODO fix this to allow placement on all sides on screen
static struct wlr_box fit_root_area(struct root *root)
{
    struct wlr_box d_box = root->m->geom;
    struct wlr_box box = root->geom;

    struct container *con;
    wl_list_for_each(con, &layer_stack, llink) {
        if (!existon(con, root->m->ws[0]))
            continue;

        struct client *c = con->client;
        struct wlr_layer_surface_v1_state *current = &c->surface.layer->current;
        int anchor = current->anchor;

        // desired_width and desired_height are == 0 if nothing is desired
        int desired_width = current->desired_width;
        int desired_height = current->desired_height;

        // resize the root area
        if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
            int diff_height = d_box.y - (d_box.y - box.y);
            box.y = MAX(diff_height, desired_height);
            box.height = d_box.height - box.y;
        }
        if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) {
            int diff_height = (d_box.y + d_box.height) - (box.y + box.height);
            box.height = d_box.height - MAX(diff_height, desired_height);
        }
        if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) {
            int diff_width = box.x - d_box.x;
            box.x = MAX(diff_width, desired_width);
            box.width = d_box.width - box.x;
        }
        if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
            int diff_width = (d_box.x + d_box.width) - (box.x + box.width);
            box.width = d_box.width - MAX(diff_width, desired_width);
        }
    }

    return box;
}

static void configure_layer_shell_container_geom(struct container *con, struct wlr_box ref)
{
    if (!con->client)
        return;
    if (con->client->type != LAYER_SHELL)
        return;

    struct monitor *m = con->m;
    int desired_width = con->client->surface.layer->current.desired_width;
    int desired_height = con->client->surface.layer->current.desired_height;

    struct wlr_box geom = {
        .x = ref.x + m->geom.x,
        .y = ref.y + m->geom.y,
        .width = desired_width != 0 ? desired_width : m->geom.width,
        .height = desired_height != 0 ? desired_height : m->geom.height,
    };

    resize(con, geom, false);
}

void set_root_color(struct root *root, float color[static 4])
{
    printf("set root color\n");
    memcpy(root->color, color, sizeof(float)*4);
}

void set_root_area(struct root *root, struct wlr_box geom)
{
    root->geom = geom;

    if (root->consider_layer_shell) {
        root->geom = fit_root_area(root);
    }

    // arrange layer stack based programs
    struct container *con;
    wl_list_for_each(con, &layer_stack, llink) {
        if (!existon(con, root->m->ws[0]))
            continue;

        struct monitor *m = root->m;
        struct client *c = con->client;
        struct wlr_layer_surface_v1_state *current = &c->surface.layer->current;
        int anchor = current->anchor;

        struct wlr_box geom = root->geom;
        geom.x = 0;
        geom.y = 0;

        if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP)
            geom.y = 0;
        if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT)
            geom.x = 0;
        if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM)
            /* geom.y = m->geom.height - con->geom.height; */
            geom.y = m->geom.height;
        if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT)
            /* geom.x = m->geom.width - con->geom.width; */
            geom.x = m->geom.width;

        configure_layer_shell_container_geom(con, geom);

        if (con->client->surface.layer->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND) {
            continue;
        }
        if (con->floating) {
            continue;
        }

        con->hidden = !root->consider_layer_shell;
    }
}

void root_damage_whole(struct root *root)
{
    wlr_output_damage_add_box(root->m->damage, &root->geom);
}
