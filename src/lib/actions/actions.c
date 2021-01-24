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

static struct container *grabc = NULL;
static int grabcx, grabcy; /* client-relative */

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
        focus_container(con, selected_monitor, FOCUS_NOOP);
}

int set_resize_direction(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws[0];
    ws->layout[0].resize_dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_arrange(lua_State *L)
{
    arrange();
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
    int d = lt->resize_dir;

    lua_getglobal_safe(L, "Resize_main_all");
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_original_copy_data_ref);
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_box_data_ref);
    lua_pushnumber(L, n);
    lua_pushinteger(L, d);

    lua_call_safe(L, 5, 1, 0);

    lt->lua_layout_copy_data_ref = lua_copy_table(L);
    arrange();
    return 0;
}

int lib_set_floating(lua_State *L)
{
    bool b = lua_toboolean(L, -1);
    lua_pop(L, 1);
    struct container *sel = selected_container(selected_monitor);
    if (!sel)
        return 0;
    set_container_floating(sel, b);
    arrange();
    return 0;
}

int set_nmaster(lua_State *L)
{
    selected_monitor->ws[0]->layout[0].nmaster = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
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
    struct container *sel = selected_container(m);
    if (!sel)
        return 0;

    bool found = false;
    struct container *con;
   if (i > 0) {
        wl_list_for_each(con, &sel->mlink, mlink) {
            if (con == sel)
                continue;
            if (visibleon(con, m)) {
                found = true;
                break;
            }
        }
    } else {
        wl_list_for_each_reverse(con, &sel->mlink, mlink) {
            if (con == sel)
                continue;
            if (visibleon(con, m)) {
                found = true;
                break;
            }
        }
    }

    if (found) {
        /* If only one client is visible on selMon, then c == sel */
        focus_container(con, m, FOCUS_LIFT);
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
    struct container *sel = selected_container(m);

    if (!sel)
        return 0;

    struct container *con;
    if (i > 0) {
        int j = 1;
        wl_list_for_each(con, &sel->mlink, mlink) {
            if (hiddenon(con, m))
                break;  /* found it */
            j++;
        }
    } else {
        wl_list_for_each_reverse(con, &sel->mlink, mlink) {
            if (hiddenon(con, m))
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

    focus_container(con, m, FOCUS_LIFT);
    arrange();
    return 0;
}

int lib_move_resize(lua_State *L)
{
    int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    grabc = xytocontainer(server.cursor->x, server.cursor->y);
    if (!grabc)
        return 0;

    /* Float the window and tell motionnotify to grab it */
    set_container_floating(grabc, true);
    switch (server.cursorMode = ui) {
        case CURSOR_MOVE:
            grabcx = server.cursor->x - grabc->geom.x;
            grabcy = server.cursor->y - grabc->geom.y;
            wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
                    "fleur", server.cursor);
            wlr_seat_pointer_notify_clear_focus(server.seat);
            arrange();
            break;
        case CURSOR_RESIZE:
            /* Doesn't work for X11 output - the next absolute motion event
             * returns the cursor to where it started */
            grabcx = server.cursor->x - grabc->geom.x;
            grabcy = server.cursor->y - grabc->geom.y;
            wlr_cursor_warp_closest(server.cursor, NULL,
                    grabc->geom.x + grabc->geom.width,
                    grabc->geom.y + grabc->geom.height);
            wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
                    "bottom_right_corner", server.cursor);
            wlr_seat_pointer_notify_clear_focus(server.seat);
            arrange();
            break;
        default:
            break;
    }
    return 0;
}

// TODO optimize this function
void motionnotify(uint32_t time)
{
    double sx = 0, sy = 0;
    struct wlr_surface *surface = NULL;

    set_selected_monitor(xytomon(server.cursor->x, server.cursor->y));
    bool action = false;
    struct wlr_box geom;
    /* If we are currently grabbing the mouse, handle and return */
    switch (server.cursorMode) {
        case CURSOR_MOVE:
            action = true;
            geom.x = server.cursor->x - grabcx;
            geom.y = server.cursor->y - grabcy;
            geom.width = grabc->geom.width;
            geom.height = grabc->geom.height;
            /* Move the grabbed client to the new position. */
            resize(grabc, geom, false);
            return;
            break;
        case CURSOR_RESIZE:
            action = true;
            geom.x = grabc->geom.x;
            geom.y = grabc->geom.y;
            geom.width = server.cursor->x - grabc->geom.x;
            geom.height = server.cursor->y - grabc->geom.y;
            resize(grabc, geom, false);
            return;
            break;
        default:
            break;
    }

    bool is_popup = false;
    struct container *con;
    struct monitor *m = selected_monitor;
    if ((con = selected_container(m))) {
        switch (con->client->type) {
            case XDG_SHELL:
                is_popup = !wl_list_empty(&con->client->surface.xdg->popups);
                if (is_popup) {
                    surface = wlr_xdg_surface_surface_at(
                            con->client->surface.xdg,
                            /* absolute mouse position to relative in regards to
                             * the client */
                            server.cursor->x - con->geom.x,
                            server.cursor->y - con->geom.y,
                            &sx, &sy);
                }
                break;
            case LAYER_SHELL:
                is_popup = !wl_list_empty(&con->client->surface.layer->popups);
                if (is_popup) {
                    surface = wlr_layer_surface_v1_surface_at(
                            con->client->surface.layer,
                            server.cursor->x - con->geom.x,
                            server.cursor->y - con->geom.y,
                            &sx, &sy);
                }
                break;
            default:
                break;
        }

        // if surface and subsurface exit
        if (!surface) {
            is_popup = false;
        } else if (surface == selected_container(m)->client->surface.xdg->surface) {
            struct container *con = xytocontainer(server.cursor->x, server.cursor->y);
            if (con) {
                is_popup = is_popup && surface == con->client->surface.xdg->surface;
            }
        }

        if (!surface && !is_popup) {
            if (!wl_list_empty(&popups)) {
                struct xdg_popup *popup = wl_container_of(popups.next, popup, plink);
                wlr_xdg_popup_destroy(popup->xdg->base);
            }
            surface = wlr_surface_surface_at(get_wlrsurface(con->client),
                    server.cursor->x - con->geom.x,
                    server.cursor->y - con->geom.y, &sx, &sy);
        }
    }

    /* If there's no client surface under the server.cursor, set the cursor
     * image to a default. This is what makes the cursor image appear when you
     * move it off of a client or over its border. */
    wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
            "left_ptr", server.cursor);

    // if there is no popup use the selected client's surface
    if (!is_popup) {
        con = xytocontainer(server.cursor->x, server.cursor->y);
        if (con) {
            surface = get_wlrsurface(con->client);
        }
    }
    if (!action && con) {
        pointer_focus(con, surface, sx, sy, time);
    }
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
    focus_top_container(m, FOCUS_NOOP);
    arrange(false);
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
    struct container *sel = selected_container(selected_monitor);
    if (!sel)
        return 0;
    set_container_floating(sel, !sel->floating);
    arrange();
    return 0;
}

