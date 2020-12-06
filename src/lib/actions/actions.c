#include "lib/actions/actions.h"

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
#include "tile/tileTexture.h"
#include "tile/tileUtils.h"
#include "utils/stringUtils.h"
#include "xdg-shell-protocol.h"

static struct container *grabc = NULL;
static int grabcx, grabcy; /* client-relative */

static void set_container_floating(struct container *con, bool floating);
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

    if (sloppyFocus)
        focus_container(selected_monitor, con, ACTION_NOOP);
}

int arrange_this(lua_State *L)
{
    bool reset = lua_toboolean(L, -1);
    lua_pop(L, 1);
    arrange(reset);
    return 0;
}

int toggle_consider_layer_shell(lua_State *L)
{
    root.consider_layer_shell = !root.consider_layer_shell;
    arrange(false);
    return 0;
}

int spawn(lua_State *L)
{
    const char *cmd = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    if (fork() == 0) {
        setsid();
        execl("/bin/sh", "/bin/sh", "-c", cmd, (void *)NULL);
    }
    return 0;
}

int update_layout(lua_State *L)
{
    struct layout l = get_config_layout(L, "layout");
    set_selected_layout(selected_monitor->tagset, l);
    arrange(true);
    return 0;
}

int focus_on_stack(lua_State *L)
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
        focus_container(m, con, ACTION_LIFT);
    }
    return 0;
}

int focus_on_hidden_stack(lua_State *L)
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
        wl_list_insert(m->containers.prev, &sel->mlink);
    }

    focus_container(m, con, ACTION_LIFT);
    arrange(false);
    return 0;
}

int move_resize(lua_State *L)
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
            arrange(false);
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
            arrange(false);
            break;
        default:
            break;
    }
    return 0;
}

static void set_container_floating(struct container *con, bool floating)
{
    if (con->floating == floating)
        return;
    con->floating = floating;
    lift_container(con);
    arrange(false);
}

// TODO optimize this function
void motionnotify(uint32_t time)
{
    double sx = 0, sy = 0;
    struct wlr_surface *surface = NULL;

    selected_monitor = xytomon(server.cursor->x, server.cursor->y);
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
            resize(grabc, geom, true);
            update_container_overlay(grabc);
            return;
            break;
        case CURSOR_RESIZE:
            action = true;
            geom.x = grabc->geom.y;
            geom.y = server.cursor->y - grabcy;
            geom.width = server.cursor->x - grabc->geom.x;
            geom.height = server.cursor->y - grabc->geom.y;
            resize(grabc, geom, true);
            update_container_overlay(grabc);
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
            struct xdg_popup *popup, *tmp;
            wl_list_for_each_safe(popup, tmp, &popups, link) {
                wlr_xdg_popup_destroy(popup->xdg->base);
            }
            surface = wlr_surface_surface_at(get_wlrsurface(con->client),
                    server.cursor->x - con->geom.x,
                    server.cursor->y - con->geom.y, &sx, &sy);
        }
    }

    /* If there's no client surface under the server.cursor, set the cursor image to a
     * default. This is what makes the cursor image appear when you move it
     * off of a client or over its border. */
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

int tag(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = selected_monitor;
    struct container *sel = selected_container(m);

    ipc_event_workspace();

    if (!sel || !ui)
        return 0;

    toggle_add_tag(sel->client->tagset, position_to_flag(ui));
    focus_top_container(m, ACTION_LIFT);
    arrange(false);
    return 0;
}

int toggle_tag(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct container *sel = selected_container(selected_monitor);
    if (sel) {
        unsigned int newtags = sel->client->tagset->selTags[0] ^ ui;
        if (newtags) {
            set_selelected_Tags(sel->client->tagset, newtags);
            focus_top_container(selected_monitor, ACTION_LIFT);
            arrange(false);
        }
    }
    return 0;
}

int view(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = selected_monitor;
    m->tagset->focusedTag = flag_to_position(ui);
    set_selelected_Tags(m->tagset, ui);
    focus_top_container(m, ACTION_NOOP);
    arrange(false);
    return 0;
}

int toggle_add_view(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = selected_monitor;
    toggle_add_tag(m->tagset, ui);
    focus_top_container(m, ACTION_NOOP);
    arrange(false);
    return 0;
}


int toggle_view(lua_State *L)
{
    struct monitor *m = selected_monitor;
    toggle_tagset(m->tagset);
    focus_top_container(m, ACTION_LIFT);
    arrange(false);
    return 0;
}

int toggle_floating(lua_State *L)
{
    struct container *sel = selected_container(selected_monitor);
    if (!sel)
        return 0;
    set_container_floating(sel, !sel->floating);
    return 0;
}

int move_client(lua_State *L)
{
    struct wlr_box geom;
    geom.x = server.cursor->x - grabcx;
    geom.y = server.cursor->y - grabcy;
    geom.width = grabc->geom.width;
    geom.height = grabc->geom.height;
    resize(grabc, geom, 1);
    return 0;
}

int resize_client(lua_State *L)
{
    struct wlr_box geom;
    geom.x = grabc->geom.x;
    geom.y = grabc->geom.y;
    geom.width = server.cursor->x - grabc->geom.x;
    geom.height =  server.cursor->y - grabc->geom.y;
    resize(grabc, geom, true);
    return 0;
}

int quit(lua_State *L)
{
    wl_display_terminate(server.display);
    return 0;
}

int zoom(lua_State *L)
{
    struct monitor *m = selected_monitor;
    struct container *c, *old = selected_container(m);

    if (!old || old->floating)
        return 0;

    /* Search for the first tiled window that is not sel, marking sel as
     * NULL if we pass it along the way */
    struct container *con;
    wl_list_for_each(con, &m->stack, slink) {
        if (visibleon(con, m) && !c->floating) {
            if (c != old)
                break;
            old = NULL;
        }
    }

    focus_container(selected_monitor, con, ACTION_NOOP);
    return 0;
}

int read_overlay(lua_State *L)
{
    char file[NUM_CHARS];
    char filename[NUM_DIGITS];
    const char *layout = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    char *config_path = get_config_file("layouts");

    // create array for each file
    lua_newtable(L);

    // tags are counted from 1
    for (int i = 1; i <= 9; i++) {
        intToString(filename, i);
        strcpy(file, "");
        join_path(file, config_path);
        join_path(file, layout);
        join_path(file, filename);

        FILE *fp;
        if ( (fp = fopen(file, "r")) == NULL)
            break;
        // create inner array for each line
        lua_newtable(L);
        int j = 1;
        size_t g;
        char *line = NULL;
        while ((getline(&line, &g, fp)) != -1) {
            // create inner array for each number in file
            lua_newtable(L);

            struct wlr_fbox box;
            char *pend;
            box.x = strtof(line, &pend);
            box.y = strtof(pend, &pend);
            box.width = strtof(pend, &pend);
            box.height = strtof(pend, NULL);
            lua_pushnumber(L, box.x);
            lua_pushnumber(L, box.y);
            lua_pushnumber(L, box.width);
            lua_pushnumber(L, box.height);
            lua_rawseti(L, -5, 4);
            lua_rawseti(L, -4, 3);
            lua_rawseti(L, -3, 2);
            lua_rawseti(L, -2, 1);

            lua_rawseti(L, -2, j);
            j++;
        }
        lua_rawseti(L, -2, i);
        fclose(fp);
    }

    free(config_path);
    return 1;
}

int kill_client(lua_State *L)
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
