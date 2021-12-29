#ifndef LIB_GEOM_H
#define LIB_GEOM_H

#include <lua.h>
#include <lauxlib.h>
#include <wlr/types/wlr_box.h>

void create_lua_geometry(lua_State *L, struct wlr_box *geom);
void lua_load_geom(lua_State *L);
struct wlr_box *check_geometry(lua_State *L, int argn);

// static methods
// methods
// getter
int lib_geometry_get_x(lua_State *L);
int lib_geometry_get_y(lua_State *L);
int lib_geometry_get_width(lua_State *L);
int lib_geometry_get_height(lua_State *L);
// setter
int lib_geometry_set_x(lua_State *L);
int lib_geometry_set_y(lua_State *L);
int lib_geometry_set_width(lua_State *L);
int lib_geometry_set_height(lua_State *L);

#endif /* LIB_GEOM_H */
