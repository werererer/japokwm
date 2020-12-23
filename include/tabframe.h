#ifndef TAB_FRAME_H
#define TAB_FRAME_H
#include "container.h"

struct tabframe {
    // the maximum amount of tabs (if -1 it means as many as possible)
    int tabcount;
    int selected_count;
    struct wlr_list bar_items;

    struct wlr_list containers;
    struct wlr_box geom;
};

struct tabframe *create_tabframe();
void destroy_tabframe(struct tabframe *tf);
void update_tabframe(struct tabframe *tf);
#endif /* TAB_FRAME_H */
