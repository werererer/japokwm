#include "client.h"
#include <stdio.h>
#include <wayland-util.h>

#include "tile/tile.h"
#include "utils/coreUtils.h"

//global variables
struct wl_list clients; /* tiling order */
struct wl_list focus_stack;  /* focus order */
struct wl_list stack;   /* stacking z-order */
struct wlr_output_layout *output_layout;
struct wlr_box sgeom;

/* function implementations */
void applybounds(struct client *c, struct wlr_box bbox)
{
    /* set minimum possible */
    c->geom.width = MAX(1, c->geom.width);
    c->geom.height = MAX(1, c->geom.height);

    if (c->geom.x >= bbox.x + bbox.width)
        c->geom.x = bbox.x + bbox.width - c->geom.width;
    if (c->geom.y >= bbox.y + bbox.height)
        c->geom.y = bbox.y + bbox.height - c->geom.height;
    if (c->geom.x + c->geom.width + 2 * c->bw <= bbox.x)
        c->geom.x = bbox.x;
    if (c->geom.y + c->geom.height + 2 * c->bw <= bbox.y)
        c->geom.y = bbox.y;
}

void applyrules(struct client *c)
{
    const char *appid, *title;
    unsigned int i, newtags = 0;
    const struct rule *r;
    struct monitor *m; 
    /* rule matching */
    c->floating = false;
    switch (c->type) {
        case XDGShell:
            appid = c->surface.xdg->toplevel->app_id;
            title = c->surface.xdg->toplevel->title;
            break;
        case LayerShell:
            appid = "test";
            title = "test";
            break;
        case X11Managed:
        case X11Unmanaged:
            appid = c->surface.xwayland->class;
            title = c->surface.xwayland->title;
            break;
    }
    if (!appid)
        appid = "broken";
    if (!title)
        title = "broken";

    for (r = rules; r < END(rules); r++) {
        if ((!r->title || strstr(title, r->title))
                && (!r->id || strstr(appid, r->id))) {
            c->floating = r->floating;
            newtags |= r->tags;
            i = 0;
            wl_list_for_each(m, &mons, link)
                if (r->monitor == i++)
                    selMon = m;
        }
    }
    setmon(c, selMon, newtags);
}

struct client *selClient()
{
    if (wl_list_length(&focus_stack))
    {
        struct client *c = wl_container_of(focus_stack.next, c, flink);
        if (!visibleon(c, selMon))
            return NULL;
        else
            return c;
    } else {
        return NULL;
    }
}

struct wlr_surface *getWlrSurface(struct client *c)
{
    switch (c->type) {
        case XDGShell:
            return c->surface.xdg->surface;
            break;
        case LayerShell:
            return c->surface.layer->surface;
            break;
        case X11Managed:
        case X11Unmanaged:
            return c->surface.xwayland->surface;
        default:
            printf("wlr_surface is not supported: \n");
            return NULL;
    }
}

bool visibleon(struct client *c, struct monitor *m)
{
    if (m && c) {
        bool sameMon = c->mon == m;
        bool sameTag = c->mon->tagset.selTags[0] & (1 << m->tagset.focusedTag);
        return sameMon && sameTag;
    }
    return false;
}
