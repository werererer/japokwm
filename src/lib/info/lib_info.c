#include "lib/info/lib_info.h"

#include <lauxlib.h>

#include "container.h"
#include "monitor.h"
#include "seat.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "workspace.h"
#include "layout.h"
#include "tagset.h"
#include "root.h"

int lib_get_active_layout(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct tagset *tagset = monitor_get_active_tagset(m);

    struct layout *lt = tagset_get_layout(tagset);
    lua_pushstring(L, lt->symbol);
    return 1;
}

int lib_get_this_container_count(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct tagset *tagset = monitor_get_active_tagset(m);

    int i = get_slave_container_count(tagset) + 1;
    lua_pushinteger(L, i);
    return 1;
}

int lib_get_n_tiled(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct tagset *tagset = monitor_get_active_tagset(m);
    struct layout *lt = tagset_get_layout(tagset);
    int i = lt->n_tiled;
    lua_pushinteger(L, i);
    return 1;
}

int lib_this_container_position(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct container *sel = get_focused_container(m);

    int position = get_position_in_container_focus_stack(sel);
    lua_pushinteger(L, position);
    return 1;
}

int lib_stack_position_to_position(lua_State *L)
{
    int pos = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);

    struct container *con = get_container_in_stack(ws, pos);
    int focus_position = get_position_in_container_focus_stack(con);
    lua_pushinteger(L, focus_position);
    return 1;
}

int lib_get_next_empty_workspace(lua_State *L)
{
    enum wlr_direction dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int id = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct workspace *ws = NULL;
    switch (dir) {
        case WLR_DIRECTION_LEFT:
            ws = get_prev_empty_workspace(server.workspaces, id);
            break;
        case WLR_DIRECTION_RIGHT:
            ws = get_next_empty_workspace(server.workspaces, id);
            break;
        default:
            if (dir & WLR_DIRECTION_LEFT && dir & WLR_DIRECTION_RIGHT) {
                ws = get_nearest_empty_workspace(server.workspaces, id);
            } else {
                ws = get_workspace(id);
            }
    }

    int ws_id = (ws) ? ws->id : id;
    lua_pushinteger(L, ws_id);
    return 1;
}

int lib_get_nmaster(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);
    lua_pushinteger(L, lt->nmaster);
    return 1;
}

int lib_get_previous_layout(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);

    struct layout *lt = ws->previous_layout;
    lua_pushstring(L, lt->symbol);
    return 1;
}

int lib_get_workspace(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    lua_pushinteger(L, m->tagset->selected_ws_id);
    return 1;
}

int lib_get_workspace_count(lua_State *L)
{
    lua_pushinteger(L, server.workspaces->len);
    return 1;
}

int lib_get_container_under_cursor(lua_State *L)
{
    struct seat *seat = input_manager_get_default_seat();
    struct wlr_cursor *wlr_cursor = seat->cursor->wlr_cursor;

    struct container *con = xy_to_container(wlr_cursor->x, wlr_cursor->y);
    int pos = get_position_in_container_focus_stack(con);
    lua_pushinteger(L, pos);
    return 1;
}

int lib_get_root_area(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct root *root = m->root;
    lua_createtable(L, 1, 0);
    lua_pushinteger(L, root->geom.x);
    lua_rawseti(L, -2, 1);
    lua_pushinteger(L, root->geom.y);
    lua_rawseti(L, -2, 2);
    lua_pushinteger(L, root->geom.width);
    lua_rawseti(L, -2, 3);
    lua_pushinteger(L, root->geom.height);
    lua_rawseti(L, -2, 4);
    return 1;
}

int lib_is_container_not_in_limit(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    struct wlr_fbox geom = lua_togeometry(L);
    lua_pop(L, 1);

    bool not_in_limit = is_resize_not_in_limit(&geom, &lt->options.layout_constraints);
    return not_in_limit;
}

int lib_is_container_not_in_master_limit(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);

    struct wlr_fbox geom = lua_togeometry(L);
    lua_pop(L, 1);

    bool not_in_limit = is_resize_not_in_limit(&geom, &lt->options.master_constraints);
    return not_in_limit;
}

int cmp_str_bool(const void *s1, const void *s2)
{
    return strcmp(s1, s2) == 0;
}

int lib_is_keycombo(lua_State *L)
{
    const char *key_combo_name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    guint pos = 0;
    bool found = g_ptr_array_find_with_equal_func(
            server.named_key_combos,
            key_combo_name, cmp_str_bool, &pos);
    lua_pushboolean(L, found);
    return 1;
}
