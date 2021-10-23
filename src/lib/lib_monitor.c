#include "lib/lib_monitor.h"

#include <lauxlib.h>
#include <lua.h>

#include "monitor.h"
#include "server.h"
#include "translationLayer.h"
#include "server.h"
#include "lib/lib_workspace.h"

static const struct luaL_Reg monitor_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg monitor_f[] =
{
    {"get_focused", lib_monitor_get_focused},
    {NULL, NULL},
};

static const struct luaL_Reg monitor_m[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg monitor_setter[] =
{
    {"scale", lib_monitor_set_scale},
    {"transform", lib_monitor_set_transform},
    {NULL, NULL},
};

static const struct luaL_Reg monitor_getter[] =
{
    {"workspace", lib_monitor_get_workspace},
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

struct monitor *check_monitor(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_MONITOR);
    luaL_argcheck(L, ud != NULL, 1, "`monitor' expected");
    return *(struct monitor **)ud;
}

static void create_lua_monitor(lua_State *L, struct monitor *m) {
    struct monitor **user_monitor = lua_newuserdata(L, sizeof(struct monitor *));
    *user_monitor = m;

    luaL_setmetatable(L, CONFIG_MONITOR);
}

// functions
int lib_monitor_get_focused(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    create_lua_monitor(L, m);
    return 1;
}
// methods
// getter
int lib_monitor_get_workspace(lua_State *L)
{
    struct monitor *m = check_monitor(L, 1);
    lua_pop(L, 1);

    struct workspace *ws = monitor_get_active_workspace(m);
    create_lua_workspace(L, ws);
    return 1;
}

// setter
int lib_monitor_set_scale(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    float scale = luaL_checknumber(L, -1);
    scale_monitor(m, scale);
    return 0;
}

int lib_monitor_set_transform(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    enum wl_output_transform transform = luaL_checkinteger(L, -1);
    transform_monitor(m, transform);
    return 0;
}
