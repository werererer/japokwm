#include "event_handler.h"

#include <lua.h>

#include "utils/parseConfigUtils.h"

struct event_handler *create_event_handler()
{
    struct event_handler *event_handler = calloc(1, sizeof(struct event_handler));
    event_handler->on_start_func_refs = g_ptr_array_new();
    event_handler->on_focus_func_refs = g_ptr_array_new();
    event_handler->on_update_func_refs = g_ptr_array_new();
    event_handler->on_create_container_func_refs = g_ptr_array_new();
    return event_handler;
}

void destroy_event_handler(struct event_handler *event_handler)
{
    g_ptr_array_free(event_handler->on_start_func_refs, TRUE);
    g_ptr_array_free(event_handler->on_focus_func_refs, TRUE);
    g_ptr_array_free(event_handler->on_update_func_refs, TRUE);
    g_ptr_array_free(event_handler->on_create_container_func_refs, TRUE);
}

GPtrArray *event_name_to_signal(struct event_handler *event_handler,
        const char *event)
{
    if (strcmp(event, "on_start") == 0)
        return event_handler->on_start_func_refs;
    if (strcmp(event, "on_focus") == 0)
        return event_handler->on_focus_func_refs;
    if (strcmp(event, "on_update") == 0)
        return event_handler->on_update_func_refs;
    if (strcmp(event, "on_create_container") == 0)
        return event_handler->on_create_container_func_refs;
    return NULL;
}

static void emit_signal(GPtrArray *func_refs)
{
    for (int i = 0; i < func_refs->len; i++) {
        int *ref = g_ptr_array_index(func_refs, i);
        lua_rawgeti(L, LUA_REGISTRYINDEX, *ref);
        lua_call_safe(L, 0, 0, 0);
    }
}

static void emit_signal_to_one_int_argument_functions(GPtrArray *func_refs, int n)
{
    for (int i = 0; i < func_refs->len; i++) {
        int *ref = g_ptr_array_index(func_refs, i);
        lua_rawgeti(L, LUA_REGISTRYINDEX, *ref);
        lua_pushinteger(L, n);
        lua_call_safe(L, 1, 0, 0);
    }
}

void call_update_function(struct event_handler *ev, int n)
{
    emit_signal_to_one_int_argument_functions(ev->on_update_func_refs, n);
}

void call_create_container_function(struct event_handler *ev, int n)
{
    emit_signal_to_one_int_argument_functions(ev->on_create_container_func_refs, n);
}

void call_on_focus_function(struct event_handler *ev, int n)
{
    emit_signal_to_one_int_argument_functions(ev->on_focus_func_refs, n);
}

void call_on_start_function(struct event_handler *ev)
{
    emit_signal(ev->on_start_func_refs);
}
