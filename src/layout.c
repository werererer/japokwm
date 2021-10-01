#include "layout.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lua.h>
#include <assert.h>

#include "server.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "workspace.h"

struct layout *create_layout(lua_State *L)
{
    struct layout *lt = calloc(1, sizeof(*lt));
    lt->nmaster = 1;
    *lt = (struct layout) {
        .symbol = "",
        .n_area = 1,
        .nmaster = 1,
    };

    lt->options = create_options();

    lua_get_default_master_layout_data(L);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_master_layout_data_ref);

    lua_get_default_resize_data(L);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_resize_data_ref);

    lua_get_default_layout_data(L);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_layout_copy_data_ref);

    lua_createtable(L, 0, 0);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_layout_ref);

    lua_get_default_resize_function(L);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_resize_function_ref);

    return lt;
}

void destroy_layout(struct layout *lt)
{
    debug_print("destroy layout: %p\n", lt);
    destroy_options(lt->options);

    free(lt);
}

void lua_copy_table(lua_State *L, int *ref)
{
    // lua copy table safe will execute lua_ref_safe. This will override the
    // old table that is sitting in the position of *ref to prevent this we
    // simply set *ref equals 0
    *ref = 0;
    lua_copy_table_safe(L, ref);
    return;
}

void lua_copy_table_safe(lua_State *L, int *ref)
{
    assert(lua_istable(L, -1));
    lua_getglobal_safe(L, "Deep_copy");
    lua_insert(L, -2);
    lua_call_safe(L, 1, 1, 0);

    lua_ref_safe(L, LUA_REGISTRYINDEX, ref);
    return;
}

struct resize_constraints lua_toresize_constrains(lua_State *L)
{
    struct resize_constraints resize_constraints;
    lua_getfield(L, -1, "min_width");
    resize_constraints.min_width = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "max_width");
    resize_constraints.max_width = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "min_height");
    resize_constraints.min_height = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "max_height");
    resize_constraints.max_height = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return resize_constraints;
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
    dest_lt->lua_layout_copy_data_ref = 0;
    dest_lt->lua_layout_original_copy_data_ref = 0;
    dest_lt->lua_layout_ref = 0;
    dest_lt->lua_master_layout_data_ref = 0;
    dest_lt->lua_resize_function_ref = 0;
    copy_layout_safe(dest_lt, src_lt);
}

void copy_layout_safe(struct layout *dest_lt, struct layout *src_lt)
{
    if (!src_lt)
        return;

    if (src_lt->lua_layout_copy_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_copy_data_ref);
        lua_copy_table_safe(L, &dest_lt->lua_layout_copy_data_ref);
    }

    if (src_lt->lua_layout_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_ref);
        lua_copy_table_safe(L, &dest_lt->lua_layout_ref);
    }

    if (src_lt->lua_master_layout_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_master_layout_data_ref);
        lua_copy_table_safe(L, &dest_lt->lua_master_layout_data_ref);
    }

    if (src_lt->lua_layout_original_copy_data_ref > 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, src_lt->lua_layout_original_copy_data_ref);
        lua_copy_table_safe(L, &dest_lt->lua_layout_original_copy_data_ref);
    }

    if (src_lt->lua_master_layout_data_ref > 0) {
        lua_get_default_master_layout_data(L);
        lua_ref_safe(L, LUA_REGISTRYINDEX, &dest_lt->lua_master_layout_data_ref);
    }

    if (src_lt->lua_resize_data_ref > 0) {
        lua_get_default_resize_data(L);
        lua_ref_safe(L, LUA_REGISTRYINDEX, &dest_lt->lua_resize_data_ref);
    }

    if (src_lt->lua_resize_function_ref > 0) {
        lua_get_default_resize_function(L);
        lua_ref_safe(L, LUA_REGISTRYINDEX, &dest_lt->lua_resize_function_ref);
    }

    copy_options(dest_lt->options, src_lt->options);

    return;
}

bool lua_is_layout_data(lua_State *L, const char *name)
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

int cmp_layout(const void *ptr1, const void *ptr2)
{
    const struct layout *lt1 = ptr1;
    const struct layout *lt2 = ptr2;
    return strcmp(lt1->symbol, lt2->symbol) == 0;
}

int cmp_layout_to_string(const void *ptr1, const void *symbol_ptr)
{
    const struct layout *lt1 = ptr1;
    const char *symbol = symbol_ptr;
    return strcmp(lt1->symbol, symbol) == 0;
}
