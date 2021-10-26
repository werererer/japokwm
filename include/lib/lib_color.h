#ifndef LIB_COLOR_H
#define LIB_COLOR_H

#include <lua.h>
#include <lauxlib.h>

// functions
void lua_load_color(lua_State *L);
int lib_color_new(lua_State *L);

struct color check_color(lua_State *L, int narg);

#endif /* LIB_COLOR_H */
