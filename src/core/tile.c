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
