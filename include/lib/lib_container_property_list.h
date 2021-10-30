#ifndef LIB_CONTAINER_PROPERTY_LIST_H
#define LIB_CONTAINER_PROPERTY_LIST_H

#include <lua.h>
#include <lauxlib.h>
#include <glib.h>

void create_lua_container_property_list(lua_State *L, GPtrArray *array);

#endif /* LIB_CONTAINER_PROPERTY_LIST_H */
