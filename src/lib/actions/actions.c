#include "lib/actions/actions.h"

#include <inttypes.h>
#include <lua.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "container.h"
#include "ipc-server.h"
#include "layout_set.h"
#include "monitor.h"
#include "popup.h"
#include "root.h"
#include "server.h"
#include "stringop.h"
#include "tile/tileUtils.h"
#include "translationLayer.h"
#include "utils/stringUtils.h"
#include "utils/parseConfigUtils.h"
#include "workspace.h"
#include "xdg-shell-protocol.h"

int lib_arrange(lua_State *L)
{
    arrange();
    return 0;
}

int lib_focus_container(lua_State *L)
{
    int pos = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = selected_monitor;
    struct container *con = container_position_to_container(m->ws_ids[0], pos);

    if (!con)
        return 0;

    focus_container(con, FOCUS_NOOP);
    return 0;
}

int lib_toggle_bars(lua_State *L)
{
    struct monitor *m = selected_monitor;
    toggle_bars_visible(m);
    arrange();
    return 0;
}

int lib_resize_main(lua_State *L)
{
    float n = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace(&server.workspaces, m->ws_ids[0]);
    struct layout *lt = &ws->layout[0];
    int dir = lt->options.resize_dir;

    lua_getglobal_safe(L, "Resize_main_all");
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_original_copy_data_ref);
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->resize_data_ref);
    lua_pushnumber(L, n);
    lua_pushinteger(L, dir);

    lua_call_safe(L, 5, 1, 0);

    lua_copy_table_safe(L, &lt->lua_layout_copy_data_ref);
    arrange();
    return 0;
}

int lib_set_floating(lua_State *L)
{
    printf("lib set floating\n");
    bool floating = lua_toboolean(L, -1);
    lua_pop(L, 1);
    struct container *sel = focused_container(selected_monitor);
    if (!sel)
        return 0;
    set_container_floating(sel, floating);
    arrange();
    return 0;
}

int lib_set_nmaster(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);
    lt->nmaster = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    arrange();
    return 0;
}

int lib_increase_nmaster(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->master_layout_data_ref);
    int max_nmaster = luaL_len(L, -1);
    lua_pop(L, 1);

    printf("max nmaster: %i\n", max_nmaster);
    printf("max nmaster: %i\n", max_nmaster);
    lt->nmaster = MIN(max_nmaster, lt->nmaster + 1);
    arrange();
    return 0;
}

int lib_decrease_nmaster(lua_State *L)
{
    struct layout *lt = get_layout_on_monitor(selected_monitor);

    lt->nmaster = MAX(1, lt->nmaster - 1);
    arrange();
    return 0;
}

int lib_exec(lua_State *L)
{
    const char *cmd = luaL_checkstring(L, -1);
    exec(cmd);
    return 0;
}

int lib_focus_on_stack(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    struct container *sel = focused_container(m);
    if (!sel)
        return 0;

    if (sel->client->type == LAYER_SHELL) {
        focus_top_container(m, FOCUS_NOOP);
        return 0;
    }

    struct container *con = get_relative_container(m, sel, i);
    if (!con)
        return 0;

    /* If only one client is visible on selMon, then c == sel */
    focus_container(con, FOCUS_LIFT);
    return 0;
}

int lib_focus_on_hidden_stack(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    struct container *sel = focused_container(m);

    if (!sel)
        return 0;
    if (sel->client->type == LAYER_SHELL)
        return 0;

    struct container *con = get_relative_hidden_container(m, i);

    if (!con) {
        printf("contaier not found\n");
        return 0;
    }
    if (sel == con) {
        printf("selected equals con\n");
        return 0;
    }

    wl_list_remove(&con->mlink);
    wl_list_insert(&sel->mlink, &con->mlink);
    con->hidden = false;
    if (i >= 0) {
        // replace selected container with a hidden one and move the selected
        // container to the end of containers
        wl_list_remove(&sel->mlink);
        wl_list_insert(containers.prev, &sel->mlink);
        sel->hidden = true;
    } else if (i < 0) {
        // replace current container with a hidden one and move the selected
        // container to the first position that is not visible
        wl_list_remove(&sel->mlink);
        update_container_positions(m);
        struct container *last_container = container_position_to_container(m->ws_ids[0], 0);
        struct container *container_hidden = get_relative_container(m, last_container, -1);
        wl_list_insert(&container_hidden->mlink, &sel->mlink);
        sel->hidden = true;
    }

    focus_container(con, FOCUS_NOOP);
    arrange();
    return 0;
}

int lib_move_resize(lua_State *L)
{
    int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    move_resize(ui);
    return 0;
}

int lib_view(lua_State *L)
{
    unsigned int ws_id = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    if (!m)
        return 0;

    struct workspace *ws = get_workspace(&server.workspaces, ws_id);
    if (!ws)
        return 0;

    push_workspace(m, &server.workspaces, ws->id);
    return 0;
}

int lib_toggle_view(lua_State *L)
{
    struct monitor *m = selected_monitor;
    focus_top_container(m, FOCUS_LIFT);
    arrange(false);
    return 0;
}

