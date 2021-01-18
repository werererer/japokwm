#include "lib/actions/libcontainer.h"
#include "container.h"
#include "client.h"

int container_setsticky(lua_State *L)
{
    bool sticky = lua_toboolean(L, -1);
    lua_pop(L, 1);
    int position = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct container *con = container_position_to_container(position);
    client_setsticky(con->client, sticky);
    return 0;
}
