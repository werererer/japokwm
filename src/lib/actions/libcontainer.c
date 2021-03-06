#include "lib/actions/libcontainer.h"
#include "container.h"
#include "client.h"
#include "tile/tileUtils.h"
#include "server.h"

int container_set_sticky(lua_State *L)
{
    bool sticky = lua_toboolean(L, -1);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    struct container *con = get_container(get_workspace(m->ws_id), i);
    client_setsticky(con->client, sticky);
    return 0;
}

int container_set_ratio(lua_State *L)
{
    float ratio = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    int position = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace(m->ws_id);
    struct container *con = get_container(ws, position);

    if (!con)
        return 0;

    con->ratio = ratio;
    return 0;
}

int container_set_alpha(lua_State *L)
{
    float alpha = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    int position = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace(m->ws_id);
    struct container *con = get_container(ws, position);

    if (!con)
        return 0;

    con->alpha = alpha;
    return 0;
}
