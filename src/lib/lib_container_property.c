#include "lib/lib_container_property.h"

#include "container.h"
#include "translationLayer.h"
#include "lib/lib_geom.h"
#include "tile/tileUtils.h"

static const struct luaL_Reg container_property_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg container_property_static_methods[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg container_property_m[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg container_property_setter[] =
{
    {"floating", lib_container_property_set_floating},
    {"geom", lib_container_property_set_geom},
    {NULL, NULL},
};

static const struct luaL_Reg container_property_getter[] =
{
    {"floating", lib_container_property_get_floating},
    {"geom", lib_container_property_get_geometry},
    {NULL, NULL},
};

void lua_load_container_property(lua_State *L)
{
    create_class(L,
            container_property_meta,
            container_property_static_methods,
            container_property_m,
            container_property_setter,
            container_property_getter,
            CONFIG_CONTAINER_PROPERTY);

    luaL_newlib(L, container_property_static_methods);
    lua_setglobal(L, "Container_property");
}

struct container_property *check_container_property(lua_State *L, int narg)
{
    void **ud = luaL_checkudata(L, narg, CONFIG_CONTAINER_PROPERTY);
    luaL_argcheck(L, ud != NULL, narg, "`container_property' expected");
    return *(struct container_property **)ud;
}

void create_lua_container_property(lua_State *L, struct container_property *container_property)
{
    struct container_property **user_container_property =
        lua_newuserdata(L, sizeof(struct container_property));
    *user_container_property = container_property;

    luaL_setmetatable(L, CONFIG_CONTAINER_PROPERTY);
}

// static methods
// methods
// getter
int lib_container_property_get_geometry(lua_State *L)
{
    struct container_property *property = check_container_property(L, 1);
    lua_pop(L, 1);

    create_lua_geometry(L, container_property_get_floating_geom(property));
    return 1;
}

int lib_container_property_get_floating(lua_State *L)
{
    struct container_property *property = check_container_property(L, 1);
    lua_pop(L, 1);

    bool is_floating = container_property_is_floating(property);
    lua_pushboolean(L, is_floating);
    return 1;
}
// setter
int lib_container_property_set_floating(lua_State *L)
{
    bool is_floating = lua_toboolean(L, -1);
    lua_pop(L, 1);

    struct container_property *property = check_container_property(L, 1);
    lua_pop(L, 1);

    container_property_set_floating(property, is_floating);

    arrange();
    return 0;
}

int lib_container_property_set_geom(lua_State *L)
{
    struct wlr_box *geom = check_geometry(L, -1);
    lua_pop(L, 1);

    struct container_property *property = check_container_property(L, 1);
    lua_pop(L, 1);

    container_property_set_floating_geom(property, geom);

    arrange();
    return 0;
}
