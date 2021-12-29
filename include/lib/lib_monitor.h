#ifndef LIB_MONITOR_H
#define LIB_MONITOR_H

#include <lua.h>

void lua_load_monitor(lua_State *L);

// static methods
int lib_monitor_get_focused(lua_State *L);
// methods

// getter
//
int lib_monitor_get_previous_layout(lua_State *L);
int lib_monitor_get_root(lua_State *L);
int lib_monitor_get_tag(lua_State *L);
// setter
int lib_monitor_set_scale(lua_State *L);
int lib_monitor_set_transform(lua_State *L);

#endif /* LIB_MONITOR_H */
