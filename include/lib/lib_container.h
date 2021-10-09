#ifndef LIB_CONTAINER_H
#define LIB_CONTAINER_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>

struct container;

void create_lua_container(struct container *con);
void lua_load_container();

// methods
int lib_container_get_focused(lua_State *L);
int lib_container_get_workspace(lua_State *L);
int lib_container_set_ratio(lua_State *L);
int lib_container_set_sticky(lua_State *L);
int lib_container_set_sticky_restricted(lua_State *L);
int lib_container_toggle_add_sticky(lua_State *L);
int lib_container_toggle_add_sticky_restricted(lua_State *L);

// getter
int lib_container_get_alpha(lua_State *L);
int lib_container_get_sticky(lua_State *L);

// setter
int lib_container_set_alpha(lua_State *L);
int lib_container_set_sticky(lua_State *L);

#endif /* LIB_CONTAINER_H */
