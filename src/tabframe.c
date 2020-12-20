#include "tabframe.h"
#include "tile/tileTexture.h"
#include <stdlib.h>

struct tabframe *create_tabframe()
{
    struct tabframe *tf = calloc(1, sizeof(struct tabframe));
    wlr_list_init(&tf->containers);
    wlr_list_init(&tf->bar_items);
    return tf;
}

void destroy_tabframe(struct tabframe *tf)
{
    wlr_list_finish(&tf->bar_items);
    wlr_list_finish(&tf->containers);
    free(tf);
}

void update_tabframe(struct tabframe *tf)
{
    float color[4] = {1.0, 0.0, 0, 1.0};
    float black[4] = {0.0, 0.0, 0, 1.0};
    float t[4] = {1.0, 0.6, 0, 1.0};
    wlr_list_clear(&tf->bar_items);
    for (int i = 0; i < tf->tabcount; i++) {
        if (i == tf->selected_count) {
            struct wlr_box head = tf->geom;
            head.height = 20;
            head.width /= tf->tabcount;
            head.x += i*head.width;
            struct pos_texture *pTexture =
                create_textbox(head, (float *)t, black, "test");
            wlr_list_insert(&tf->bar_items, i, pTexture);
        } else {
            struct wlr_box head = tf->geom;
            head.height = 20;
            head.width /= tf->tabcount;
            head.x += i*head.width;
            struct pos_texture *pTexture =
                create_textbox(head, (float *)color, black, "test");
            wlr_list_insert(&tf->bar_items, i, pTexture);
        }
    }
}
