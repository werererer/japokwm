#include "lib/layout/lib_layout.h"

#include "monitor.h"
#include "server.h"

int set_layout(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws;
    struct layout *lt = &ws->layout[0];

    // reset options
    copy_options(&lt->options, &server.options);

    // 3. argument
    lt->lua_box_data_ref = lua_copy_table(L);
    // 2. argument
    lt->lua_layout_master_copy_data_ref = lua_copy_table(L);
    // 1.argument
    lt->lua_layout_copy_data_ref = lua_copy_table(L);

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);
    lt->lua_layout_original_copy_data_ref = lua_copy_table(L);
    return 0;
}

