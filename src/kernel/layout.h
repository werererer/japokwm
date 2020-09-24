#ifndef LAYOUT
#define LAYOUT

typedef struct {
    const char *symbol;
    void (*arrange)(Monitor *);
} Layout;

#endif
