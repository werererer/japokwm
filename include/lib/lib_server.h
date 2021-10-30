#ifndef LIB_SERVER_H
#define LIB_SERVER_H

#include <lua.h>
struct server;

void create_lua_server(struct server *server);
void lua_load_server(lua_State *L);

// functions
int lib_server_get_focused_workspace(lua_State *L);
int lib_server_get_workspaces(lua_State *L);
int lib_server_quit(lua_State *L);

// getter
int lib_server_get_default_layout_ring(lua_State *L);
// setter
int lib_server_set_default_layout_ring(lua_State *L);

#endif /* LIB_SERVER_H */
