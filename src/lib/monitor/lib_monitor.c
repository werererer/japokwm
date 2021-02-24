#include "lib/monitor/lib_monitor.h"

#include "monitor.h"

int lib_set_scale(lua_State *L)
{
    struct monitor *m = selected_monitor;
    float scale = luaL_checknumber(L, -1);
    scale_monitor(m, scale);
    return 0;
}

int lib_set_transform(lua_State *L)
{
    struct monitor *m = selected_monitor;
    enum wl_output_transform transform = luaL_checkinteger(L, -1);
    transform_monitor(m, transform);
    return 0;
}
