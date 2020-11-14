#ifndef ACTIONS_H
#define ACTIONS_H
#include <lua.h>
#include <lauxlib.h>
#include <stdint.h>

#include "monitor.h"
#include "client.h"

int arrangeThis(lua_State *L);
int focusOnHiddenStack(lua_State *L);
int focusOnStack(lua_State *L);
int moveClient(lua_State *L);
int moveResize(lua_State *L);
int quit(lua_State *L);
int readOverlay(lua_State *L);
int resizeClient(lua_State *L);
int spawn(lua_State *L);
int tag(lua_State *L);
int toggleAddView(lua_State *L);
int toggleFloating(lua_State *L);
int toggleView(lua_State *L);
int toggletag(lua_State *L);
int updateLayout(lua_State *L);
int view(lua_State *L);
int zoom(lua_State *L);
// kill
int killClient(lua_State *L);

void motionnotify(uint32_t time);

#endif /* ACTIONS_H */
