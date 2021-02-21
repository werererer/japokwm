#ifndef LOCAL_EVENT_HANDLER_H
#define LOCAL_EVENT_HANDLER_H

#include <lua.h>
#include <lauxlib.h>

int local_set_update_function(lua_State *L);
int local_set_create_container_function(lua_State *L);

#endif /* LOCAL_EVENT_HANDLER_H */
