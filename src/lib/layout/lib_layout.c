#include "lib/layout/lib_layout.h"

#include "monitor.h"
#include "server.h"

int lib_set_layout(lua_State *L)
{
    printf("set layout\n");
    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws[0];
    struct layout *lt = &ws->layout[0];

    int layout_index = server.layout_set.lua_layout_index;
    *lt = server.default_layout;
    server.layout_set.lua_layout_index = layout_index;

    // reset options
    copy_options(&lt->options, &server.default_layout.options);

    // 1. argument -- layout_set
    if (lua_islayout_data(L, "layout_data"))
        lt->lua_layout_copy_data_ref = lua_copy_table(L);
    else
        lua_pop(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);
    lt->lua_layout_original_copy_data_ref = lua_copy_table(L);
    return 0;
}
