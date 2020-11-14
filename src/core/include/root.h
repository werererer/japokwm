#ifndef ROOT_H
#define ROOT_H

#include <wlr/types/wlr_box.h>

struct root {
    /* window area(area where windows can tile) */
    struct wlr_box w;
    float color[4];
};
extern struct root root;

#endif /* ROOT_H */
