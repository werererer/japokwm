#include "tile/tile.h"
#include <coreUtils.h>
#include <julia.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include "tile/tileUtils.h"
#include "client.h"

struct wlr_list *containers;

void swap(struct wlr_box *b1, struct wlr_box *b2) {
    struct wlr_box *b, box;
    b = &box;
    *b = *b1;
    *b1 = *b2;
    *b2 = *b;
}

static void push(struct wlr_box *c)
{
    wlr_list_push(containers, c);
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
