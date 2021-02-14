#include "lib/config/localconfig.h"

#include "monitor.h"
#include "utils/gapUtils.h"
#include "utils/coreUtils.h"

int local_set_arrange_by_focus(lua_State *L)
{
    struct workspace *ws = get_workspace_on_monitor(selected_monitor);

    // 1. argument
    ws->layout[0].options.arrange_by_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_gaps(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    lt->options.outer_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    lt->options.inner_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);

    configure_gaps(&lt->options.inner_gap, &lt->options.outer_gap);
    return 0;
}

int local_set_tile_borderpx(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);
    lt->options.tile_border_px = luaL_checkinteger(L, -1); lua_pop(L, 1);

    reset_tiled_client_borders(lt->options.tile_border_px);
    return 0;
}

int local_set_float_borderpx(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);
    lt->options.float_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    reset_floating_client_borders(lt->options.float_border_px);
    return 0;
}

int local_set_sloppy_focus(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);
    lt->options.sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_focus_color(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);
    lua_tocolor(lt->options.focus_color);
    lua_pop(L, 1);
    return 0;
}

int local_set_border_color(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);
    lua_tocolor(lt->options.focus_color);
    lua_pop(L, 1);
    return 0;
}

int local_set_layout_constraints(lua_State *L)
{
    printf("local_set_layout_constraints\n");
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    lt->options.layout_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int local_set_master_constraints(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    lt->options.master_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int local_set_update_function(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    lt->options.update_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

int local_set_resize_direction(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    lt->options.resize_dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_master_layout_data(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    if (lua_islayout_data(L, "master_layout_data")) {

        lt->options.master_layout_data_ref = lua_copy_table(L);
    }
    else
        lua_pop(L, 1);
    return 0;
}

int local_set_resize_data(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    if (lua_istable(L, -1))
        lt->options.resize_data_ref = lua_copy_table(L);
    else
        lua_pop(L, 1);
    return 0;
}
