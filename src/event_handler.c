#include "event_handler.h"

#include <lua.h>

#include "utils/parseConfigUtils.h"

struct event_handler get_default_event_handler()
{
    struct event_handler event_handler = {
        .update_func_ref = 0,
        .create_container_func_ref = 0,
    };
    return event_handler;
}

void call_update_function(struct event_handler *ev, int n)
{
    if (ev->update_func_ref == 0)
        return;
    lua_rawgeti(L, LUA_REGISTRYINDEX, ev->update_func_ref);
    lua_pushinteger(L, n);
    lua_call_safe(L, 1, 0, 0);
}

void call_create_container_function(struct event_handler *ev, int n)
{
    if (ev->create_container_func_ref == 0)
        return;
    lua_rawgeti(L, LUA_REGISTRYINDEX, ev->create_container_func_ref);
    lua_pushinteger(L, n);
    lua_call_safe(L, 1, 0, 0);
}

void call_on_start_function(struct event_handler *ev)
{
    if (ev->on_start_func_ref == 0)
        return;
    lua_rawgeti(L, LUA_REGISTRYINDEX, ev->on_start_func_ref);
    lua_call_safe(L, 0, 0, 0);
}
