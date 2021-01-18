#include "lib/config/config.h"
#include "utils/gapUtils.h"
#include "server.h"

int set_gaps(lua_State *L)
{
    server.options.outer_gap = luaL_checkinteger(L ,-1);
    server.options.inner_gap = luaL_checkinteger(L ,-1);
    configure_gaps(&server.options.inner_gap, &server.options.outer_gap);
    return 0;
}

int set_borderpx(lua_State *L)
{
    server.options.border_px = luaL_checkinteger(L, -1);
    return 0;
}

int set_sloppy_focus(lua_State *L)
{
    server.options.sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}
