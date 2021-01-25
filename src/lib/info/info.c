#include "lib/info/info.h"

#include "container.h"
#include "tile/tileUtils.h"
#include <lauxlib.h>

int lib_get_this_container_count(lua_State *L)
{
    int i = get_slave_container_count(selected_monitor) + 1;
    lua_pushinteger(L, i);
    return 1;
}

int lib_this_container_position(lua_State *L)
{
    struct container *con, *sel = selected_container(selected_monitor);
    int n = 1;
    bool handled = false;

    wl_list_for_each(con, &containers, mlink) {
        if (!visibleon(con, selected_monitor->ws[0]) || con->floating)
            continue;
        if (con == sel) {
            handled = true;
            break;
        }
        n++;
    }
    if (!handled)
        n = 1;
    lua_pushinteger(L, n);
    return 1;
}

int lib_get_next_empty_workspace(lua_State *L)
{
    int id = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct workspace *ws = get_next_empty_workspace(id);
    lua_pushinteger(L, ws->id);
    return 1;
}

int lib_get_workspace(lua_State *L)
{
    return 0;
}
