#ifndef INFO_H
#define INFO_H

#include <lua.h>
#include <lauxlib.h>

void lua_load_info(lua_State *L);

int lib_get_container_under_cursor(lua_State *L);
// decided
int lib_get_root_area(lua_State *L);
int lib_get_this_container_count(lua_State *L);
int lib_get_workspace_count(lua_State *L);
int lib_is_container_not_in_limit(lua_State *L);
int lib_is_container_not_in_master_limit(lua_State *L);
int lib_is_keycombo(lua_State *L);
int lib_stack_position_to_position(lua_State *L);
int lib_this_container_position(lua_State *L);

#endif /* INFO_H */
