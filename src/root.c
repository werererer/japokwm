#include "root.h"
#include "client.h"
#include "container.h"
#include "tagset.h"
#include "utils/coreUtils.h"

struct root root = {.consider_layer_shell = true};

// TODO: documentation?
static void set_layer_shell(struct container *con)
{
    con->geom.x = 0;
    con->geom.y = 0;
    if (con->client->surface.layer->current.desired_width) {
        con->geom.width = con->client->surface.layer->current.desired_width;
    } else {
        con->geom.width = selected_monitor->wlr_output->width;
    }

    if (con->client->surface.layer->current.desired_height) {
        con->geom.height = con->client->surface.layer->current.desired_height;
    }
    else {
        con->geom.height = selected_monitor->wlr_output->height;
    }
}

// TODO: Reduce side effects
void set_root_area(struct monitor *m)
{
    root.w = m->geom;
    int maxWidth = 0, maxHeight = 0;
    struct container *con;
    wl_list_for_each(con, &layerstack, llink) {
        set_layer_shell(con);
        // if desired_width/height == 0 they are fullscreen and have no effect
        maxWidth = MAX(maxWidth, con->client->surface.layer->current.desired_width);
        maxHeight = MAX(maxHeight, con->client->surface.layer->current.desired_height);
        // move the current window barely out of view
        if (!root.consider_layer_shell) {
            con->geom.x -= maxWidth;
            con->geom.y -= maxHeight;
        }
    }
    if (root.consider_layer_shell) {
        root.w.x += maxWidth;
        root.w.width -= maxWidth;
        root.w.y += maxHeight;
        root.w.height -= maxHeight;
    }
}
