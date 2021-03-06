#include "lib/info/info.h"

#include <lauxlib.h>

#include "container.h"
#include "server.h"
#include "tile/tileUtils.h"

int lib_get_this_container_count(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = monitor_get_active_workspace(m);

    int i = get_slave_container_count(ws) + 1;
    lua_pushinteger(L, i);
    return 1;
}

int lib_this_container_position(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct container *sel = get_focused_container(m);

    int position = get_position_in_container_stack(sel);
    lua_pushinteger(L, position);
    return 1;
}

int lib_get_next_empty_workspace(lua_State *L)
{
    enum wlr_direction dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int id = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    printf("lib_get_next_empty_workspace: %i\n", id);

    struct workspace *ws;
    switch (dir) {
        case WLR_DIRECTION_LEFT:
            ws = get_prev_empty_workspace(&server.workspaces, id);
            break;
        case WLR_DIRECTION_RIGHT:
            ws = get_next_empty_workspace(&server.workspaces, id);
            break;
        default:
            ws = get_workspace(id);
    }

    int ws_id = (ws) ? ws->id : id;
    lua_pushinteger(L, ws_id);
    return 1;
}

int lib_get_nmaster(lua_State *L)
{
    struct layout *lt = get_layout_in_monitor(selected_monitor);
    lua_pushinteger(L, lt->nmaster);
    return 1;
}

int lib_get_workspace(lua_State *L)
{
    struct monitor *m = selected_monitor;
    lua_pushinteger(L, m->ws_id);
    return 1;
}

int lib_get_container_under_cursor(lua_State *L)
{
    struct wlr_cursor *cursor = server.cursor.wlr_cursor;

    struct container *con = xy_to_container(cursor->x, cursor->y);
    int pos = get_position_in_container_stack(con);
    lua_pushinteger(L, pos);
    return 1;
}

int lib_is_container_not_in_limit(lua_State *L)
{
    struct layout *lt = get_layout_in_monitor(selected_monitor);

    struct wlr_fbox geom = lua_togeometry(L);
    lua_pop(L, 1);

    bool not_in_limit = is_resize_not_in_limit(&geom, &lt->options.layout_constraints);
    return not_in_limit;
}

int lib_is_container_not_in_master_limit(lua_State *L)
{
    struct layout *lt = get_layout_in_monitor(selected_monitor);

    struct wlr_fbox geom = lua_togeometry(L);
    lua_pop(L, 1);

    bool not_in_limit = is_resize_not_in_limit(&geom, &lt->options.master_constraints);
    return not_in_limit;
}
