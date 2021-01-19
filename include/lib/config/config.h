#ifndef CONFIG_H
#define CONFIG_H

#include "options.h"
#include <lauxlib.h>
#include <lua.h>

// TODO add other stuff
int set_gaps(lua_State *L);
int set_borderpx(lua_State *L);
int set_sloppy_focus(lua_State *L);

#endif /* CONFIG_H */
