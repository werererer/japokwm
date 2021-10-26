#ifndef LIB_CONTAINER_PROPERTY_H
#define LIB_CONTAINER_PROPERTY_H

#include <lua.h>
#include <lauxlib.h>

struct container_property;

void lua_load_container_property(lua_State *L);
void create_lua_container_property(lua_State *L, struct container_property *container_property);

// functions
// methods
// getter
int lib_container_property_get_floating(lua_State *L);
int lib_container_property_get_geometry(lua_State *L);
// setter
int lib_container_property_set_floating(lua_State *L);

#endif /* LIB_CONTAINER_PROPERTY_H */
