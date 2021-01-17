#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdbool.h>
#include <lua.h>
#include <lauxlib.h>

struct layout {
    const char *name;
    const char *symbol;
    // the amount of slave windows plus the master are (+1)
    int n;
    // the amount master windows
    int nmaster;
    int resize_dir;
    int lua_layout_index;
    int lua_layout_copy_data_index;
    int test;
};

bool is_same_layout(struct layout layout, struct layout layout2);
int lua_copy_table(lua_State *L);

extern struct layout default_layout;
extern struct layout prev_layout;
#endif /* LAYOUT_H */
