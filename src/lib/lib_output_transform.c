#include "lib/lib_output_transform.h"

#include "translationLayer.h"
#include "server.h"

static const struct luaL_Reg output_transform_getter[] =
{
    {"normal", lib_output_transform_get_normal},
    {"tranform_90", lib_output_transform_get_tranform_90},
    {"transform_180", lib_output_transform_get_transform_180},
    {"transform_270", lib_output_transform_get_transform_270},
    {"transform_flipped", lib_output_transform_get_transform_flipped},
    {"transform_flipped_90", lib_output_transform_get_transform_flipped_90},
    {"transform_flipped_180", lib_output_transform_get_transform_flipped_180},
    {"transform_flipped_270", lib_output_transform_get_transform_flipped_270},
    {NULL, NULL},
};

void lua_load_output_transform(lua_State *L)
{
    create_enum(L, output_transform_getter, CONFIG_OUTPUT_TRANSFORM);

    lua_createtable(L, 0, 0);
    luaL_setmetatable(L, CONFIG_DIRECTION);
    lua_setglobal(L, "Output_transform");
}

enum wl_output_transform check_output_transform(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_OUTPUT_TRANSFORM);
    luaL_argcheck(L, ud != NULL, narg, "`output_transform' expected");
    return *(enum wl_output_transform *)ud;
}

static void create_lua_output_transform(lua_State *L, enum wl_output_transform output_transform) {
    enum wl_output_transform *user_output_transform = lua_newuserdata(L, sizeof(enum wl_output_transform));
    *user_output_transform = output_transform;

    luaL_setmetatable(L, CONFIG_OUTPUT_TRANSFORM);
}

// getter
int lib_output_transform_get_normal(lua_State *L)
{
    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_NORMAL);
    return 0;
}

int lib_output_transform_get_tranform_90(lua_State *L)
{
    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_90);
    return 0;
}

int lib_output_transform_get_transform_180(lua_State *L)
{
    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_180);
    return 0;
}

int lib_output_transform_get_transform_270(lua_State *L)
{
    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_270);
    return 0;
}

int lib_output_transform_get_transform_flipped(lua_State *L)
{
    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_FLIPPED);
    return 0;
}

int lib_output_transform_get_transform_flipped_90(lua_State *L)
{
    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_FLIPPED_90);
    return 0;
}

int lib_output_transform_get_transform_flipped_180(lua_State *L)
{
    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_FLIPPED_180);
    return 0;
}

int lib_output_transform_get_transform_flipped_270(lua_State *L)
{
    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_FLIPPED_270);
    return 0;
}
