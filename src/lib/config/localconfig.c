#include "lib/config/localconfig.h"

#include "monitor.h"
#include "utils/gapUtils.h"
#include "utils/coreUtils.h"

int local_set_arrange_by_focus(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws;

    // 2. argument
    ws->layout.options.arrange_by_focus = lua_toboolean(L, -1);
    return 0;
}

int local_set_gaps(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws->layout;
    lt->options.outer_gap = luaL_checkinteger(L ,-1);
    lt->options.inner_gap = luaL_checkinteger(L ,-1);
    configure_gaps(&lt->options.inner_gap, &lt->options.outer_gap);
    return 0;
}

int local_set_borderpx(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws->layout;
    lt->options.border_px = luaL_checkinteger(L, -1);

    reset_client_borders(lt->options.border_px);
    return 0;
}

int local_set_sloppy_focus(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws->layout;
    lt->options.sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int local_set_focus_color(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws->layout;
    lua_get_color(lt->options.focus_color);
    return 0;
}

int local_set_border_color(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = &m->ws->layout;
    lua_get_color(lt->options.focus_color);
    return 0;
}
