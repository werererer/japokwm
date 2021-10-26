#include "lib/lib_geom.h"

#include "translationLayer.h"
#include "server.h"

static const struct luaL_Reg geom_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg geom_f[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg geom_m[] = {
    {NULL, NULL},
};

static const struct luaL_Reg geom_getter[] = {
    {"x", lib_geometry_get_x},
    {"y", lib_geometry_get_y},
    {"width", lib_geometry_get_width},
    {"height", lib_geometry_get_height},
    {NULL, NULL},
};

static const struct luaL_Reg geom_setter[] = {
    {"x", lib_geometry_set_x},
    {"y", lib_geometry_set_y},
    {"width", lib_geometry_set_width},
    {"height", lib_geometry_set_height},
    {NULL, NULL},
};

void create_lua_geometry(lua_State *L, struct wlr_box *geom)
{
    if (!geom) {
        lua_pushnil(L);
        return;
    }
    struct wlr_box **user_con = lua_newuserdata(L, sizeof(struct wlr_box*));
    *user_con = geom;

    luaL_setmetatable(L, CONFIG_GEOMETRY);
}

void lua_load_geom(lua_State *L)
{
    create_class(L,
            geom_meta,
            geom_f,
            geom_m,
            geom_setter,
            geom_getter,
            CONFIG_GEOMETRY);

    luaL_newlib(L, geom_f);
    lua_setglobal(L, "Geom");
}

struct wlr_box *check_geometry(lua_State *L, int argn)
{
    void **ud = luaL_checkudata(L, argn, CONFIG_GEOMETRY);
    luaL_argcheck(L, ud != NULL, argn, "`geometry' expected");
    return (struct wlr_box *)*ud;
}

// functions
// methods
// getter
int lib_geometry_get_x(lua_State *L)
{
    struct wlr_box *geom = check_geometry(L, 1);
    lua_pop(L, 1);

    lua_pushinteger(L, geom->x);
    return 1;
}

int lib_geometry_get_y(lua_State *L)
{
    struct wlr_box *geom = check_geometry(L, 1);
    lua_pop(L, 1);

    lua_pushinteger(L, geom->y);
    return 1;
}

int lib_geometry_get_width(lua_State *L)
{
    struct wlr_box *geom = check_geometry(L, 1);
    lua_pop(L, 1);

    lua_pushinteger(L, geom->width);
    return 1;
}

int lib_geometry_get_height(lua_State *L)
{
    struct wlr_box *geom = check_geometry(L, 1);
    lua_pop(L, 1);

    lua_pushinteger(L, geom->height);
    return 1;
}

// setter
int lib_geometry_set_x(lua_State *L)
{
    int x = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct wlr_box *geom = check_geometry(L, 1);
    lua_pop(L, 1);

    geom->x = x;
    return 0;
}

int lib_geometry_set_y(lua_State *L)
{
    int y = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct wlr_box *geom = check_geometry(L, 1);
    lua_pop(L, 1);

    geom->y = y;
    return 0;
}

int lib_geometry_set_width(lua_State *L)
{
    int width = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct wlr_box *geom = check_geometry(L, 1);
    lua_pop(L, 1);

    geom->width = width;
    return 0;
}

int lib_geometry_set_height(lua_State *L)
{
    int height = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct wlr_box *geom = check_geometry(L, 1);
    lua_pop(L, 1);

    geom->height = height;
    return 0;
}
