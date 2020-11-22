#ifndef LAYOUT_H
#define LAYOUT_H

struct layout {
    char *symbol;
    /* this function gets executed when the arrange function is called
     * usually this function will read the data about the layout from a
     * file.*/
    int funcId;
};

void layoutCreate(struct layout *lt, const char *symbol, int funcId);
void layoutDestroy(struct layout *lt);

extern struct layout defaultLayout;
extern struct layout prevLayout;
#endif /* LAYOUT_H */
