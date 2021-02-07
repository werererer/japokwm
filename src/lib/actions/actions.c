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
#include "monitor.h"
#include "parseConfig.h"
#include "popup.h"
#include "root.h"
#include "server.h"
#include "stringop.h"
#include "tile/tileUtils.h"
#include "translationLayer.h"
#include "utils/stringUtils.h"
#include "workspace.h"
#include "xdg-shell-protocol.h"

static void pointer_focus(struct container *con, struct wlr_surface *surface,
        double sx, double sy, uint32_t time);

static void pointer_focus(struct container *con, struct wlr_surface *surface,
        double sx, double sy, uint32_t time)
{
    /* Use top level surface if nothing more specific given */
    if (con && !surface)
        surface = get_wlrsurface(con->client);

    /* If surface is NULL, clear pointer focus */
    if (!surface) {
        wlr_seat_pointer_notify_clear_focus(server.seat);
        return;
    }

    /* If surface is already focused, only notify motion */
    if (surface == server.seat->pointer_state.focused_surface) {
        wlr_seat_pointer_notify_motion(server.seat, time, sx, sy);
        return;
    }
    /* Otherwise, let the client know that the mouse cursor has entered one
     * of its surfaces, and make keyboard focus follow if desired. */
    wlr_seat_pointer_notify_enter(server.seat, surface, sx, sy);

    if (con->client->type == X11_UNMANAGED)
        return;

    if (con->m->ws[0]->layout[0].options.sloppy_focus)
        focus_container(con, FOCUS_NOOP);
}

int lib_arrange(lua_State *L)
{
    arrange();
    return 0;
}

int lib_focus_container(lua_State *L)
{
    int pos = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    printf("focus container: %i\n", pos);
    struct container *con = container_position_to_container(pos);
    printf("container: %p\n", con);

    if (!con)
        return 0;

    focus_container(con, FOCUS_NOOP);
    return 0;
}

int lib_toggle_consider_layer_shell(lua_State *L)
{
    selected_monitor->root->consider_layer_shell = !selected_monitor->root->consider_layer_shell;
    arrange();
    return 0;
}

int lib_resize_main(lua_State *L)
{
    float n = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws[0];
    struct layout *lt = &ws->layout[0];
    int dir = lt->options.resize_dir;

    lua_getglobal_safe(L, "Resize_main_all");
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_original_copy_data_ref);
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_resize_data_ref);
    lua_pushnumber(L, n);
    lua_pushinteger(L, dir);

    lua_call_safe(L, 5, 1, 0);

    lt->lua_layout_copy_data_ref = lua_copy_table(L);
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
    selected_monitor->ws[0]->layout[0].nmaster = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    arrange();
    return 0;
}

int lib_increase_nmaster(lua_State *L)
{
    struct layout *lt = &selected_monitor->ws[0]->layout[0];
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_master_copy_data_ref);
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
    struct layout *lt = &selected_monitor->ws[0]->layout[0];

    lt->nmaster = MAX(1, lt->nmaster - 1);
    arrange();
    return 0;
}

int lib_spawn(lua_State *L)
{
    const char *cmd = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    if (fork() == 0) {
        setsid();
        execl("/bin/sh", "/bin/sh", "-c", cmd, (void *)NULL);
    }
    return 0;
}

int lib_focus_on_stack(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws[0];
    struct container *sel = focused_container(m);
    if (!sel)
        return 0;

    bool found = false;
    struct container *con;
    if (i > 0) {
        wl_list_for_each(con, &sel->mlink, mlink) {
            if (con == sel)
                continue;
            if (visibleon(con, ws)) {
                found = true;
                break;
            }
        }
    } else {
        wl_list_for_each_reverse(con, &sel->mlink, mlink) {
            if (con == sel)
                continue;
            if (visibleon(con, ws)) {
                found = true;
                break;
            }
        }
    }

    if (found) {
        /* If only one client is visible on selMon, then c == sel */
        focus_container(con, FOCUS_LIFT);
    }
    return 0;
}

