#ifndef LIB_ROOT_H
#define LIB_ROOT_H

#include <lua.h>
#include <lauxlib.h>

struct root;

void create_lua_root(lua_State *L, struct root *root);
void lua_load_root(lua_State *L);

// functions
// methods
int lib_root_get_area(lua_State *L);
// getters
// setters

#endif /* LIB_ROOT_H */
