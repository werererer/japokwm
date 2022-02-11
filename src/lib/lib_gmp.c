#include "lib/lib_gmp.h"

#include "translationLayer.h"

static const struct luaL_Reg gmp_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg gmp_static_methods[] =
{
    {"new", lib_gmp_new},
    {NULL, NULL},
};

static const struct luaL_Reg gmp_static_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg gmp_static_getter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg gmp_methods[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg gmp_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg gmp_getter[] =
{
    {NULL, NULL},
};

void lua_load_gmp(lua_State *L)
{
    create_class(L,
            gmp_meta,
            gmp_static_methods,
            gmp_methods,
            gmp_setter,
            gmp_getter,
            CONFIG_GMP);

    create_static_accessor(L,
            "Gmp",
            gmp_static_methods,
            gmp_static_setter,
            gmp_static_getter);
}

static void create_lua_gmp(lua_State *L, mpz_t *gmp)
{
    lua_newtable(L);
    lua_pushlightuserdata(L, gmp);
    lua_setfield(L, -2, "__gmp");
}

int lib_gmp_new(lua_State *L)
{
    mpz_t *mpz = lua_newuserdata(L, sizeof(mpz_t));
    mpz_init(*mpz);

    luaL_getmetatable(L, CONFIG_GMP);
    lua_setmetatable(L, -2);

    return 1;
}

struct color check_gmp(lua_State *L, int narg);
