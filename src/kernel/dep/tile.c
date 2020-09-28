#include "tile.h"
#include <stdio.h>

void
tile(Monitor *m)
{
    printf("tile\n");
    unsigned int i, n = 0, h, mw, my, ty;
    Client *c;

    wl_list_for_each(c, &clients, link)
        if (VISIBLEON(c, m) && !c->isfloating)
            n++;
    if (n == 0)
        return;

    if (n > m->nmaster)
        mw = m->nmaster ? m->w.width * m->mfact : 0;
    else
        mw = m->w.width;
    i = my = ty = 0;
    wl_list_for_each(c, &clients, link) {
        if (!VISIBLEON(c, m) || c->isfloating)
            continue;
        if (i < m->nmaster) {
            h = (m->w.height - my) / (MIN(n, m->nmaster) - i);
            resize(c, m->w.x, m->w.y + my, mw, h, 0);
            my += c->geom.height;
        } else {
            h = (m->w.height - ty) / (n - i);
            resize(c, m->w.x + mw, m->w.y + ty, m->w.width - mw, h, 0);
            ty += c->geom.height;
        }
        i++;
    }
}

void
monocle(Monitor *m)
{
    Client *c;

    wl_list_for_each(c, &clients, link) {
        if (!VISIBLEON(c, m) || c->isfloating)
            continue;
        resize(c, m->w.x, m->w.y, m->w.width, m->w.height, 0);
    }
}

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
    /*
     * Note that I took some shortcuts here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at
     * the new size, then commit any movement that was prepared.
     */
    struct wlr_box *bbox = interact ? &sgeom : &c->mon->w;
    c->geom.x = x;
    c->geom.y = y;
    c->geom.width = w;
    c->geom.height = h;
    applybounds(c, bbox);
    /* wlroots makes this a no-op if size hasn't changed */
#ifdef XWAYLAND
    if (c->type != XDGShell)
        wlr_xwayland_surface_configure(c->surface.xwayland,
                c->geom.x, c->geom.y,
                c->geom.width - 2 * c->bw, c->geom.height - 2 * c->bw);
    else
#endif
        c->resize = wlr_xdg_toplevel_set_size(c->surface.xdg,
                c->geom.width - 2 * c->bw, c->geom.height - 2 * c->bw);
}

