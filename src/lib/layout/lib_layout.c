#include "lib/layout/lib_layout.h"

#include "monitor.h"
#include "server.h"
#include "utils/coreUtils.h"

// TODO refactor
int lib_set_layout(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    int layout_index = server.layout_set.lua_layout_index;
    server.layout_set.lua_layout_index = layout_index;

    copy_layout_safe(lt, &server.default_layout);

    // 1. argument -- layout_set
    if (lua_islayout_data(L, "layout_data"))
        lua_copy_table_safe(L, &lt->lua_layout_copy_data_ref);
    else
        lua_pop(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);
    lua_copy_table_safe(L, &lt->lua_layout_original_copy_data_ref);
    lua_pop(L, 1);
    return 0;
}
