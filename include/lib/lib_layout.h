#ifndef LIB_LAYOUT_H
#define LIB_LAYOUT_H

#include <lua.h>
#include <lauxlib.h>

struct layout;

void create_lua_layout(struct layout *layout);
void lua_init_layout(struct layout *layout);
void lua_load_layout();

int lib_set_layout(lua_State *L);

#endif /* LIB_LAYOUT_H */
