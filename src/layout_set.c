#include "layout_set.h"
#include "utils/coreUtils.h"

struct layout_set get_default_layout_set()
{
    struct layout_set layout_set = {
        .key = "",
        .lua_layout_index = 1,
    };

    lua_createtable(L, 0, 0);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &layout_set.layout_sets_ref);

    return layout_set;
}

void lua_set_layout_set_element(lua_State *L, const char *key, int layout_set_ref)
{
    lua_pushstring(L, key);
    lua_rawgeti(L, LUA_REGISTRYINDEX, layout_set_ref);
    lua_settable(L, -3);
}

void lua_get_layout_set_element(lua_State *L, const char *key)
{
    lua_pushstring(L, key);
    lua_rawget(L, -2);
}

bool lua_is_index_defined(lua_State *L, const char *key)
{
    lua_get_layout_set_element(L, key);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    lua_pop(L, 1);
    return true;
}
