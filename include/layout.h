#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdbool.h>

struct layout {
    char *symbol;
    /* this function gets executed when the arrange function is called
     * usually this function will read the data about the layout from a
     * file.*/
    int funcId;
    int n;
    int nmaster;
    int id;
};

void create_layout(struct layout *lt, const char *symbol, int funcId);
void destroy_layout(struct layout *lt);

bool is_same_layout(struct layout layout, struct layout layout2);

extern struct layout defaultLayout;
extern struct layout prev_layout;
#endif /* LAYOUT_H */
