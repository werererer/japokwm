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
    copy_layout(&lt_stack[1], &lt_stack[0]);
    copy_layout(&lt_stack[0], &lt);
}

void set_layout(lua_State *L, struct layout *lt)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->options.layouts_ref);

    if (lua_gettop(L) <= 1) {
        lt->lua_layout_index++;
        if (lt->lua_layout_index > luaL_len(L, -1)) {
            lt->lua_layout_index = 1;
        }
    }

    lua_rawgeti(L, -1, lt->lua_layout_index);
    lua_rawgeti(L, -1, 2);
    const char *layout_name = luaL_checkstring(L, -1);
    lua_pop(L, 3);
    load_layout(L, lt, layout_name);
}

void load_layout(lua_State *L, struct layout *lt, const char *layout_name)
{
    lt->name = layout_name;

    char *config_path = get_config_file("layouts");
    char file[NUM_CHARS] = "";
    strcpy(file, "");
    join_path(file, config_path);
    join_path(file, layout_name);
    join_path(file, "init.lua");
    if (config_path)
        free(config_path);

    if (!file_exists(file))
        return;

    if (luaL_loadfile(L, file)) {
        lua_pop(L, 1);
        return;
    }
    lua_call_safe(L, 0, 0, 0);
}

void load_default_layout(lua_State *L, struct layout *lt)
{
    load_layout(L, lt, default_layout.name);
}
