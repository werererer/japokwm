#ifndef LIB_ACTIONS_H
#define LIB_ACTIONS_H
#include <lua.h>
#include <lauxlib.h>
#include <stdint.h>

#include "monitor.h"
#include "client.h"

int lib_arrange(lua_State *L);
int lib_focus_container(lua_State *L);
int lib_focus_on_hidden_stack(lua_State *L);
int lib_focus_on_stack(lua_State *L);
int lib_kill(lua_State *L);
int lib_load_next_layout_in_set(lua_State *L);
int lib_load_prev_layout_in_set(lua_State *L);
int lib_load_layout_in_set(lua_State *L);
int lib_load_layout(lua_State *L);
int lib_move_container_to_workspace(lua_State *L);
int lib_move_resize(lua_State *L);
int lib_move_to_scratchpad(lua_State *L);
int lib_quit(lua_State *L);
int lib_repush(lua_State *L);
int lib_resize_main(lua_State *L);
int lib_set_floating(lua_State *L);
int lib_show_scratchpad(lua_State *L);
int lib_exec(lua_State *L);
int lib_toggle_bars(lua_State *L);
int lib_toggle_floating(lua_State *L);
int lib_toggle_layout(lua_State *L);
int lib_toggle_view(lua_State *L);
int lib_toggle_workspace(lua_State *L);
int lib_view(lua_State *L);
int lib_tag_view(lua_State *L);
int lib_zoom(lua_State *L);
int lib_set_nmaster(lua_State *L);
int lib_increase_nmaster(lua_State *L);
int lib_decrease_nmaster(lua_State *L);
int lib_move_workspace_to(lua_State *L);
int lib_swap_workspace(lua_State *L);

#endif /* LIB_ACTIONS_H */
