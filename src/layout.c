#include "layout.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lua.h>

#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"

struct layout default_layout;

struct layout get_default_layout()
{
    struct layout lt = (struct layout) {
        .symbol = "s",
        .name = "master",
        .nmaster = 1,
        .options = get_default_options(),
    };
    lua_get_basic_layout();
    lt.lua_layout_copy_data_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_createtable(L, 0, 0);
    lt.lua_layout_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_get_basic_layout();
    lt.lua_layout_master_copy_data_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return lt;
}

int lua_copy_table(lua_State *L)
{
    lua_getglobal_safe(L, "Deep_copy");
    lua_insert(L, -2);
    lua_call_safe(L, 1, 1, 0);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return ref;
}

bool is_same_layout(struct layout layout, struct layout layout2)
{
    const char *c = layout.symbol;
    const char *c2 = layout2.symbol;
    // same string means same layout
    return strcmp(c, c2) != 0;
}

void copy_layout(struct layout *dest_lt, struct layout *src_lt)
{
    if (!dest_lt)
        return;
    if (!src_lt)
        return;

    if (src_lt->lua_layout_copy_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_copy_data_ref);
        dest_lt->lua_layout_copy_data_ref = lua_copy_table(L);
    }

    if (src_lt->lua_layout_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_ref);
        dest_lt->lua_layout_ref = lua_copy_table(L);
    }

    if (src_lt->lua_layout_master_copy_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_master_copy_data_ref);
        dest_lt->lua_layout_master_copy_data_ref = lua_copy_table(L);
    }

    if (src_lt->lua_layout_original_copy_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_original_copy_data_ref);
        dest_lt->lua_layout_original_copy_data_ref = lua_copy_table(L);
    }

    dest_lt->resize_dir = src_lt->resize_dir;

    copy_options(&dest_lt->options, &src_lt->options);
}

void push_layout(struct layout lt_stack[static 2], struct layout lt)
{
    lt_stack[1] = lt_stack[0];
    lt_stack[0] = lt;
}
