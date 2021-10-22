#include "lib/lib_list2D.h"

#include "translationLayer.h"
#include "server.h"
#include "lib/lib_container.h"
#include "utils/coreUtils.h"
#include "tile/tileUtils.h"
#include "tagset.h"

#include <stdlib.h>
#include <ctype.h>

static const struct luaL_Reg list2D_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg list2D_f[] =
{
    {"__index", lib_list2D_get},
    {NULL, NULL},
};

static const struct luaL_Reg list2D_m[] = {
    {"find", lib_list2D_find},
    {"get", lib_list2D_get},
    {"repush", lib_list2D_repush},
    {"swap", lib_list2D_swap},
    {NULL, NULL},
};

static const struct luaL_Reg list2D_getter[] = {
    {"len", lib_list2D_length},
    {NULL, NULL},
};

static const struct luaL_Reg list2D_setter[] = {
    {NULL, NULL},
};

void create_lua_list2D(lua_State *L, GPtrArray2D *arr)
{
    if (!arr) {
        lua_pushnil(L);
        return;
    }
    GPtrArray **user_con = lua_newuserdata(L, sizeof(GPtrArray *));
    *user_con = arr;

    luaL_setmetatable(L, CONFIG_LIST2D);
}

void lua_load_list2D()
{
    create_class(
            list2D_meta,
            list2D_f,
            list2D_m,
            list2D_setter,
            list2D_getter,
            CONFIG_LIST);

    luaL_newlib(L, list2D_f);
    lua_setglobal(L, "List2D");
}

GPtrArray *check_list2D(lua_State *L, int argn)
{
    void **ud = luaL_checkudata(L, argn, CONFIG_LIST);
    luaL_argcheck(L, ud != NULL, argn, "`list' expected");
    return (GPtrArray *)*ud;
}

// functions
int lib_list2D_to_array(lua_State *L)
{
    GPtrArray *array = check_list2D(L, 1);
    lua_pop(L, 1);
    lua_createtable(L, array->len, 0);
    for (int i = 0; i < array->len; i++) {
        struct container *con = get_in_composed_list(array, i);
        create_lua_container(L, con);
        lua_rawseti (L, -2, i+1); /* In lua indices start at 1 */
    }
    return 0;
}

// methods
int lib_list2D_find(lua_State *L)
{
    struct container *con = check_container(L, 2);
    lua_pop(L, 1);
    GPtrArray *array = check_list2D(L, 1);
    lua_pop(L, 1);

    guint pos;
    g_ptr_array_find(array, con, &pos);

    lua_pushinteger(L, pos);
    return 1;
}

int lib_list2D_get(lua_State *L)
{
    const char *key = luaL_checkstring(L, -1); // convert lua to c index
    GPtrArray *array = check_list2D(L, 1);
    debug_print("key: %s\n", key);

    bool is_number = lua_isnumber(L, -1);
    if (!is_number) {
        get_lua_value(L);
        return 1;
    }
    int i = lua_tonumber(L, -1)-1;

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

int lib_list2D_swap(lua_State *L)
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

int lib_list2D_repush(lua_State *L)
{
    int abs_index = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    GPtrArray *array = check_list2D(L, 1);
    lua_pop(L, 1);

    struct container *con = g_ptr_array_steal_index(array, i);
    g_ptr_array_insert(array, abs_index, con);

    struct workspace *ws = server_get_selected_workspace();
    tagset_reload(ws);
    arrange();
    return 0;
}

// getter
int lib_list2D_length(lua_State *L)
{
    GPtrArray* list = check_list2D(L, 1);
    lua_pop(L, 1);
    lua_pushinteger(L, list->len);
    return 1;
}
// setter
