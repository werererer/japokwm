#include "lib/config/local_config.h"

#include "client.h"
#include "layout.h"
#include "monitor.h"
#include "utils/gapUtils.h"
#include "utils/coreUtils.h"
#include "workspace.h"
#include "server.h"
#include "tile/tileUtils.h"

int local_set_arrange_by_focus(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);

    // 1. argument
    ws->layout->options.arrange_by_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_inner_gaps(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    lt->options.inner_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);

    configure_gaps(&lt->options.inner_gap, &lt->options.outer_gap);
    return 0;
}

int local_set_outer_gaps(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    lt->options.outer_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);

    configure_gaps(&lt->options.inner_gap, &lt->options.outer_gap);
    return 0;
}

int local_set_tile_borderpx(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);
    lt->options.tile_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    arrange();
    return 0;
}

int local_set_float_borderpx(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);
    lt->options.float_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    reset_floating_client_borders(lt->options.float_border_px);
    return 0;
}

int local_set_sloppy_focus(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);
    lt->options.sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_focus_color(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);
    lua_tocolor(lt->options.focus_color);
    lua_pop(L, 1);
    return 0;
}

int local_set_border_color(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);
    lua_tocolor(lt->options.focus_color);
    lua_pop(L, 1);
    return 0;
}

int local_set_layout_constraints(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    lt->options.layout_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int local_set_hidden_edges(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    lt->options.hidden_edges = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_smart_hidden_edges(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    lt->options.smart_hidden_edges = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_master_constraints(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    lt->options.master_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int local_set_resize_direction(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    lt->options.resize_dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_resize_function(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_resize_function_ref);
    return 0;
}

int local_set_master_layout_data(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    if (lua_is_layout_data(L, "master_layout_data"))
        lua_copy_table_safe(L, &lt->lua_master_layout_data_ref);
    else
        lua_pop(L, 1);
    return 0;
}

int local_set_resize_data(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    if (lua_istable(L, -1)) {
        lua_copy_table_safe(L, &lt->lua_resize_data_ref);
    } else {
        lua_pop(L, 1);
    }
    return 0;
}
