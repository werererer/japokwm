#ifndef LAYOUT_H
#define LAYOUT_H

struct layout {
    char *symbol;
    /* this function gets executed when the arrange function is called
     * usually this function will read the data about the layout from a
     * file.*/
    int funcId;
};

extern struct layout defaultLayout;
extern struct layout prevLayout;
#endif /* LAYOUT_H */
