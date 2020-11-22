#include "lib/actions/overlay.h"

#include <lua.h>

#include "tile/tileTexture.h"
#include "lib/actions/actions.h"

int write_this_overlay(lua_State *L)
{
    const char *layout = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    /* lua_pushstring(L, "bemenu-run"); */
    /* spawn(L); */
    write_overlay(selected_monitor, layout);
    return 0;
}

int set_overlay(lua_State *L)
{
    overlay = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int get_overlay(lua_State *L)
{
    lua_pushboolean(L, overlay);
    return 1;
}
