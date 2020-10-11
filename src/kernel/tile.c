#include "tile.h"
#include <coreUtils.h>
#include <julia.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <tileUtils.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include "client.h"

struct wlr_list *containers;

void swap(struct wlr_box *c1, struct wlr_box *c2) {
    struct wlr_box *c;
    *c = *c1;
    *c1 = *c2;
    *c2 = *c;
}

static void push(struct wlr_box *c)
{
    wlr_list_push(containers, c);
}

static void add(struct wlr_box *c, size_t i)
{
    wlr_list_insert(containers, i, c);
}

static void del(size_t i)
{
    wlr_list_del(containers, i);
}

void addBox(int x, int y, int w, int h)
{
    struct wlr_box box;
    box.x = x;
    box.y = y;
    box.width = w;
    box.height = h;
    push(&box);
}

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
    /*
     * Note that I took some shortcuts here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at
     * the new size, then commit any movement that was prepared.
     */
    struct wlr_box bbox = interact ? sgeom : c->mon->w;
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

