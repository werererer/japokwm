#include "lib/lib_root.h"

#include "translationLayer.h"
#include "root.h"
#include "lib/lib_geom.h"

static const struct luaL_Reg root_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg root_static_methods[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg root_methods[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg root_setter[] =
{
    {"area", lib_root_get_area},
    {NULL, NULL},
};

static const struct luaL_Reg root_getter[] =
{
    {NULL, NULL},
};

void lua_load_root(lua_State *L)
{
    create_class(L,
            root_meta,
            root_static_methods,
            root_methods,
            root_setter,
            root_getter,
            CONFIG_ROOT);

    luaL_newlib(L, root_static_methods);
    lua_setglobal(L, "Root");
}

struct root *check_root(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_ROOT);
    luaL_argcheck(L, ud != NULL, narg, "`root' expected");
    return *(struct root **)ud;
}

void create_lua_root(lua_State *L, struct root *root)
{
    struct root **user_root = lua_newuserdata(L, sizeof(struct root));
    *user_root = root;

    luaL_setmetatable(L, CONFIG_ROOT);
}

// static methods
// methods
// getter
// setter
int lib_root_get_area(lua_State *L)
{
    struct root *root = check_root(L, 1);
    lua_pop(L, 1);
    create_lua_geometry(L, root->geom);
    return 1;
}
