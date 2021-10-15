#include "lib/lib_color.h"

#include "translationLayer.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "color.h"

static const struct luaL_Reg color_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg color_f[] =
{
    {"new", lib_color_new},
    {NULL, NULL},
};

static const struct luaL_Reg color_m[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg color_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg color_getter[] =
{
    {NULL, NULL},
};

void lua_load_color()
{
    create_class(color_meta,
            color_f,
            color_m,
            color_setter,
            color_getter,
            CONFIG_COLOR);

    luaL_newlib(L, color_f);
    lua_setglobal(L, "Color");
}

struct color check_color(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_COLOR);
    luaL_argcheck(L, ud != NULL, 1, "`container' expected");
    return *(struct color *)ud;
}

static void create_lua_color(struct color color) {
    struct color *user_color = lua_newuserdata(L, sizeof(struct color));
    *user_color = color;

    luaL_setmetatable(L, CONFIG_COLOR);
}

// functions
int lib_color_new(lua_State *L)
{
    float alpha = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    float blue = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    float green = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    float red = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    struct color color = {
        .red = red,
        .green = green,
        .blue = blue, 
        .alpha = alpha,
    };
    create_lua_color(color);
    return 1;
}
