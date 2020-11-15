#include "root.h"
#include "tagset.h"
#include "client.h"

struct root root;

void setRootArea(struct monitor *m)
{
    root.w = m->m;

    struct client *c;
    wl_list_for_each(c, &layerStack, llink) {
        // if desired_width/height == 0 they are fullscreen and have no effect
        root.w.x += c->surface.layer->current.desired_width;
        root.w.y += c->surface.layer->current.desired_height;
    }
}
