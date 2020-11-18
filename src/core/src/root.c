#include "root.h"
#include "tagset.h"
#include "client.h"
#include "utils/coreUtils.h"

struct root root;

// TODO: Reduce side effects
void setRootArea(struct monitor *m)
{
    root.w = m->m;
    int maxWidth = 0, maxHeight = 0;
    struct client *c;
    wl_list_for_each(c, &layerStack, llink) {
        // if desired_width/height == 0 they are fullscreen and have no effect
        maxWidth = MAX(maxWidth, c->surface.layer->current.desired_width);
        maxHeight = MAX(maxHeight, c->surface.layer->current.desired_height);
    }
    root.w.width -= root.w.x += maxWidth;
    root.w.height -= root.w.y += maxHeight;
}
