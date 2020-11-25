#ifndef ROOT_H
#define ROOT_H

#include <wlr/types/wlr_box.h>
#include "monitor.h"

struct root {
    /* window area(area where windows can tile) */
    struct wlr_box *w;
    float color[4];
    /* should anchored layershell programs be taken into consideration */
    bool consider_layer_shell;
};
extern struct root root;

/* set the are where windows can be placed in respect to layershell based 
 * programs which occupie space*/
void set_root_area(struct monitor *m);
#endif /* ROOT_H */
