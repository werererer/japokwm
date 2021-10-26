#include "lib/lib_screen_transform.h"

#include <wayland-server.h>

#include "translationLayer.h"

static const struct luaL_Reg screen_transform_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg screen_transform_f[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg screen_transform_m[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg screen_transform_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg screen_transform_getter[] =
{
    {NULL, NULL},
};

void lua_load_screen_transform(lua_State *L)
{
    create_class(L, screen_transform_meta,
            screen_transform_f,
            screen_transform_m,
            screen_transform_setter,
            screen_transform_getter,
            CONFIG_OUTPUT_TRANSFORM);

    luaL_newlib(L, screen_transform_f);
    lua_setglobal(L, "screen_transform");
}

enum wl_output_transform check_screen_transform(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_OUTPUT_TRANSFORM);
    luaL_argcheck(L, ud != NULL, narg, "`screen_transform' expected");
    return *(enum wl_output_transform *)ud;
}

static void create_lua_screen_transform(lua_State *L, enum wl_output_transform screen_transform) {
    enum wl_output_transform *user_screen_transform = lua_newuserdata(L, sizeof(enum wl_output_transform));
    *user_screen_transform = screen_transform;

    luaL_setmetatable(L, CONFIG_OUTPUT_TRANSFORM);
}

