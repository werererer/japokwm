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

#endif /* LIB_SERVER_H */
