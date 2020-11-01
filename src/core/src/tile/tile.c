#include "tile/tile.h"
#include <julia.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>

#include "utils/coreUtils.h"
#include "tile/tileUtils.h"
#include "client.h"

void swap(struct wlr_box *b1, struct wlr_box *b2) {
    struct wlr_box *b, box;
    b = &box;
    *b = *b1;
    *b1 = *b2;
    *b2 = *b;
}

/* static void push(struct wlr_box *c) */
/* { */
/*     wlr_list_push(containers, c); */
/* } */

/* static void del(size_t i) */
/* { */
/*     wlr_list_del(containers, i); */
/* } */

void addBox(int x, int y, int w, int h)
{
    // struct wlr_box box;
    /* box.x = x; */
    /* box.y = y; */
    /* box.width = w; */
    /* box.height = h; */
    //push(&box);
}

void focusOnStack(int i)
{
    struct client *c, *sel = selClient();
    if (!sel)
        return;
    if (i > 0) {
        int j = 1;
        wl_list_for_each(c, &sel->link, link) {
            if (visibleon(c, selMon))
                break;  /* found it */
            j++;
        }
    } else {
        wl_list_for_each_reverse(c, &sel->link, link) {
            if (visibleon(c, selMon))
                break;  /* found it */
        }
    }
    /* If only one client is visible on selMon, then c == sel */
    focusClient(sel, c, true);
}

void focusOnHiddenStack(int i)
{
    struct client *c, *sel = selClient();
    if (!sel)
        return;
    if (i > 0) {
        int j = 1;
        wl_list_for_each(c, &sel->link, link) {
            if (hiddenon(c, selMon))
                break;  /* found it */
            j++;
        }
    } else {
        wl_list_for_each_reverse(c, &sel->link, link) {
            if (hiddenon(c, selMon))
                break;  /* found it */
        }
    }
    if (sel && c) {
        if (sel == c)
            return;
        // replace current client with a hidden one
        wl_list_remove(&c->link);
        wl_list_insert(&sel->link, &c->link);
        wl_list_remove(&sel->link);
        wl_list_insert(clients.prev, &sel->link);
    }
    /* If only one client is visible on selMon, then c == sel */
    focusClient(sel, c, true);
    arrange(selMon, false);
}