int lib_get_nmaster(lua_State *L)
{
    lua_pushinteger(L, selected_monitor->ws[0]->layout[0].nmaster);
    return 1;
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

    struct container *con;
    if (i > 0) {
        int j = 1;
        wl_list_for_each(con, &sel->mlink, mlink) {
            if (hiddenon(con, m->ws[0]))
                break;  /* found it */
            j++;
        }
    } else {
        wl_list_for_each_reverse(con, &sel->mlink, mlink) {
            if (hiddenon(con, m->ws[0]))
                break;  /* found it */
        }
    }

    if (sel == con)
        return 0;

    if (sel && con) {
        // replace current client with a hidden one
        wl_list_remove(&con->mlink);
        wl_list_insert(&sel->mlink, &con->mlink);
        wl_list_remove(&sel->mlink);
        wl_list_insert(containers.prev, &sel->mlink);
    }

    focus_container(con, FOCUS_LIFT);
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
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    if (!m)
        return 0;

    struct workspace *ws = get_workspace(ui);
    if (!ws)
        return 0;

    if (is_workspace_occupied(ws)) {
        set_selected_monitor(ws->m);
        return 0;
    }
    set_next_unoccupied_workspace(m, ws);
    focus_top_container(m->ws[0], FOCUS_NOOP);
    arrange();
    return 0;
}

int lib_toggle_view(lua_State *L)
{
    struct monitor *m = selected_monitor;
    focus_top_container(m->ws[0], FOCUS_LIFT);
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
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace(ui);
    struct container *con = focused_container(m);

    if (!con || con->client->type == LAYER_SHELL)
        return 0;

    con->client->ws = ws;
    arrange();
    focus_top_container(m->ws[0], FOCUS_NOOP);

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

    if (!sel)
        return 0;
    if (sel->floating)
        return 0;
    if (sel->client->type == LAYER_SHELL) {
        focus_top_container(m->ws[0], FOCUS_NOOP);
        sel = wl_container_of(&containers.next, sel, mlink);
    }
    if (!sel)
        return 0;

    struct container *master = wl_container_of(containers.next, master, mlink);
    struct container *previous;
    if (sel == master)
        previous = wl_container_of(containers.next->next, previous, mlink);
    else
        previous = wl_container_of(sel->mlink.prev, previous, mlink);

    bool found = false;
    struct container *con;
    // loop from selected monitor to previous item
    wl_list_for_each(con, sel->mlink.prev, mlink) {
        if (!visibleon(con, m->ws[0]) || con->floating)
            continue;
        if (con == master)
            continue;

        found = true;
        break; /* found */
    }
    if (!found)
        return 0;

    wl_list_remove(&con->mlink);
    wl_list_insert(&containers, &con->mlink);

    arrange();
    // focus new master window
    focus_container(previous, FOCUS_NOOP);

    if (selected_monitor->ws[0]->layout[0].options.arrange_by_focus) {
        focus_top_container(m->ws[0], FOCUS_NOOP);
        arrange();
    }
    return 0;
}

int lib_repush(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int abs_index = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;

    struct container *sel = get_container(selected_monitor, i);
    /* struct container *sel = selected_container(m); */

    if (!sel || sel->floating)
        return 0;

    struct container *master = wl_container_of(containers.next, master, mlink);
    struct container *previous;
    if (sel == master)
        previous = wl_container_of(containers.next->next, previous, mlink);
    else
        previous = wl_container_of(sel->mlink.prev, previous, mlink);

    bool found = false;
    struct container *con;
    // loop from selected monitor to previous item
    wl_list_for_each(con, sel->mlink.prev, mlink) {
        if (!visibleon(con, m->ws[0]) || con->floating)
            continue;
        if (con == master)
            continue;

        found = true;
        break; /* found */
    }
    if (!found)
        return 0;

    wl_list_remove(&con->mlink);

    // find container to put current container after
    struct container *con2 = get_container(selected_monitor, abs_index);
    wl_list_insert(&con2->mlink, &con->mlink);

    arrange();
    // focus new master window
    focus_container(previous, FOCUS_NOOP);

    if (selected_monitor->ws[0]->layout[0].options.arrange_by_focus) {
        arrange();
    }
    return 0;
}

int lib_load_default_layout(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws[0];
    struct layout *lt = &ws->layout[0];

    int argc = lua_gettop(L);
    if (argc > 0) {
        lt->lua_layout_index = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    set_layout(L, ws, lt->options.layouts_ref);

    arrange();
    return 0;
}

int lib_load_layout(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws[0];

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
    struct client *sel = focused_container(selected_monitor)->client;
    if (sel) {
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
        }
    }
    return 0;
}

int lib_toggle_layout(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws[0];
    push_layout(ws->layout, ws->layout[1]);
    arrange();
    return 0;
}

int lib_toggle_workspace(lua_State *L)
{
    struct monitor *m = selected_monitor;
    push_workspace(m->ws, m->ws[1]);
    arrange();
    return 0;
}
