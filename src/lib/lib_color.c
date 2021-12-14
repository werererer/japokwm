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

static const struct luaL_Reg color_static_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg color_static_getter[] =
{
    {"BLACK", lib_color_get_black},
    {"WHITE", lib_color_get_white},
    {"RED", lib_color_get_red},
    {"GREEN", lib_color_get_green},
    {"BLUE", lib_color_get_blue},
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

void lua_load_color(lua_State *L)
{
    create_class(L,
            color_meta,
            color_f,
            color_m,
            color_setter,
            color_getter,
            CONFIG_COLOR);

    create_static_accessor(L,
            "Color",
            color_f,
            color_static_setter,
            color_static_getter);

    luaL_newlib(L, color_f);
    lua_setglobal(L, "Color");
}

struct color check_color(lua_State *L, int narg)
{
    void **ud = luaL_checkudata(L, narg, CONFIG_COLOR);
    luaL_argcheck(L, ud != NULL, narg, "`color' expected");
    return *(struct color *)ud;
}

static void create_lua_color(lua_State *L, struct color color)
{
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
    create_lua_color(L, color);
    return 1;
}

// static setters
// static getters
int lib_color_get_black(lua_State *L)
{
    struct color color = BLACK;
    create_lua_color(L, color);
    return 1;
}

int lib_color_get_white(lua_State *L)
{
    struct color color = WHITE;
    create_lua_color(L, color);
    return 1;
}

int lib_color_get_red(lua_State *L)
{
    struct color color = RED;
    create_lua_color(L, color);
    return 1;
}

int lib_color_get_green(lua_State *L)
{
    struct color color = GREEN;
    create_lua_color(L, color);
    return 1;
}

int lib_color_get_blue(lua_State *L)
{
    struct color color = BLUE;
    create_lua_color(L, color);
    return 1;
}
