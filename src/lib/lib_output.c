#include "lib/lib_output.h"

#include <lauxlib.h>
#include <lua.h>

#include "output.h"
#include "server.h"
#include "translationLayer.h"
#include "server.h"
#include "lib/lib_tag.h"
#include "lib/lib_root.h"

static const struct luaL_Reg monitor_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg monitor_static_methods[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg monitor_methods[] =
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
    {"focused", lib_monitor_get_focused},
    {"root", lib_monitor_get_root},
    {"tag", lib_monitor_get_tag},
    {NULL, NULL},
};

void lua_load_monitor(lua_State *L)
{
    create_class(L,
            monitor_meta,
            monitor_static_methods,
            monitor_methods,
            monitor_setter,
            monitor_getter,
            CONFIG_MONITOR);

    luaL_newlib(L, monitor_static_methods);
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

// static methods
int lib_monitor_get_focused(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    create_lua_monitor(L, m);
    return 1;
}
// methods
// getter
int lib_monitor_get_root(lua_State *L)
{
    struct monitor *m = check_monitor(L, 1);
    lua_pop(L, 1);

    create_lua_root(L, m->root);
    return 1;
}

int lib_monitor_get_tag(lua_State *L)
{
    struct monitor *m = check_monitor(L, 1);
    lua_pop(L, 1);

    struct tag *tag = monitor_get_active_tag(m);
    create_lua_tag(L, tag);
    return 1;
}

// setter

int lib_monitor_set_scale(lua_State *L)
{
    float scale = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    struct monitor *m = check_monitor(L, 1);
    lua_pop(L, 1);
    scale_monitor(m, scale);
    return 0;
}

int lib_monitor_set_transform(lua_State *L)
{
    enum wl_output_transform transform = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = check_monitor(L, 1);
    lua_pop(L, 1);
    transform_monitor(m, transform);
    return 0;
}
