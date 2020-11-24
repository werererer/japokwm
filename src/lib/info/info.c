#include "lib/info/info.h"

#include "tile/tileUtils.h"

int thisTiledClientCount(lua_State *L)
{
    int i = tiled_client_count(selected_monitor);
    lua_pushinteger(L, i);
    return 1;
}

int thisClientPos(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct client *c;
    int n = 1;
    bool handled = false;

    wl_list_for_each(c, &clients, link) {
        if (visibleon(c, m) && !c->floating) {
            if (c == selected_client()) {
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
