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
    memcpy(root->color, root_color, sizeof(float)*4);
    root->m = m;
    return root;
}

void destroy_root(struct root *root)
{
    free(root);
}

// TODO fix this to allow placement on all sides on screen
static struct wlr_box get_max_dimensions(struct root *root)
{
    struct wlr_box box = {.x = 0, .y = 0, .width = 0, .height = 0};

    struct container *con;
    wl_list_for_each(con, &layer_stack, llink) {
        if (!visibleon(con, root->m))
            continue;

        // desired_width and desired_height are == 0 if nothing is desired
        int desired_width = con->client->surface.layer->current.desired_width;
        int desired_height = con->client->surface.layer->current.desired_height;

        box.width = MAX(box.width, desired_width);
        box.height = MAX(box.height, desired_height);
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
        .x = ref.x,
        .y = ref.y,
        .width = desired_width != 0 ? desired_width : m->geom.width,
        .height = desired_height != 0 ? desired_height : m->geom.height,
    };

    resize(con, geom, false);
}


void set_root_area(struct root *root, struct wlr_box geom)
{
    root->geom = geom;

    struct wlr_box max_geom = get_max_dimensions(root);

    if (root->consider_layer_shell) {
        root->geom.x += max_geom.width;
        root->geom.width -= max_geom.width;
        root->geom.y += max_geom.height;
        root->geom.height -= max_geom.height;
    }

    struct container *con;
    wl_list_for_each(con, &layer_stack, llink) {
        if (!visibleon(con, root->m))
            continue;

        struct wlr_box geom = root->geom;
        geom.x = 0;
        geom.y = 0;
        configure_layer_shell_container_geom(con, geom);

        // move the current window barely out of view
        if (!root->consider_layer_shell) {
            con->geom.x = -max_geom.width;
            con->geom.y = -max_geom.height;
        }
    }
}

