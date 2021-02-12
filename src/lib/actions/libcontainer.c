#include "lib/actions/libcontainer.h"
#include "container.h"
#include "client.h"
#include "tile/tileUtils.h"

int container_set_sticky(lua_State *L)
{
    bool sticky = lua_toboolean(L, -1);
    lua_pop(L, 1);
    int position = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct container *con = container_position_to_container(position);
    client_setsticky(con->client, sticky);
    return 0;
}

int container_set_ratio(lua_State *L)
{
    float ratio = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    int position = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct container *con = container_position_to_container(position);

    if (!con)
        return 0;

    con->ratio = ratio;
    return 0;
}
