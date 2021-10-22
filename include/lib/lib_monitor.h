#ifndef LIB_MONITOR_H
#define LIB_MONITOR_H

#include <lua.h>

void lua_load_monitor();

int lib_set_scale(lua_State *L);
int lib_set_transform(lua_State *L);

#endif /* LIB_MONITOR_H */
