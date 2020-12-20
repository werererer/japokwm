#ifndef ROOT_H
#define ROOT_H

#include <wlr/types/wlr_box.h>

struct root {
    /* window area(area where windows can tile) */
    struct wlr_box w;
    float color[4];
    /* should anchored layershell programs be taken into consideration */
    bool consider_layer_shell;
};

struct root *create_root();
void destroy_root(struct root *root);

#endif /* ROOT_H */
