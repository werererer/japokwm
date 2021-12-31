#ifndef LIB_LIST2D_h
#define LIB_LIST2D_h

#include <lua.h>
#include <lauxlib.h>
#include <glib.h>
#include "utils/coreUtils.h"

void create_lua_list2D(lua_State *L, GPtrArray2D *arr);
void lua_load_list2D(lua_State *L);

GPtrArray *check_list2D(lua_State *L, int argn);

// static methods
int lib_list2D_to_array(lua_State *L);

// methods
int lib_list2D_find(lua_State *L);
int lib_list2D_get(lua_State *L);
int lib_list2D_repush(lua_State *L);
int lib_list2D_swap(lua_State *L);
// getter
int lib_list2D_length(lua_State *L);
// setter
#endif /* LIB_LIST2D_h */