int lib_move_client(lua_State *L)
{
    struct wlr_box geom;
    geom.x = server.cursor->x - grabcx;
    geom.y = server.cursor->y - grabcy;
    geom.width = grabc->geom.width;
    geom.height = grabc->geom.height;
    resize(grabc, geom, false);
    return 0;
}

// TODO optimize
int lib_move_client_to_workspace(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace(ui);
    struct container *con = selected_container(m);

    if (!con || con->client->type == LAYER_SHELL)
        return 0;

    con->client->ws = ws;
    arrange();
    focus_top_container(m, FOCUS_NOOP);

    container_damage_whole(con);

    return 0;
}

int lib_resize_client(lua_State *L)
{
    struct wlr_box geom;
    geom.x = grabc->geom.x;
    geom.y = grabc->geom.y;
    geom.width = server.cursor->x - grabc->geom.x;
    geom.height = server.cursor->y - grabc->geom.y;
    resize(grabc, geom, false);
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
    struct container *sel = selected_container(m);

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
        if (!visibleon(con, m) || con->floating)
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
    focus_container(previous, selected_monitor, FOCUS_NOOP);

    if (selected_monitor->ws[0]->layout[0].options.arrange_by_focus) {
        focus_top_container(m, FOCUS_NOOP);
        arrange();
    }
    return 0;
}

int lib_repush(lua_State *L)
{
    int abs_index = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = selected_monitor;
    struct container *sel = selected_container(m);

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
        if (!visibleon(con, m) || con->floating)
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
    struct container *con2;
    if (abs_index > 1) {
        int i = 1;
        wl_list_for_each(con2, &containers, mlink) {
            if (i == abs_index)
                break;
            i++;
        }
    } else if (abs_index == 0) {
        con2 = wl_container_of(&containers, con2, mlink);
    } else {
        int i = wl_list_length(&containers);
        wl_list_for_each_reverse(con2, &containers, mlink) {
            if (i == abs_index)
                break;
            i--;
        }
    }
    wl_list_insert(&con2->mlink, &con->mlink);

    arrange();
    // focus new master window
    focus_container(previous, selected_monitor, FOCUS_NOOP);

    if (selected_monitor->ws[0]->layout[0].options.arrange_by_focus) {
        focus_top_container(m, FOCUS_NOOP);
        arrange();
    }
    return 0;
}

int lib_load_layout(lua_State *L)
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

int lib_kill_client(lua_State *L)
{
    struct client *sel = selected_container(selected_monitor)->client;
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
