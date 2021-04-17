#ifndef LIB_EVENT_HANDLER_H
#define LIB_EVENT_HANDLER_H

#include <lua.h>
#include <lauxlib.h>

int lib_set_create_container_function(lua_State *L);
int lib_set_on_focus_function(lua_State *L);
int lib_set_on_start_function(lua_State *L);
int lib_set_update_function(lua_State *L);

#endif /* LIB_EVENT_HANDLER_H */
