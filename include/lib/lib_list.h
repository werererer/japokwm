#ifndef LIB_LIST_H
#define LIB_LIST_H

#include <lua.h>
#include <lauxlib.h>
#include <glib.h>

void create_lua_list(lua_State *L, GPtrArray *arr);
void lua_load_list(lua_State *L);

// functions
int lib_list_to_array(lua_State *L);
// methods
int lib_list_find(lua_State *L);
int lib_list_get(lua_State *L);
int lib_list_repush(lua_State *L);
int lib_list_swap(lua_State *L);
// getter
int lib_list_length(lua_State *L);
// setter

#endif /* LIB_LIST_H */
