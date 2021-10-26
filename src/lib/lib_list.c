#include "lib/lib_list.h"

#include "translationLayer.h"
#include "server.h"
#include "lib/lib_container.h"
#include "utils/coreUtils.h"
#include "tile/tileUtils.h"
#include "tagset.h"
#include "workspace.h"

#include <GLES2/gl2.h>
#include <stdlib.h>
#include <ctype.h>

static const struct luaL_Reg list_meta[] =
{
    {"__index", lib_list_get},
    {NULL, NULL},
};

static const struct luaL_Reg list_f[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg list_m[] = {
    {"find", lib_list_find},
    {"get", lib_list_get},
    {"repush", lib_list_repush},
    {"swap", lib_list_swap},
    {NULL, NULL},
};

static const struct luaL_Reg list_getter[] = {
    {"len", lib_list_length},
    {NULL, NULL},
};

static const struct luaL_Reg list_setter[] = {
    {NULL, NULL},
};

void create_lua_list(lua_State *L, GPtrArray *arr)
{
    if (!arr) {
        lua_pushnil(L);
        return;
    }
    GPtrArray **user_con = lua_newuserdata(L, sizeof(GPtrArray *));
    *user_con = arr;

    luaL_setmetatable(L, CONFIG_LIST);
}

void lua_load_list(lua_State *L)
{
    create_class(L,
            list_meta,
            list_f,
            list_m,
            list_setter,
            list_getter,
            CONFIG_LIST);

    luaL_newlib(L, list_f);
    lua_setglobal(L, "List");
}

GPtrArray *check_list(lua_State *L, int argn)
{
    void **ud = luaL_checkudata(L, argn, CONFIG_LIST);
    luaL_argcheck(L, ud != NULL, argn, "`list' expected");
    return (GPtrArray *)*ud;
}

// functions
// methods
int lib_list_find(lua_State *L)
{
    struct container *con = check_container(L, 2);
    lua_pop(L, 1);
    GPtrArray *array = check_list(L, 1);
    lua_pop(L, 1);

    guint pos;
    g_ptr_array_find(array, con, &pos);

    lua_pushinteger(L, c_idx_to_lua_idx(pos));
    return 1;
}

int lib_list_get(lua_State *L)
{
    const char *key = luaL_checkstring(L, -1); // convert lua to c index
    GPtrArray *array = check_list(L, 1);
    debug_print("key: %s\n", key);

    bool is_number = lua_isnumber(L, -1);
    if (!is_number) {
        get_lua_value(L);
        return 1;
    }

    int i = lua_idx_to_c_idx(lua_tonumber(L, -1));
    if (i < 0) {
        lua_pushnil(L);
        return 1;
    }
    if (i >= array->len) {
        lua_pushnil(L);
        return 1;
    }

    struct container *con = g_ptr_array_index(array, i);
    create_lua_container(L, con);
    return 1;
}

int lib_list_swap(lua_State *L)
{
    // int j = luaL_checkinteger(L, -1);
    // lua_pop(L, 1);
    // int i = luaL_checkinteger(L, -1);
    // lua_pop(L, 1);
    // GPtrArray *array = check_list(L, 1);
    // lua_pop(L, 1);

    // struct container *con 
    // g_ptr_array_index(array)
    return 0;
}

int lib_list_repush(lua_State *L)
{
    int abs_index = lua_idx_to_c_idx(luaL_checkinteger(L, -1));
    lua_pop(L, 1);
    int i = lua_idx_to_c_idx(luaL_checkinteger(L, -1));
    lua_pop(L, 1);
    GPtrArray *array = check_list(L, 1);
    lua_pop(L, 1);

    workspace_repush(array, i, abs_index);

    struct workspace *ws = server_get_selected_workspace();
    tagset_reload(ws);
    arrange();
    return 0;
}

// getter
int lib_list_length(lua_State *L)
{
    GPtrArray* list = check_list(L, 1);
    lua_pop(L, 1);
    lua_pushinteger(L, list->len);
    return 1;
}
// setter
