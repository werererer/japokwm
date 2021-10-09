#include "lib/event_handler/lib_event_handler.h"

#include <string.h>

#include "utils/coreUtils.h"
#include "server.h"
#include "translationLayer.h"

static const struct luaL_Reg event_f[] =
{
    {"add_listener", lib_add_listener},
    {NULL, NULL},
};

static const struct luaL_Reg event_m[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg event_getter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg event_setter[] =
{
    {NULL, NULL},
};

void create_lua_event_handler(struct event_handler *event_handler) {
    if (!event_handler)
        return;
    struct event_handler **user_con = lua_newuserdata(L, sizeof(struct event_handler*));
    *user_con = event_handler;

    luaL_setmetatable(L, CONFIG_EVENT);
}

void lua_load_events()
{
    create_class(event_f, event_m, event_setter, event_getter, CONFIG_EVENT);

    luaL_newlib(L, event_f);
    lua_setglobal(L, "Event");
}

void lua_init_events(struct event_handler *event_handler)
{
    create_lua_event_handler(event_handler);
    lua_setglobal(L, "event");
}

struct event_handler *check_event_handler(lua_State *L, int narg)
{
    void **ud = luaL_checkudata(L, narg, CONFIG_EVENT);
    luaL_argcheck(L, ud != NULL, 1, "`event_handler' expected");
    return (struct event_handler *)*ud;
}

int lib_add_listener(lua_State *L)
{
    int *func_ref = calloc(1, sizeof(*func_ref));
    lua_ref_safe(L, LUA_REGISTRYINDEX, func_ref);
    const char *event = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    struct event_handler *event_handler = check_event_handler(L, 1);
    lua_pop(L, 1);

    GPtrArray *signal = event_name_to_signal(event_handler, event);
    g_ptr_array_add(signal, func_ref);
    return 0;
}
