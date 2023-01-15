#ifndef LIB_ROOT_H
#define LIB_ROOT_H

#include <lua.h>
#include <lauxlib.h>

struct root;

void create_lua_root(lua_State *L, struct root *root);
void lua_load_root(lua_State *L);

// static methods
// methods
// getters
int lib_root_get_geom(lua_State *L);
// setters

#endif /* LIB_ROOT_H */
