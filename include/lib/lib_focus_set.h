#ifndef LIB_FOCUS_SET_H
#define LIB_FOCUS_SET_H

#include <lua.h>
#include <lauxlib.h>

struct focus_set;

void lua_load_focus_set(lua_State *L);
void create_lua_focus_set(lua_State *L, struct focus_set *focus_set);

// getter
int lib_get_focus_stack_lists_with_layer_shell(lua_State *L);
int lib_get_focus_stack_visible_lists(lua_State *L);
int lib_get_focus_stack_lists(lua_State *L);
int lib_get_focus_stack_layer_background(lua_State *L);
int lib_get_focus_stack_layer_bottom(lua_State *L);
int lib_get_focus_stack_layer_top(lua_State *L);
int lib_get_focus_stack_layer_overlay(lua_State *L);
int lib_get_focus_stack_on_top(lua_State *L);
int lib_get_focus_stack_normal(lua_State *L);
int lib_get_focus_stack_not_focusable(lua_State *L);
#endif /* LIB_FOCUS_SET_H */
