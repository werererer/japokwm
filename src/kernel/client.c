#include "client.h"
#include "tile.h"

//global variables
struct wl_list clients; /* tiling order */
struct wl_list focus_stack;  /* focus order */
struct wl_list stack;   /* stacking z-order */
struct wlr_output_layout *output_layout;
struct wlr_box sgeom;
struct wl_list mons;
Monitor *selmon = NULL;

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

void applyrules(Client *c)
{
    const char *appid, *title;
    unsigned int i, newtags = 0;
    const Rule *r;
    Monitor *m; 
    /* rule matching */
    c->isfloating = 0;
#ifdef XWAYLAND
    if (c->type != XDGShell) {
        updatewindowtype(c);
        appid = c->surface.xwayland->class;
        title = c->surface.xwayland->title;
    } else
#endif
    {
        appid = c->surface.xdg->toplevel->app_id;
        title = c->surface.xdg->toplevel->title;
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
                    selmon = m;
            printf("%i selmon: %i\n", __LINE__, selmon == NULL);
        }
    }
    setmon(c, selmon, newtags);
}

Client *selClient()
{
    struct wl_list f = focus_stack;
    Client *c = wl_container_of(focus_stack.next, c, flink);
    if (wl_list_empty(&focus_stack) || !visibleon(c, selmon))
        return NULL;
    return c;
}
