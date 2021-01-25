#ifndef INFO_H
#define INFO_H

#include <lua.h>

int lib_get_this_container_count(lua_State *L);
int lib_this_container_position(lua_State *L);
int lib_get_next_empty_workspace(lua_State *L);
int lib_get_workspace(lua_State *L);

#endif /* INFO_H */
