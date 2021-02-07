#ifndef LOCAL_CONFIG_H
#define LOCAL_CONFIG_H

#include <lua.h>
#include <lauxlib.h>

int local_set_gaps(lua_State *L);
int local_set_tile_borderpx(lua_State *L);
int local_set_float_borderpx(lua_State *L);
int local_set_sloppy_focus(lua_State *L);
int local_set_focus_color(lua_State *L);
int local_set_border_color(lua_State *L);
int local_set_arrange_by_focus(lua_State *L);
int local_set_layout_constraints(lua_State *L);
int local_set_master_constraints(lua_State *L);
int local_set_update_function(lua_State *L);
int local_set_resize_direction(lua_State *L);

#endif /* LOCAL_CONFIG_H */
