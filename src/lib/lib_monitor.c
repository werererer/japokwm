#include "lib/lib_monitor.h"

#include <lauxlib.h>
#include <lua.h>

#include "monitor.h"
#include "server.h"

int lib_set_scale(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    float scale = luaL_checknumber(L, -1);
    scale_monitor(m, scale);
    return 0;
}

int lib_set_transform(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    enum wl_output_transform transform = luaL_checkinteger(L, -1);
    transform_monitor(m, transform);
    return 0;
}
