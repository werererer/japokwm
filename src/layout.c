#include "layout.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lua.h>

#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "workspace.h"

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

struct resize_constraints lua_toresize_constrains(lua_State *L)
{
    struct resize_constraints resize_constrains;
    lua_getfield(L, -1, "min_width");
    resize_constrains.min_width = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "max_width");
    resize_constrains.max_width = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "min_height");
    resize_constrains.min_height = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "max_height");
    resize_constrains.max_height = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return resize_constrains;
}

bool is_same_layout(struct layout layout, struct layout layout2)
{
    const char *c = layout.symbol;
    const char *c2 = layout2.symbol;
    // same string means same layout
    return strcmp(c, c2) != 0;
}

struct layout copy_layout(struct layout *src_lt)
{
    if (!src_lt)
        return default_layout;
    struct layout dest_lt = default_layout;

    if (src_lt->lua_layout_copy_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_copy_data_ref);
        dest_lt.lua_layout_copy_data_ref = lua_copy_table(L);
    }

    if (src_lt->lua_layout_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_ref);
        dest_lt.lua_layout_ref = lua_copy_table(L);
    }

    if (src_lt->lua_layout_master_copy_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_master_copy_data_ref);
        dest_lt.lua_layout_master_copy_data_ref = lua_copy_table(L);
    }

    if (src_lt->lua_layout_original_copy_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_original_copy_data_ref);
        dest_lt.lua_layout_original_copy_data_ref = lua_copy_table(L);
    }

    copy_options(&dest_lt.options, &src_lt->options);

    return dest_lt;
}

void push_layout(struct layout lt_stack[static 2], struct layout lt)
{
    lt_stack[1] = copy_layout(&lt_stack[0]);
    lt_stack[0] = copy_layout(&lt);
}
