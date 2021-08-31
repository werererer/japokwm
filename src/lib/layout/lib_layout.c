#include "lib/layout/lib_layout.h"

#include "layout.h"
#include "monitor.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "workspace.h"

// TODO refactor
int lib_set_layout(lua_State *L)
{
    int ref = 0;
    // 2. argument -- layout_set
    if (lua_is_layout_data(L, "layout_data")) {
        lua_ref_safe(L, LUA_REGISTRYINDEX, &ref);
    } else {
        lua_pop(L, 1);
    }
    // 1. argument
    const char *symbol = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    struct layout *lt = create_layout(L);

    int layout_index = server.layout_set.lua_layout_index;
    server.layout_set.lua_layout_index = layout_index;

    copy_layout_safe(lt, server.default_layout);

    lt->symbol = symbol;

    struct workspace *ws = monitor_get_active_workspace(selected_monitor);

    guint i;
    bool found = g_ptr_array_find_with_equal_func(ws->loaded_layouts, lt, cmp_layout, &i);
    if (found) {
        struct layout *old_lt = g_ptr_array_index(ws->loaded_layouts, i);
        lt->lua_layout_copy_data_ref = old_lt->lua_layout_copy_data_ref;
    } else {
        g_ptr_array_insert(ws->loaded_layouts, 0, lt);

        if (ref > 0) {
            lt->lua_layout_copy_data_ref = ref;
        }
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);
    lua_copy_table_safe(L, &lt->lua_layout_original_copy_data_ref);
    lua_pop(L, 1);

    push_layout(ws, lt);
    return 0;
}
