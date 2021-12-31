#ifndef LIB_CURSOR_MODE_H
#define LIB_CURSOR_MODE_H

#include "lua.h"
#include "lauxlib.h"

void lua_load_cursor_mode(lua_State *L);

// getter
int lib_cursor_mode_get_cursor_normal(lua_State *L);
int lib_cursor_mode_get_cursor_move(lua_State *L);
int lib_cursor_mode_get_cursor_resize(lua_State *L);

#endif /* LIB_CURSOR_MODE_H */
