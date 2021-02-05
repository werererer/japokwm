#include "lib/config/localconfig.h"

#include "monitor.h"
#include "utils/gapUtils.h"
#include "utils/coreUtils.h"

int local_set_arrange_by_focus(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws[0];

    // 1. argument
    ws->layout[0].options.arrange_by_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_gaps(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws[0]->layout[0];
    lt->options.outer_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    lt->options.inner_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);

    configure_gaps(&lt->options.inner_gap, &lt->options.outer_gap);
    return 0;
}

int local_set_tile_borderpx(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws[0]->layout[0];
    lt->options.tile_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    reset_tiled_client_borders(lt->options.tile_border_px);
    return 0;
}

int local_set_float_borderpx(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws[0]->layout[0];
    lt->options.tile_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    reset_floating_client_borders(lt->options.tile_border_px);
    return 0;
}

int local_set_sloppy_focus(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws[0]->layout[0];
    lt->options.sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_focus_color(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws[0]->layout[0];
    lua_tocolor(lt->options.focus_color);
    lua_pop(L, 1);
    return 0;
}

int local_set_border_color(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws[0]->layout[0];
    lua_tocolor(lt->options.focus_color);
    lua_pop(L, 1);
    return 0;
}
