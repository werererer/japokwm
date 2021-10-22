#include "lib/lib_monitor.h"

#include <lauxlib.h>
#include <lua.h>

#include "monitor.h"
#include "server.h"
#include "translationLayer.h"
#include "server.h"

static const struct luaL_Reg monitor_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg monitor_f[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg monitor_m[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg monitor_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg monitor_getter[] =
{
    {NULL, NULL},
};

void lua_load_monitor()
{
    create_class(monitor_meta,
            monitor_f,
            monitor_m,
            monitor_setter,
            monitor_getter,
            CONFIG_MONITOR);

    luaL_newlib(L, monitor_f);
    lua_setglobal(L, "Monitor");
}

struct monitor check_monitor(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_MONITOR);
    luaL_argcheck(L, ud != NULL, 1, "`monitor' expected");
    return *(struct monitor *)ud;
}

static void create_lua_monitor(lua_State *L, struct monitor *m) {
    struct monitor **user_monitor = lua_newuserdata(L, sizeof(struct monitor *));
    *user_monitor = m;

    luaL_setmetatable(L, CONFIG_MONITOR);
}

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
