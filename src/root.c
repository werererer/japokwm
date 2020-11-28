#include "root.h"
#include "tagset.h"
#include "client.h"
#include "utils/coreUtils.h"

struct root root = {.consider_layer_shell = true};

// TODO: documentation?
static void set_layer_shell(struct client *c)
{
    c->geom.x = 0;
    c->geom.y = 0;
    if (c->surface.layer->current.desired_width) {
        c->geom.width = c->surface.layer->current.desired_width;
    }
    else {
        c->geom.width = selected_monitor->wlr_output->width;
    }

    if (c->surface.layer->current.desired_height) {
        c->geom.height = c->surface.layer->current.desired_height;
    }
    else {
        c->geom.height = selected_monitor->wlr_output->height;
    }
}

// TODO: Reduce side effects
void set_root_area(struct monitor *m)
{
    root.w = m->m;
    int maxWidth = 0, maxHeight = 0;
    struct client *c;
    wl_list_for_each(c, &layerstack, llink) {
        set_layer_shell(c);
        // if desired_width/height == 0 they are fullscreen and have no effect
        maxWidth = MAX(maxWidth, c->surface.layer->current.desired_width);
        maxHeight = MAX(maxHeight, c->surface.layer->current.desired_height);
        // move the current window barely out of view
        if (!root.consider_layer_shell) {
            c->geom.x -= maxWidth;
            c->geom.y -= maxHeight;
        }
    }
    if (root.consider_layer_shell) {
        root.w.x += maxWidth;
        root.w.width -= maxWidth;
        root.w.y += maxHeight;
        root.w.height -= maxHeight;
    }
}
