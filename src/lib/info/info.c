#include "lib/info/info.h"

#include <lauxlib.h>

#include "container.h"
#include "server.h"
#include "tile/tileUtils.h"

int lib_get_this_container_count(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace_on_monitor(m);

    int i = get_slave_container_count(ws) + 1;
    lua_pushinteger(L, i);
    return 1;
}

int lib_this_container_position(lua_State *L)
{
    struct container *con, *sel = focused_container(selected_monitor);
    int n = 0;
    bool handled = false;

    wl_list_for_each(con, &containers, mlink) {
        if (!visibleon(con, &server.workspaces, selected_monitor->ws_ids[0]))
            continue;
        if (con->floating)
            continue;
        if (con == sel) {
            handled = true;
            break;
        }
        n++;
    }
    if (!handled)
        n = 0;
    lua_pushinteger(L, n);
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
            ws = get_workspace(&server.workspaces, id);
    }

    int ws_id = (ws) ? ws->id : id;
    lua_pushinteger(L, ws_id);
    return 1;
}

int lib_get_nmaster(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);
    lua_pushinteger(L, lt->nmaster);
    return 1;
}

int lib_get_workspace(lua_State *L)
{
    struct monitor *m = selected_monitor;
    lua_pushinteger(L, m->ws_ids[0]);
    return 1;
}

int lib_get_container_under_cursor(lua_State *L)
{
    struct wlr_cursor *cursor = server.cursor.wlr_cursor;
    struct container *con = xytocontainer(cursor->x, cursor->y);

    int pos = 0;
    if (con)
        pos = con->position;
    lua_pushinteger(L, pos);
    return 1;
}

int lib_is_container_not_in_limit(lua_State *L)
{
    printf("lib_is_container_in_limit\n");
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    struct wlr_fbox geom = lua_togeometry(L);
    lua_pop(L, 1);

    bool not_in_limit = is_resize_not_in_limit(&geom, &lt->options.layout_constraints);
    printf("inlimit: %i\n", !not_in_limit);
    return not_in_limit;
}

int lib_is_container_not_in_master_limit(lua_State *L)
{
    printf("lib_is_container_in_limit\n");
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    struct wlr_fbox geom = lua_togeometry(L);
    lua_pop(L, 1);

    bool not_in_limit = is_resize_not_in_limit(&geom, &lt->options.master_constraints);
    printf("inlimit: %i\n", !not_in_limit);
    return not_in_limit;
}
