#include "layout.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lua.h>

#include "server.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "workspace.h"

struct layout get_default_layout()
{
    struct layout lt = (struct layout) {
        .symbol = "master",
        .name = "",
        .n = 1,
        .nmaster = 1,
        .options = get_default_options(),
    };

    lua_get_default_layout_data();
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt.lua_layout_copy_data_ref);
    lua_createtable(L, 0, 0);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt.lua_layout_ref);
    return lt;
}

void lua_copy_table(lua_State *L, int *ref)
{
    if (*ref > 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, *ref);
    }

    lua_getglobal_safe(L, "Deep_copy");
    lua_insert(L, -2);
    lua_call_safe(L, 1, 1, 0);

    *ref = luaL_ref(L, LUA_REGISTRYINDEX);
    printf("reference: %i\n", *ref);
    return;
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
    struct layout dest_lt = server.default_layout;

    if (!src_lt)
        return dest_lt;

    if (src_lt->lua_layout_copy_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_copy_data_ref);
        lua_copy_table(L, &dest_lt.lua_layout_copy_data_ref);
    }

    if (src_lt->lua_layout_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_ref);
        lua_copy_table(L, &dest_lt.lua_layout_ref);
    }

    if (src_lt->options.master_layout_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->options.master_layout_data_ref);
        lua_copy_table(L, &dest_lt.options.master_layout_data_ref);
    }

    if (src_lt->lua_layout_original_copy_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_original_copy_data_ref);
        lua_copy_table(L, &dest_lt.lua_layout_original_copy_data_ref);
    }

    copy_options(&dest_lt.options, &src_lt->options);

    return dest_lt;
}

void push_layout(struct layout lt_stack[static 2], struct layout lt)
{
    lt_stack[1] = lt_stack[0];
    lt_stack[0] = lt;
}

bool lua_islayout_data(lua_State *L, const char *name)
{
    if (!lua_istable(L, -1))
        return false;

    int len1 = luaL_len(L, -1);
    if (len1 <= 0)
        return false;

    for (int i = 1; i <= len1; i++) {
        lua_rawgeti(L, -1, i);
        int len2 = luaL_len(L, -1);
        if (len2 <= 0)
        {
            char c[NUM_CHARS] = "";
            snprintf(c, NUM_CHARS, "%s[%i] == nil", name, i);
            handle_error(c);
            return false;
        }
        for (int j = 1; j <= len2; j++) {
            lua_rawgeti(L, -1, j);
            int len3 = luaL_len(L, -1);
            if (len3 != 4) {
                char c[NUM_CHARS] = "";
                snprintf(c, NUM_CHARS, "%s[%i][%i].size != 4", name, i, j);
                handle_error(c);
                return false;
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    return true;
}
