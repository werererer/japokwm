#include "lib/layout/lib_layout.h"

#include "monitor.h"
#include "server.h"
#include "utils/coreUtils.h"

// TODO refactor
int lib_set_layout(lua_State *L)
{
    struct layout *lt = get_layout_in_monitor(selected_monitor);

    int layout_index = server.layout_set.lua_layout_index;
    server.layout_set.lua_layout_index = layout_index;

    copy_layout_safe(lt, &server.default_layout);

    struct workspace *ws = get_workspace(lt->ws_id);

    int i = wlr_list_find(&ws->loaded_layouts, cmp_ptr, &lt);
    if (i != -1) {
        struct layout *old_lt = ws->loaded_layouts.items[i];
        lt->lua_layout_copy_data_ref = old_lt->lua_layout_copy_data_ref;
        lua_pop(L, 1);
    } else {
        printf("not found\n");
        wlr_list_insert(&ws->loaded_layouts, 0, lt);

        // 1. argument -- layout_set
        if (lua_islayout_data(L, "layout_data"))
            lua_copy_table_safe(L, &lt->lua_layout_copy_data_ref);
        else
            lua_pop(L, 1);
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);
    lua_copy_table_safe(L, &lt->lua_layout_original_copy_data_ref);
    lua_pop(L, 1);
    return 0;
}
