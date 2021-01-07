#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdbool.h>

struct layout {
    const char *symbol;
    const char *name;
    /* this function gets executed when the arrange function is called
     * usually this function will read the data about the layout from a
     * file.*/
    int funcId;
    // the amount of slave windows plus the master are (+1)
    int n;
    // the amount master windows
    int nmaster;
    int id;
};

void create_layout(struct layout *lt, const char *symbol, int funcId);
void destroy_layout(struct layout *lt);

bool is_same_layout(struct layout layout, struct layout layout2);

extern struct layout default_layout;
extern struct layout prev_layout;
#endif /* LAYOUT_H */
