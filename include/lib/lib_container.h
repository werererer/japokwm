#ifndef LIB_CONTAINER_H
#define LIB_CONTAINER_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>

struct container;

void create_lua_container(lua_State *L, struct container *con);
void lua_load_container(lua_State *L);
struct container *check_container(lua_State *L, int argn);

// static methods
int lib_container_is_equal(lua_State *L);
int lib_container_focus(lua_State *L);

// methods
int lib_container_move_to_tag(lua_State *L);
int lib_container_set_ratio(lua_State *L);
int lib_container_get_tags(lua_State *L);
int lib_container_set_sticky_restricted(lua_State *L);
int lib_container_toggle_add_sticky(lua_State *L);
int lib_container_toggle_add_sticky_restricted(lua_State *L);
int lib_container_kill(lua_State *L);

// getter
int lib_container_get_alpha(lua_State *L);
int lib_container_get_app_id(lua_State *L);
int lib_container_get_current_property(lua_State *L);
int lib_container_get_focused(lua_State *L);
int lib_container_get_property(lua_State *L);
int lib_container_get_tags(lua_State *L);
int lib_container_get_tag(lua_State *L);

// setter
int lib_container_set_alpha(lua_State *L);
int lib_container_property_set_floating(lua_State *L);
int lib_container_get_tags(lua_State *L);
int lib_container_set_tags(lua_State *L);

#endif /* LIB_CONTAINER_H */
