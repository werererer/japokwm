#ifndef ROOT_H
#define ROOT_H

#include <wlr/types/wlr_box.h>
#include "monitor.h"

struct root {
    /* window area(area where windows can tile) */
    struct wlr_box w;
    float color[4];
};
extern struct root root;

/* set the are where windows can be placed in respect to layershell based 
 * programs which occupie space*/
void setRootArea(struct monitor *m);
#endif /* ROOT_H */
