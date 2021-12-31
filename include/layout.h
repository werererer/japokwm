#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdbool.h>
#include <lua.h>
#include <lauxlib.h>
#include "options.h"

struct layout {
    const char *name;

    // this is an option set by the user
    int current_max_area;
    /* number of all windows in layout even if they are invisible). Note that
     * floating windows don't belong to the layout and are thereby not counted */
    int n_all;
    // the amount of slave windows plus the master are (+1)
    int n_area;
    int n_area_max;
    // the amount of visible windows
    int n_visible;
    // number of hidden windows
    int n_hidden;
    // number of floating windows
    int n_floating;
    // number of tiled windows
    int n_tiled;
    int n_tiled_max;
    // the absolute amount of master windows
    int n_master_abs;
    // the amount master windows
    int n_master;
    int lua_resize_function_ref;
    int lua_layout_ref;
    int lua_layout_copy_data_ref;
    int lua_layout_original_copy_data_ref;
    GPtrArray *linked_layouts;
    GPtrArray *linked_loaded_layouts;

    int lua_master_layout_data_ref;
    int lua_resize_data_ref;

    int tag_id;

    struct options *options;
};

struct layout *create_layout(lua_State *L);
void destroy_layout(struct layout *lt);

bool is_same_layout(struct layout layout, struct layout layout2);
bool lua_is_layout_data(lua_State *L, const char *name);
void lua_copy_table(lua_State *L, int *ref);
// copy table and override old value
void lua_copy_table_safe(lua_State *L, int *ref);
struct resize_constraints lua_toresize_constrains(lua_State *L);
// copy layout and create new references
void copy_layout(struct layout *dest_lt, struct layout *src_lt);
// copy layout and override all references with the given ones
void copy_layout_safe(struct layout *dest_lt, struct layout *src_lt);

int deep_copy_table(lua_State *L);

int cmp_layout(const void *ptr1, const void *ptr2);
int cmp_layout_to_string(const void *ptr1, const void *symbol_ptr);
#endif /* LAYOUT_H */
