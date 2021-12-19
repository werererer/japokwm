#ifndef LIB_WORKSPACE
#define LIB_WORKSPACE

#include <lua.h>
#include <lauxlib.h>

#include "bitset/bitset.h"

struct workspace;

void create_lua_workspace(lua_State *L, struct workspace *ws);
void lua_load_workspace(lua_State *L);

struct workspace *check_workspace(lua_State *L, int narg);

// functions
int lib_workspace_get_next_empty(lua_State *L);
int lib_workspace_get(lua_State *L);

// methods
int lib_workspace_get_id(lua_State *L);
int lib_workspace_swap(lua_State *L);
int lib_workspace_swap_smart(lua_State *L);
int lib_workspace_toggle_bars(lua_State *L);

// setter
int lib_set_tags(lua_State *L);
int lib_workspace_set_bars_visibility(lua_State *L);

// getter
int lib_workspace_get_bars_visibility(lua_State *L);
int lib_workspace_get_focus_set(lua_State *L);
int lib_workspace_get_focus_stack(lua_State *L);
int lib_workspace_get_focused(lua_State *L);
int lib_workspace_get_layout(lua_State *L);
int lib_workspace_get_previous_layout(lua_State *L);
int lib_workspace_get_stack(lua_State *L);
int lib_workspace_get_tags(lua_State *L);
int lib_workspace_get_visible_focus_set(lua_State *L);
int lib_workspace_get_visible_focus_stack(lua_State *L);
int lib_workspace_get_visible_stack(lua_State *L);
#endif /* LIB_WORKSPACE */
