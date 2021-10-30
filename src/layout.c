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
    *lt = (struct layout) {
        .current_max_area = -1,
        .name = "",
        .n_area = 1,
        .n_master = 1,
    };
    lt->linked_layouts = g_ptr_array_new();
    lt->linked_loaded_layouts = g_ptr_array_new();

    lt->options = create_options();

    lua_get_default_master_layout_data(L);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_master_layout_data_ref);

    lua_get_default_resize_data(L);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_resize_data_ref);

    lua_get_default_layout_data(L);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_layout_original_copy_data_ref);

    lua_get_default_layout_data(L);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_layout_copy_data_ref);

    lua_get_default_resize_function(L);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_resize_function_ref);

    return lt;
}

void destroy_layout(struct layout *lt)
{
    destroy_options(lt->options);

    g_ptr_array_unref(lt->linked_layouts);
    g_ptr_array_unref(lt->linked_loaded_layouts);

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

// This function represents something like that
// function Deep_copy(orig)
//     local orig_type = type(orig)
//     local copy
//     if orig_type == 'table' then
//         copy = {}
//         for orig_key, orig_value in next, orig, nil do
//             copy[Deep_copy(orig_key)] = Deep_copy(orig_value)
//         end
//         setmetatable(copy, Deep_copy(getmetatable(orig)))
//     else -- number, string, boolean, etc
//         copy = orig
//     end
//     return copy
// end
//
int deep_copy_table(lua_State *L)
{
    // this function takes one argument
    // [table]
    if (lua_istable(L, 1)) {
        // local copy = {}
        lua_createtable(L, 0, 0);
        // [table, copy]

        // lenght of the original table

        // first key
        lua_pushnil(L);
        while (lua_next(L, 1) != 0) {
            // [table, copy, k, v]
            // we need to keep a key for lua_next to work so make a copy of it
            lua_pushvalue(L, -2);
            // [table, copy, k, v, k]

            // get copied key
            lua_pushcfunction(L, deep_copy_table);
            // [table, copy, k, v, k, cfunc]
            lua_insert(L, -2);
            // [table, copy, k, v, cfunc, k]
            lua_call_safe(L, 1, 1, 0);
            // [table, copy, k, v, copied_k]
            lua_insert(L, -2);
            // [table, copy, k, copied_k, v]

            // get copied value
            lua_pushcfunction(L, deep_copy_table);
            // [table, copy, k, copied_k, v, cfunc]
            lua_insert(L, -2);
            // [table, copy, k, copied_k, cfunc, v]
            lua_call_safe(L, 1, 1, 0);
            // [table, copy, k, copied_k, copied_v]
            lua_settable(L, 2);

            // we need to keep a key else lua_next won't work
            // [table, copy, k]
        }

        // [table, copy]
        if (lua_getmetatable(L, 1) == 1) {
            // [table, copy, metatable]
            // get copied value
            lua_pushcfunction(L, deep_copy_table);
            // [table, copy, metatable, cfunc]
            lua_insert(L, -2);
            // [table, copy, cfunc, metatable]
            lua_call_safe(L, 1, 1, 0);
            // [table, copy, metatable_copy]
            lua_setmetatable(L, -2);
            // [table, copy]
        }
        // [table, copy]
        return 1;
    } else {
        // we only copy tables here ;)
        return 1;
    }
    return 1;
}

void lua_copy_table_safe(lua_State *L, int *ref)
{
    assert(lua_istable(L, -1));
    lua_pushcfunction(L, deep_copy_table);
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
    const char *c = layout.name;
    const char *c2 = layout2.name;
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
    return strcmp(lt1->name, lt2->name) == 0;
}

int cmp_layout_to_string(const void *ptr1, const void *symbol_ptr)
{
    const struct layout *lt1 = ptr1;
    const char *symbol = symbol_ptr;
    return strcmp(lt1->name, symbol) == 0;
}
