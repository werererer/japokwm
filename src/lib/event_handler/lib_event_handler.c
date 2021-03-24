#include "lib/event_handler/lib_event_handler.h"

#include "utils/coreUtils.h"
#include "server.h"

int lib_set_update_function(lua_State *L)
{
    lua_ref_safe(L, LUA_REGISTRYINDEX,
            &server.default_layout.options.event_handler.update_func_ref);
    return 0;
}

int lib_set_create_container_function(lua_State *L)
{
    lua_ref_safe(L, LUA_REGISTRYINDEX,
            &server.default_layout.options.event_handler.create_container_func_ref);
    return 0;
}

int lib_set_on_start_function(lua_State *L)
{
    lua_ref_safe(L, LUA_REGISTRYINDEX,
            &server.default_layout.options.event_handler.on_start_func_ref);
    return 0;
}
