#include <glib.h>
#include <lua.h>
#include <lauxlib.h>

#include "layout.h"
#include "utils/parseConfigUtils.h"
#include "server.h"

/*
   first create a table like this: 
   {
     test_int = 0,
     test_str = "original",
     test_table = {test_table = "inner"}
   }
   create a copy and change values:
   {
     test_int = 1,
     test_str = "copied",
     test_table = {test_table = "copied inner"}
   }
   then compare each field. they should differ
*/
void test_deep_copy_table()
{
    const char *str_test_field = "test_str";
    const char *int_test_field = "test_int";
    const char *table_test_field = "test_table";

    lua_State *L = luaL_newstate();
    init_server();
    luaL_openlibs(L);

    lua_createtable(L, 0, 0);
    // [table]
    lua_pushstring(L, str_test_field);
    // [table, k]
    lua_pushstring(L, "original");
    // [table, k, v]
    lua_settable(L, -3);
    // [table]
    lua_pushstring(L, int_test_field);
    // [table, k]
    lua_pushinteger(L, 0);
    // [table, k, v]
    lua_settable(L, -3);
    // [table]
    lua_pushstring(L, table_test_field);
    // [table, k]
    {
        lua_createtable(L, 0, 0);
        lua_pushstring(L, str_test_field);
        lua_pushstring(L, "inner");
        lua_settable(L, -3);
    }
    // [table, k, v]
    lua_settable(L, -3);
    // [table]

    lua_pushcfunction(L, deep_copy_table);
    // [table, cfunc]
    lua_pushvalue(L, -2);
    // [table, cfunc, table]
    lua_call(L, 1, 1);
    // [table, copied_table]

    lua_pushstring(L, str_test_field);
    // [table, copied_table, k]
    lua_pushstring(L, "changed");
    // [table, copied_table, k, v]
    lua_settable(L, -3);
    // [table, copied_table]
    lua_pushstring(L, int_test_field);
    // [table, copied_table, k]
    lua_pushinteger(L, 1);
    // [table, copied_table, k, v]
    lua_settable(L, -3);
    // [table, copied_table]
    lua_getfield(L, 2, table_test_field);
    {
        // [table, copied_table, copied_table.inner_table]
        lua_pushstring(L, str_test_field);
        lua_pushstring(L, "copied inner");
        lua_settable(L, -3);
    }
    lua_pop(L, 1);
    // [table, copied_table]

    // [table, copied_table]
    lua_getfield(L, 1, str_test_field);
    // [table, copied_table, table.test_field]
    const char *original_str = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    // [table, copied_table]
    lua_getfield(L, 1, int_test_field);
    // [table, copied_table, v]
    int original_int = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    // [table, copied_table]
    lua_getfield(L, 1, table_test_field);
    // [table, copied_table, table.inner_table]
    lua_getfield(L, -1, str_test_field);
    // [table, copied_table, table.inner_table, table.inner_table.str]
    const char *original_inner_str = luaL_checkstring(L, -1);
    lua_pop(L, 2);
    // [table, copied_table]

    lua_getfield(L, 2, str_test_field);
    // [table, copied_table, copied_table.test_field]
    const char *copied_str = luaL_checkstring(L, -1);
    // [table, copied_table]
    lua_pop(L, 1);
    lua_getfield(L, 2, int_test_field);
    // [table, copied_table, v]
    int copied_int = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    // [table, copied_table]
    lua_getfield(L, 2, table_test_field);
    // [table, copied_table, copied_table.inner_table]
    lua_getfield(L, -1, str_test_field);
    // [table, copied_table, copied_table.inner_table, copied_table.inner_table.str]
    const char *copied_inner_str = luaL_checkstring(L, -1);
    lua_pop(L, 2);
    // [table, copied_table]

    g_assert_cmpint(original_int, !=, copied_int);
    g_assert_cmpstr(original_str, !=, copied_str);
    g_assert_cmpstr(original_inner_str, !=, copied_inner_str);
    lua_pop(L, 2);
}

#define PREFIX "layout"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(test_deep_copy_table);

    return g_test_run();
}
