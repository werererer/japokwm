#ifndef OVERLAY_H
#define OVERLAY_H
#include <lua.h>
#include <lauxlib.h>
#include <stdbool.h>

int write_this_overlay(lua_State *L);
int set_overlay(lua_State *L);
int get_overlay(lua_State *L);
#endif /* OVERLAY_H */
