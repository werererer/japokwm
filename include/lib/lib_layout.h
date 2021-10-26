#ifndef LIB_LAYOUT_H
#define LIB_LAYOUT_H

#include <lua.h>
#include <lauxlib.h>

struct layout;

void create_lua_layout(lua_State *L, struct layout *layout);
void lua_init_layout(struct layout *layout);
void lua_load_layout(lua_State *L);

// functions
int lib_layout_get_focused(lua_State *L);
int lib_layout_get_active_layout(lua_State *L);
int lib_layout_load(lua_State *L);
int lib_layout_load_in_set(lua_State *L);
int lib_layout_load_next_in_set(lua_State *L);
int lib_layout_load_prev_in_set(lua_State *L);
// methods
int lib_layout_set_layout(lua_State *L);
int lib_layout_set_master_layout_data(lua_State *L);
int lib_layout_set_linked_layouts_ref(lua_State *L);
int lib_layout_set_resize_data(lua_State *L);
int lib_layout_set_resize_function(lua_State *L);
// setter
int lib_layout_set_n_area(lua_State *L);
int lib_layout_set_nmaster(lua_State *L);
int lib_layout_set_default_layout(lua_State *L);
// getter
int lib_layout_get_direction(lua_State *L);
int lib_layout_get_layout_data(lua_State *L);
int lib_layout_get_n(lua_State *L);
int lib_layout_get_n_area(lua_State *L);
int lib_layout_get_n_master(lua_State *L);
int lib_layout_get_n_tiled(lua_State *L);
int lib_layout_get_nmaster(lua_State *L);
int lib_layout_get_o_layout_data(lua_State *L);
int lib_layout_get_resize_data(lua_State *L);

#endif /* LIB_LAYOUT_H */
