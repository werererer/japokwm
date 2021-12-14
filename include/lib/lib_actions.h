#ifndef LIB_ACTIONS_H
#define LIB_ACTIONS_H
#include <lua.h>
#include <lauxlib.h>
#include <stdint.h>

#include "monitor.h"
#include "client.h"

void lua_load_action(lua_State *L);

int lib_arrange(lua_State *L);
int lib_async_exec(lua_State *L);
int lib_create_output(lua_State *L);
int lib_exec(lua_State *L);
// what is that?
int lib_move_resize(lua_State *L);
int lib_focus_on_hidden_stack(lua_State *L);
int lib_focus_on_stack(lua_State *L);
int lib_move_to_scratchpad(lua_State *L);
int lib_deep_copy(lua_State *L);
int lib_resize_main(lua_State *L);
int lib_show_scratchpad(lua_State *L);
int lib_start_keycombo(lua_State *L);
int lib_swap_on_hidden_stack(lua_State *L);
int lib_toggle_all_bars(lua_State *L);
int lib_toggle_tags(lua_State *L);
int lib_toggle_view(lua_State *L);
int lib_toggle_workspace(lua_State *L);
int lib_view(lua_State *L);
int lib_view_or_tag(lua_State *L);
int lib_zoom(lua_State *L);

#endif /* LIB_ACTIONS_H */
