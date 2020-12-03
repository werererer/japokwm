#include "lib/info/info.h"

#include "container.h"
#include "tile/tileUtils.h"

int thisTiledClientCount(lua_State *L)
{
    int i = tiled_container_count(selected_monitor);
    lua_pushinteger(L, i);
    return 1;
}

int this_container_position(lua_State *L)
{
    struct container *con;
    int n = 1;
    bool handled = false;

    wl_list_for_each(con, &selected_monitor->stack, slink) {
        if (visibleon(con->client, selected_monitor->tagset) && !con->floating) {
            if (con == selected_container()) {
                handled = true;
                break;
            }
        }
        n++;
    }
    if (!handled)
        n = 1;
    lua_pushinteger(L, n);
    return 1;
}
