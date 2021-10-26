#ifndef LIB_DIRECTION_H
#define LIB_DIRECTION_H
#include <lua.h>
#include <lauxlib.h>
#include <wlr/util/edges.h>

void lua_load_direction(lua_State *L);

// getter
int lib_direction_get_all(lua_State *L);
int lib_direction_get_bottom(lua_State *L);
int lib_direction_get_left(lua_State *L);
int lib_direction_get_none(lua_State *L);
int lib_direction_get_right(lua_State *L);
int lib_direction_get_top(lua_State *L);

#endif /* LIB_DIRECTION_H */
