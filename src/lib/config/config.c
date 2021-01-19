#include "lib/config/config.h"
#include "utils/gapUtils.h"
#include "utils/coreUtils.h"
#include "server.h"

int lib_set_gaps(lua_State *L)
{
    server.options.outer_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    server.options.inner_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    configure_gaps(&server.options.inner_gap, &server.options.outer_gap);
    return 0;
}

int lib_set_borderpx(lua_State *L)
{
    server.options.border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_focus_color(lua_State *L)
{
    lua_get_color(server.options.focus_color);
    return 0;
}

int lib_set_border_color(lua_State *L)
{
    lua_get_color(server.options.border_color);
    return 0;
}

int lib_set_root_color(lua_State *L)
{
    lua_get_color(server.options.root_color);
    return 0;
}

int lib_set_sloppy_focus(lua_State *L)
{
    server.options.sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}
