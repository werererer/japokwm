#ifndef LIB_COLOR_H
#define LIB_COLOR_H

#include <lua.h>
#include <lauxlib.h>

// functions
void lua_load_color(lua_State *L);
int lib_color_new(lua_State *L);

struct color check_color(lua_State *L, int narg);

// static setters
// static getters
int lib_color_get_black(lua_State *L);
int lib_color_get_white(lua_State *L);
int lib_color_get_red(lua_State *L);
int lib_color_get_green(lua_State *L);
int lib_color_get_blue(lua_State *L);

#endif /* LIB_COLOR_H */
