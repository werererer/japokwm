#ifndef CONFIG_H
#define CONFIG_H

#include "options.h"
#include <lauxlib.h>
#include <lua.h>

// TODO add other stuff
int lib_set_borderpx(lua_State *L);
int lib_set_gaps(lua_State *L);
int lib_set_root_color(lua_State *L);
int lib_set_sloppy_focus(lua_State *L);
int lib_set_focus_color(lua_State *L);
int lib_set_border_color(lua_State *L);

#endif /* CONFIG_H */
