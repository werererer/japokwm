#include "lib/lib_direction.h"

#include <wlr/util/edges.h>

#include "translationLayer.h"

static const struct luaL_Reg direction_getter[] =
{
    {"all", lib_direction_get_all},
    {"bottom", lib_direction_get_bottom},
    {"horizontal", lib_direction_get_horizontal},
    {"left", lib_direction_get_left},
    {"none", lib_direction_get_none},
    {"right", lib_direction_get_right},
    {"top", lib_direction_get_top},
    {"vertical", lib_direction_get_vertical},
    {NULL, NULL},
};

void lua_load_direction(lua_State *L)
{
    create_enum(L, direction_getter, CONFIG_DIRECTION);

    lua_createtable(L, 0, 0);
    luaL_setmetatable(L, CONFIG_DIRECTION);
    lua_setglobal(L, "Direction");
}

// getter
int lib_direction_get_all(lua_State *L)
{
    enum wlr_edges edges =
        WLR_EDGE_TOP |
        WLR_EDGE_BOTTOM |
        WLR_EDGE_LEFT |
        WLR_EDGE_RIGHT;
    lua_pushinteger(L, edges);
    return 1;
}

int lib_direction_get_bottom(lua_State *L)
{
    lua_pushinteger(L, WLR_EDGE_BOTTOM);
    return 1;
}

int lib_direction_get_horizontal(lua_State *L)
{
    enum wlr_edges edges =
        WLR_EDGE_LEFT |
        WLR_EDGE_RIGHT;
    lua_pushinteger(L, edges);
    return 1;
}

int lib_direction_get_left(lua_State *L)
{
    lua_pushinteger(L, WLR_EDGE_LEFT);
    return 1;
}

int lib_direction_get_none(lua_State *L)
{
    lua_pushinteger(L, WLR_EDGE_NONE);
    return 1;
}

int lib_direction_get_right(lua_State *L)
{
    lua_pushinteger(L, WLR_EDGE_RIGHT);
    return 1;
}

int lib_direction_get_top(lua_State *L)
{
    lua_pushinteger(L, WLR_EDGE_TOP);
    return 1;
}

int lib_direction_get_vertical(lua_State *L)
{
    enum wlr_edges edges =
        WLR_EDGE_TOP |
        WLR_EDGE_BOTTOM;
    lua_pushinteger(L, edges);
    return 1;
}
