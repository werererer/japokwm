#ifndef ACTIONS_H
#define ACTIONS_H
#include <lua.h>
#include <lauxlib.h>
#include <stdint.h>

#include "monitor.h"
#include "client.h"

int lib_arrange(lua_State *L);
int focus_on_hidden_stack(lua_State *L);
int focus_on_stack(lua_State *L);
int get_nmaster(lua_State *L);
int lib_kill_client(lua_State *L);
int lib_load_layout(lua_State *L);
int move_client(lua_State *L);
int lib_move_client_to_workspace(lua_State *L);
int move_resize(lua_State *L);
int lib_quit(lua_State *L);
int read_boxes(lua_State *L);
int read_master_layout(lua_State *L);
int lib_resize_client(lua_State *L);
int resize_main(lua_State *L);
int set_floating(lua_State *L);
int set_nmaster(lua_State *L);
int set_arrange_by_focus(lua_State *L);
int set_resize_direction(lua_State *L);
int spawn(lua_State *L);
int toggle_consider_layer_shell(lua_State *L);
int toggle_floating(lua_State *L);
int toggle_view(lua_State *L);
int view(lua_State *L);
int lib_zoom(lua_State *L);

void motionnotify(uint32_t time);

#endif /* ACTIONS_H */
