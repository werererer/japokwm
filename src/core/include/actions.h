#ifndef ACTIONS_H
#define ACTIONS_H
#include <lua.h>
#include <lauxlib.h>
#include <stdint.h>

#include "monitor.h"
#include "client.h"

int spawn(lua_State *L);
int focusOnStack(lua_State *L);
int focusOnHiddenStack(lua_State *L);
int moveResize(lua_State *L);
int toggleFloating(lua_State *L);
int quit(lua_State *L);
int moveClient(lua_State *L);
int resizeClient(lua_State *L);
int updateLayout(lua_State *L);
int zoom(lua_State *L);
int readOverlay(lua_State *L);

void motionnotify(uint32_t time);

#endif /* ACTIONS_H */
