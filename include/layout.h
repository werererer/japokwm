#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdbool.h>
#include <lua.h>
#include <lauxlib.h>
#include "options.h"

struct resize_constraints {
    float min_width;
    float max_width;
    float min_height;
    float max_height;
};

struct layout {
    const char *name;
    const char *symbol;
    // the amount of slave windows plus the master are (+1)
    int n;
    // the amount master windows
    int nmaster;
    int resize_dir;
    int lua_layout_ref;
    int lua_layout_index;
    int lua_layout_copy_data_ref;
    int lua_layout_original_copy_data_ref;
    int lua_layout_master_copy_data_ref;
    int lua_resize_data_ref;

    struct resize_constraints layout_constraints;
    struct resize_constraints master_constraints;

    struct options options;
};

bool is_same_layout(struct layout layout, struct layout layout2);
int lua_copy_table(lua_State *L);
struct resize_constraints lua_toresize_constrains(lua_State *L);
void push_layout(struct layout lt_stack[static 2], struct layout lt);
struct layout copy_layout(struct layout *src_lt);
struct layout get_default_layout();

extern struct layout default_layout;
#endif /* LAYOUT_H */
