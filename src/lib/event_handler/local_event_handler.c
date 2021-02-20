#include "lib/event_handler/local_event_handler.h"

#include "utils/coreUtils.h"
#include "server.h"

int local_set_update_function(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);
    struct event_handler *ev = &lt->options.event_handler;

    lua_ref_safe(L, LUA_REGISTRYINDEX, &ev->update_func_ref);
    return 0;
}

int local_set_create_container_function(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);
    struct event_handler *ev = &lt->options.event_handler;

    lua_ref_safe(L, LUA_REGISTRYINDEX, &ev->create_container_func_ref);
    return 0;
}
