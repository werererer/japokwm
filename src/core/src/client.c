#include "client.h"
#include <coreUtils.h>
#include <wayland-util.h>
#include "tile/tile.h"

//global variables
struct wl_list clients; /* tiling order */
struct wl_list focus_stack;  /* focus order */
struct wl_list stack;   /* stacking z-order */
struct wlr_output_layout *output_layout;
struct wlr_box sgeom;
struct wl_list mons;
Monitor *selMon = NULL;
Atom netatom[NetLast];

/* function implementations */
void applybounds(Client *c, struct wlr_box bbox)
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

void
updatewindowtype(Client *c)
{
	size_t i;
	for (i = 0; i < c->surface.xwayland->window_type_len; i++)
		if (c->surface.xwayland->window_type[i] == netatom[NetWMWindowTypeDialog] ||
				c->surface.xwayland->window_type[i] == netatom[NetWMWindowTypeSplash] ||
				c->surface.xwayland->window_type[i] == netatom[NetWMWindowTypeToolbar] ||
				c->surface.xwayland->window_type[i] == netatom[NetWMWindowTypeUtility])
			c->isfloating = 1;
}

void applyrules(Client *c)
{
    const char *appid, *title;
    unsigned int i, newtags = 0;
    const Rule *r;
    Monitor *m; 
    /* rule matching */
    c->isfloating = false;
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
            updatewindowtype(c);
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
            c->isfloating = r->isfloating;
            newtags |= r->tags;
            i = 0;
            wl_list_for_each(m, &mons, link)
                if (r->monitor == i++)
                    selMon = m;
        }
    }
    setmon(c, selMon, newtags);
}

Client *selClient()
{
    Client *c = wl_container_of(focus_stack.next, c, flink);
    if (!visibleon(c, selMon))
        return NULL;
    return c;
}

struct wlr_surface *getWlrSurface(Client *c)
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

bool visibleon(Client *c, Monitor *m)
{
    if (m) {
        unsigned int tags = c->tags & m->tagset[m->seltags];
        bool intersect = wlr_output_layout_intersects(
                            output_layout, m->wlr_output, &c->geom);
        return tags || !intersect;
    }
    return false;
}

