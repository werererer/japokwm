#include "lib/lib_gmp.h"

#include "translationLayer.h"

#define ROUNDING_METHOD MPFR_RNDF

static const struct luaL_Reg gmp_meta[] =
{
    {"__gc", lib_gmp_gc},
    {"__tostring", lib_gmp_tostring},
    {"__add", lib_gmp_add},
    {"__sub", lib_gmp_sub},
    {"__mul", lib_gmp_mul},
    {"__div", lib_gmp_div},
    {"__pow", lib_gmp_pow},
    {"__lt", lib_gmp_lt},
    {"__le", lib_gmp_le},
    {NULL, NULL},
};

static const struct luaL_Reg gmp_static_methods[] =
{
    {"new", lib_gmp_new},
    {"max", lib_gmp_max},
    {"min", lib_gmp_min},
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
    {"value", lib_gmp_get_value},
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

mpfr_ptr check_gmp(lua_State *L, int argn)
{
    void *ud = luaL_checkudata(L, argn, CONFIG_GMP);
    luaL_argcheck(L, ud != NULL, argn, "`mpfr' expected");
    return (mpfr_ptr)ud;
}

static void create_lua_gmp(lua_State *L, mpfr_t num)
{
    mpfr_ptr mpfr = (mpfr_ptr)lua_newuserdata(L, sizeof(mpfr_t));
    mpfr_init2(mpfr, mpfr_get_prec(num));
    mpfr_set(mpfr, num, ROUNDING_METHOD);

    luaL_setmetatable(L, CONFIG_GMP);
}

// meta
int lib_gmp_gc(lua_State *L)
{
    mpfr_ptr n = check_gmp(L, 1);
    mpfr_clear(n);
    return 0;
}

int lib_gmp_tostring(lua_State *L)
{
    mpfr_ptr n = check_gmp(L, 1);
    char data[255];
    mpfr_snprintf(data, 255, "%.*Rf", 10, n);
    lua_pushstring(L, data);
    return 1;
}

int lib_gmp_add(lua_State *L)
{
    mpfr_ptr n1 = check_gmp(L, 1);
    mpfr_ptr n2 = check_gmp(L, 2);

    mpfr_t n3;
    mpfr_init(n3);
    mpfr_add(n3, n1, n2, ROUNDING_METHOD);
    create_lua_gmp(L, n3);

    return 1;
}

int lib_gmp_sub(lua_State *L)
{
    mpfr_ptr n1 = check_gmp(L, 1);
    mpfr_ptr n2 = check_gmp(L, 2);

    mpfr_t n3;
    mpfr_init(n3);
    mpfr_sub(n3, n1, n2, ROUNDING_METHOD);
    create_lua_gmp(L, n3);
    return 1;
}

int lib_gmp_mul(lua_State *L)
{
    mpfr_ptr n1 = check_gmp(L, 1);
    mpfr_ptr n2 = check_gmp(L, 2);

    mpfr_t n3;
    mpfr_init(n3);
    mpfr_mul(n3, n1, n2, ROUNDING_METHOD);
    create_lua_gmp(L, n3);
    return 1;
}

int lib_gmp_div(lua_State *L)
{
    mpfr_ptr n1 = check_gmp(L, 1);
    mpfr_ptr n2 = check_gmp(L, 2);

    mpfr_t n3;
    mpfr_init(n3);
    mpfr_div(n3, n1, n2, ROUNDING_METHOD);
    create_lua_gmp(L, n3);
    return 1;
}

int lib_gmp_pow(lua_State *L)
{
    mpfr_ptr n1 = check_gmp(L, 1);
    mpfr_ptr n2 = check_gmp(L, 2);

    mpfr_t n3;
    mpfr_init(n3);
    mpfr_pow(n3, n1, n2, ROUNDING_METHOD);
    create_lua_gmp(L, n3);
    return 1;
}

int lib_gmp_lt(lua_State *L)
{
    mpfr_ptr n1 = check_gmp(L, 1);
    mpfr_ptr n2 = check_gmp(L, 2);

    lua_pushboolean(L, mpfr_less_p(n1, n2));
    return 1;
}

int lib_gmp_le(lua_State *L)
{
    mpfr_ptr n1 = check_gmp(L, 1);
    mpfr_ptr n2 = check_gmp(L, 2);

    lua_pushboolean(L, mpfr_lessequal_p(n1, n2));
    return 1;
}

// static methods
int lib_gmp_new(lua_State *L)
{
    double n = luaL_checknumber(L, 1);
    lua_pop(L, 1);

    mpfr_t num;
    mpfr_init(num);
    mpfr_set_d(num, n, ROUNDING_METHOD);
    create_lua_gmp(L, num);

    return 1;
}

int lib_gmp_max(lua_State *L)
{
    mpfr_ptr n1 = check_gmp(L, 1);
    mpfr_ptr n2 = check_gmp(L, 2);

    mpfr_t n3;
    mpfr_init(n3);
    mpfr_max(n3, n1, n2, ROUNDING_METHOD);
    create_lua_gmp(L, n3);
    return 1;
}

int lib_gmp_min(lua_State *L)
{
    mpfr_ptr n1 = check_gmp(L, 1);
    mpfr_ptr n2 = check_gmp(L, 2);

    mpfr_t n3;
    mpfr_init(n3);
    mpfr_min(n3, n1, n2, ROUNDING_METHOD);
    create_lua_gmp(L, n3);
    return 1;
}

// getter
int lib_gmp_get_value(lua_State *L)
{
    mpfr_ptr n = check_gmp(L, 1);
    lua_pushnumber(L, mpfr_get_d(n, ROUNDING_METHOD));
    return 1;
}
