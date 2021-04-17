#ifndef LIB_CONTAINER_H
#define LIB_CONTAINER_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>

// TODO fix later
int container_set_sticky(lua_State *L);
int container_set_ratio(lua_State *L);
int container_set_alpha(lua_State *L);

#endif /* LIB_CONTAINER_H */