int lib_toggle_floating(lua_State *L)
{
    struct container *sel = focused_container(selected_monitor);
    if (!sel)
        return 0;
    set_container_floating(sel, !sel->floating);
    arrange();
    return 0;
}

// TODO optimize
int lib_move_container_to_workspace(lua_State *L)
{
    unsigned int ws_id = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);
    struct container *con = focused_container(m);

    if (!con || con->client->type == LAYER_SHELL)
        return 0;

    set_container_workspace(con, ws->id);
    focus_top_container(m, FOCUS_NOOP);
    arrange();

    ipc_event_workspace();

    container_damage_whole(con);

    return 0;
}

int lib_quit(lua_State *L)
{
    wl_display_terminate(server.display);
    close_error_file();
    return 0;
}

int lib_zoom(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct container *sel = focused_container(m);

    if (sel->position != 0)
        repush(sel->position, 0);
    else
        repush(1, 0);

    arrange();

    // focus new master window
    struct container *con0 = container_position_to_container(m->ws_ids[0], 0);
    focus_container(con0, FOCUS_NOOP);

    struct layout *lt = get_layout_on_monitor(m);
    if (lt->options.arrange_by_focus) {
        focus_top_container(m, FOCUS_NOOP);
        arrange();
    }
    return 0;
}

int lib_repush(lua_State *L)
{
    int abs_index = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    repush(i, abs_index);
    return 0;
}

int lib_load_next_layout_in_set(lua_State *L)
{
    const char *layout_set_key = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    struct workspace *ws = get_workspace_on_monitor(selected_monitor);

    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    if (!lua_is_index_defined(L, layout_set_key)) {
        printf("is nil return\n");
        lua_pop(L, 1);
        return 0;
    }
    lua_pop(L, 1);
    server.layout_set.key = layout_set_key;

    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    lua_get_layout_set_element(L, layout_set_key);
    int n_layouts = luaL_len(L, -1);
    lua_pop(L, 2);

    server.layout_set.lua_layout_index++;
    if (server.layout_set.lua_layout_index > n_layouts) {
        server.layout_set.lua_layout_index = 1;
    }

    set_layout(L, ws);

    arrange();
    return 0;
}

int lib_load_prev_layout_in_set(lua_State *L)
{
    const char *layout_set_key = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace_on_monitor(m);

    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    if (!lua_is_index_defined(L, layout_set_key)) {
        printf("is nil return\n");
        lua_pop(L, 1);
        return 0;
    }
    lua_pop(L, 1);
    server.layout_set.key = layout_set_key;

    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    lua_get_layout_set_element(L, layout_set_key);
    int n_layouts = luaL_len(L, -1);
    lua_pop(L, 2);

    server.layout_set.lua_layout_index--;
    if (server.layout_set.lua_layout_index <= 0) {
        server.layout_set.lua_layout_index = n_layouts;
    }

    set_layout(L, ws);

    arrange();
    return 0;
}

int lib_load_layout_in_set(lua_State *L)
{
    struct workspace *ws = get_workspace_on_monitor(selected_monitor);

    server.layout_set.lua_layout_index = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    const char *layout_set_key = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    // if nil return
    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    if (!lua_is_index_defined(L, layout_set_key)) {
        lua_pop(L, 1);
        return 0;
    }
    lua_pop(L, 1);

    server.layout_set.key = layout_set_key;
    set_layout(L, ws);

    arrange();
    return 0;
}

int lib_load_layout(lua_State *L)
{
    struct workspace *ws = get_workspace_on_monitor(selected_monitor);

    lua_rawgeti(L, -1, 1);
    const char *layout_symbol = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 2);
    const char *layout_name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    load_layout(L, ws, layout_name, layout_symbol);

    arrange();
    return 0;
}

int lib_kill(lua_State *L)
{
    struct monitor *m = selected_monitor;

    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct container *con = get_container(m, i);
    struct client *sel = con->client;

    if (!sel)
        return 0;

    switch (sel->type) {
        case XDG_SHELL:
            wlr_xdg_toplevel_send_close(sel->surface.xdg);
            break;
        case LAYER_SHELL:
            wlr_layer_surface_v1_close(sel->surface.layer);
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            wlr_xwayland_surface_close(sel->surface.xwayland);
            break;
    }
    return 0;
}

int lib_toggle_layout(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace_on_monitor(m);
    push_layout(ws->layout, ws->layout[1]);
    arrange();
    return 0;
}

int lib_toggle_workspace(lua_State *L)
{
    struct monitor *m = selected_monitor;
    push_workspace(m, &server.workspaces, m->ws_ids[1]);
    return 0;
}

int lib_swap_workspace(lua_State *L)
{
    int ws_id1 = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    int ws_id2 = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct container *con;
    wl_list_for_each(con, &containers, mlink) {
        if (existon(con, &server.workspaces, ws_id1)) {
            con->client->ws_id = ws_id2;
            continue;
        }
        if (existon(con, &server.workspaces, ws_id2)) {
            con->client->ws_id = ws_id1;
            continue;
        }
    }

    struct monitor *m = selected_monitor;
    arrange();
    focus_top_container(m, FOCUS_NOOP);
    root_damage_whole(m->root);
    return 0;
}
