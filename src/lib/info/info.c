#include "lib/info/info.h"

#include "container.h"
#include "tile/tileUtils.h"

int this_tiled_client_count(lua_State *L)
{
    int i = tiled_container_count(selected_monitor);
    lua_pushinteger(L, i);
    return 1;
}

int this_container_position(lua_State *L)
{
    struct container *con, *sel = selected_container(selected_monitor);
    int n = 1;
    bool handled = false;

    wl_list_for_each(con, &selected_monitor->containers, mlink) {
        if (!visibleon(con, selected_monitor) || con->floating)
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
