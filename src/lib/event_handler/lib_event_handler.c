#include "lib/event_handler/lib_event_handler.h"

#include <string.h>

#include "utils/coreUtils.h"
#include "server.h"

int lib_add_listener(lua_State *L)
{
    int *func_ref = calloc(1, sizeof(int));
    lua_ref_safe(L, LUA_REGISTRYINDEX, func_ref);
    const char *event = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    struct event_handler *event_handler = server.default_layout->options.event_handler;
    struct wlr_list *signal = event_name_to_signal(event_handler, event);
    wlr_list_push(signal, func_ref);
    return 0;
}
