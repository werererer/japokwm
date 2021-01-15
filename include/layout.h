#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdbool.h>

struct layout {
    const char *name;
    const char *symbol;
    // the amount of slave windows plus the master are (+1)
    int n;
    // the amount master windows
    int nmaster;
    int lua_layout_data_index;
};

void create_layout(struct layout *lt, const char *name, const char *symbol);
void destroy_layout(struct layout *lt);

bool is_same_layout(struct layout layout, struct layout layout2);

extern struct layout default_layout;
extern struct layout prev_layout;
#endif /* LAYOUT_H */
