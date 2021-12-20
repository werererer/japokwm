#ifndef LIB_TAG_H
#define LIB_TAG_H

#include <lua.h>
#include <lauxlib.h>

#include "bitset/bitset.h"

struct tag;

void create_lua_tag(lua_State *L, struct tag *tag);
void lua_load_tag(lua_State *L);

struct tag *check_tag(lua_State *L, int narg);

// functions
int lib_tag_get_next_empty(lua_State *L);
int lib_tag_get(lua_State *L);

// methods
int lib_tag_get_id(lua_State *L);
int lib_tag_swap(lua_State *L);
int lib_tag_swap_smart(lua_State *L);
int lib_tag_toggle_bars(lua_State *L);

// setter
int lib_set_tags(lua_State *L);
int lib_tag_set_bars_visibility(lua_State *L);

// getter
int lib_tag_get_bars_visibility(lua_State *L);
int lib_tag_get_focus_set(lua_State *L);
int lib_tag_get_focus_stack(lua_State *L);
int lib_tag_get_focused(lua_State *L);
int lib_tag_get_layout(lua_State *L);
int lib_tag_get_previous_layout(lua_State *L);
int lib_tag_get_stack(lua_State *L);
int lib_tag_get_tags(lua_State *L);
int lib_tag_get_visible_focus_set(lua_State *L);
int lib_tag_get_visible_focus_stack(lua_State *L);
int lib_tag_get_visible_stack(lua_State *L);
#endif /* LIB_TAG_H */
