#ifndef LIB_WORKSPACE
#define LIB_WORKSPACE

#include <lua.h>
#include <lauxlib.h>

struct workspace;

void create_lua_workspace(struct workspace *ws);
void lua_load_workspace();

struct workspace *check_workspace(lua_State *L, int narg);

// functions
int lib_workspace_get_focused(lua_State *L);

// methods
int lib_workspace_swap(lua_State *L);
int lib_workspace_get_id(lua_State *L);

// setter

// getter
#endif /* LIB_WORKSPACE */
