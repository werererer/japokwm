#include "lib/lib_container_list.h"

#include "lib/lib_list.h"
#include "lib/lib_container.h"

static void _create_lua_container(lua_State *L, void *con_ptr)
{
    struct container *con = con_ptr;
    create_lua_container(L, con);
}

static void *_check_container(lua_State *L, int argn)
{
    void *con_ptr = check_container(L, argn);
    return con_ptr;
}

void create_lua_container_list(lua_State *L, GPtrArray *array)
{
    create_lua_list(L, array, _create_lua_container, _check_container);
}
