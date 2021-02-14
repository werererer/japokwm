#ifndef LAYOUT_SET_H
#define LAYOUT_SET_H

#include <lua.h>
#include <lauxlib.h>
#include <stdbool.h>

struct layout_set {
    const char *key;
    int layout_sets_ref;
    int lua_layout_index;
};

struct layout_set get_default_layout_set();

void lua_set_layout_set_element(lua_State *L, const char *key, int layout_set_ref);
void lua_get_layout_set_element(lua_State *L, const char *key);
bool lua_is_index_defined(lua_State *L, const char *key);

#endif /* LAYOUT_SET_H */
