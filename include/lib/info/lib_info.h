#ifndef INFO_H
#define INFO_H

#include <lua.h>

int lib_get_active_layout(lua_State *L);
int lib_get_container_under_cursor(lua_State *L);
int lib_get_next_empty_workspace(lua_State *L);
int lib_get_nmaster(lua_State *L);
int lib_get_root_area(lua_State *L);
int lib_get_this_container_count(lua_State *L);
int lib_get_workspace(lua_State *L);
int lib_is_container_not_in_limit(lua_State *L);
int lib_is_container_not_in_master_limit(lua_State *L);
int lib_this_container_position(lua_State *L);

#endif /* INFO_H */
