#ifndef ACTIONS_H
#define ACTIONS_H
#include <lua.h>
#include <lauxlib.h>
#include <stdint.h>

#include "monitor.h"
#include "client.h"

int arrange_this(lua_State *L);
int focus_on_hidden_stack(lua_State *L);
int focus_on_stack(lua_State *L);
int get_nmaster(lua_State *L);
int get_tabcount(lua_State *L);
int kill_client(lua_State *L);
int load_layout_lib(lua_State *L);
int move_client(lua_State *L);
int move_container_to_workspace(lua_State *L);
int move_resize(lua_State *L);
int quit(lua_State *L);
int read_boxes(lua_State *L);
int read_master_layout(lua_State *L);
int resize_client(lua_State *L);
int set_floating(lua_State *L);
int set_nmaster(lua_State *L);
int set_tabcount(lua_State *L);
int spawn(lua_State *L);
int tag(lua_State *L);
int toggle_consider_layer_shell(lua_State *L);
int toggle_floating(lua_State *L);
int toggle_tag(lua_State *L);
int toggle_view(lua_State *L);
int unload_layout(lua_State *L);
int update_layout(lua_State *L);
int view(lua_State *L);
int zoom(lua_State *L);

void motionnotify(uint32_t time);

#endif /* ACTIONS_H */
