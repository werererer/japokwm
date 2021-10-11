#include "event_handler.h"

#include <lua.h>

#include "utils/parseConfigUtils.h"
#include "utils/coreUtils.h"
#include "lib/lib_layout.h"
#include "layout.h"
#include "monitor.h"
#include "server.h"
#include "workspace.h"

struct event_handler *create_event_handler()
{
    struct event_handler *event_handler = calloc(1, sizeof(*event_handler));
    event_handler->on_start_func_refs = g_ptr_array_new();
    event_handler->on_focus_func_refs = g_ptr_array_new();
    event_handler->on_update_func_refs = g_ptr_array_new();
    event_handler->on_create_container_func_refs = g_ptr_array_new();
    return event_handler;
}

void destroy_event_handler(struct event_handler *event_handler)
{
    g_ptr_array_unref(event_handler->on_start_func_refs);
    g_ptr_array_unref(event_handler->on_focus_func_refs);
    g_ptr_array_unref(event_handler->on_update_func_refs);
    g_ptr_array_unref(event_handler->on_create_container_func_refs);
}

GPtrArray *event_name_to_signal(struct event_handler *event_handler,
        const char *event)
{
    if (strcmp(event, "on_start") == 0) {
        return event_handler->on_start_func_refs;
    }
    if (strcmp(event, "on_focus")) {
        return event_handler->on_focus_func_refs;
    }
    if (strcmp(event, "on_update")) {
        return event_handler->on_update_func_refs;
    }
    if (strcmp(event, "on_create_container")) {
        return event_handler->on_create_container_func_refs;
    }
    return NULL;
}

// this seems to be the culprit
static void emit_signal(GPtrArray *func_refs, int narg)
{
    // [..., arg1, arg2, ..., argn]
    for (int i = 0; i < func_refs->len; i++) {
        int *ref = g_ptr_array_index(func_refs, i);
        lua_rawgeti(L, LUA_REGISTRYINDEX, *ref);
        // [..., arg1, arg2, ..., argn, func]

        for (int i = 0; i < narg; i++) {
            lua_pushvalue(L, -narg-1);
        }
        // [..., func, arg1, arg2, ..., argn]
        lua_call_safe(L, narg, 0, 0);
    }
    lua_pop(L, narg);
}

void call_update_function(struct event_handler *ev, int n)
{
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = workspace_get_layout(ws);

    create_lua_layout(lt);
    emit_signal(ev->on_update_func_refs, 1);
}

void call_create_container_function(struct event_handler *ev, int n)
{
    lua_pushinteger(L, n);
    emit_signal(ev->on_create_container_func_refs, 1);
}

void call_on_focus_function(struct event_handler *ev, int n)
{
    lua_pushinteger(L, n);
    emit_signal(ev->on_focus_func_refs, 1);
}

void call_on_start_function(struct event_handler *ev)
{
    emit_signal(ev->on_start_func_refs, 0);
}
