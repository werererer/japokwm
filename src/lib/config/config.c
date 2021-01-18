#include "lib/config/config.h"
#include "utils/gapUtils.h"
#include "server.h"

int set_sloppy_focus(lua_State *L)
{
    return 0;
}

int set_gaps(lua_State *L)
{
    server.options.outer_gap = luaL_checkinteger(L ,-1);
    server.options.inner_gap = luaL_checkinteger(L ,-1);
    configure_gaps(&server.options.inner_gap, &server.options.outer_gap);
    return 0;
}
