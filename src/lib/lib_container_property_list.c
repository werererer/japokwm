#include "lib/lib_container_property_list.h"

#include "lib/lib_list.h"
#include "lib/lib_container_property.h"

static void _create_lua_container_property(lua_State *L, void *con_ptr)
{
    struct container_property *property = con_ptr;
    create_lua_container_property(L, property);
}

static void *_check_container_property(lua_State *L, int argn)
{
    void *property_ptr = check_container_property(L, argn);
    return property_ptr;
}

void create_lua_container_property_list(lua_State *L, GPtrArray *array)
{
    create_lua_list(
            L,
            array,
            _create_lua_container_property,
            _check_container_property);
}
