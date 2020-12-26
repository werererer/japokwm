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
#include "tile/tileTexture.h"
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
        printf("clear\n");
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

    if (sloppy_focus)
        focus_container(con, selected_monitor, FOCUS_NOOP);
}

int set_tabcount(lua_State *L)
{
    printf("set tabcount\n");
    /* int i = luaL_checkinteger(L, -1); */
    lua_pop(L, 1);
    /* selected_container(selected_monitor)->tabcount = i; */
    arrange(LAYOUT_NOOP);
    return 0;
}

int get_tabcount(lua_State *L)
{
    /* lua_pushinteger(L, selected_container(selected_monitor)->tabcount); */
    return 1;
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
    selected_monitor->root->consider_layer_shell = !selected_monitor->root->consider_layer_shell;
    arrange(false);
    return 0;
}

int set_floating(lua_State *L)
{
    bool b = lua_toboolean(L, -1);
    lua_pop(L, 1);
    struct container *sel = selected_container(selected_monitor);
    if (!sel)
        return 0;
    set_container_floating(sel, b);
    arrange(LAYOUT_NOOP);
    return 0;
}

int set_nmaster(lua_State *L)
{
    selected_monitor->ws->layout.nmaster = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    arrange(LAYOUT_NOOP);
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
    set_selected_layout(selected_monitor->ws, l);
    arrange(LAYOUT_RESET);
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
        focus_container(con, m, FOCUS_LIFT);
    }
    return 0;
}

int get_nmaster(lua_State *L)
{
    lua_pushinteger(L, selected_monitor->ws->layout.nmaster);
    return 1;
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
        wl_list_insert(containers.prev, &sel->mlink);
    }

    focus_container(con, m, FOCUS_LIFT);
    arrange(LAYOUT_NOOP);
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
            printf("clear2\n");
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
            printf("clear3\n");
            wlr_seat_pointer_notify_clear_focus(server.seat);
            arrange(false);
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
            update_container_overlay(grabc);
            return;
            break;
        case CURSOR_RESIZE:
            action = true;
            geom.x = grabc->geom.x;
            geom.y = grabc->geom.y;
            geom.width = server.cursor->x - grabc->geom.x;
            geom.height = server.cursor->y - grabc->geom.y;
            resize(grabc, geom, false);
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

int tag(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = selected_monitor;
    struct container *sel = selected_container(m);

    ipc_event_workspace();

    if (!sel)
        return 0;

    struct workspace *ws = get_workspace(ui);
    if (is_workspace_occupied(ws)) {
        set_selected_monitor(ws->m);
        return 0;
    }
    set_next_unoccupied_workspace(m, ws);
    focus_top_container(m, FOCUS_LIFT);
    arrange(false);
    return 0;
}

int toggle_tag(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct container *sel = selected_container(selected_monitor);
    struct monitor *m = selected_monitor;
    if (!sel)
        return 0;
    struct workspace *ws = get_workspace(ui);
    if (is_workspace_occupied(ws)) {
        printf("set_selected_monitor\n");
        set_selected_monitor(ws->m);
        return 0;
    }
    set_next_unoccupied_workspace(m, ws);
    focus_top_container(m, FOCUS_LIFT);
    arrange(false);
    return 0;
}

int view(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace(ui);
    if (is_workspace_occupied(ws)) {
        set_selected_monitor(ws->m);
        return 0;
    }
    set_next_unoccupied_workspace(m, ws);
    focus_top_container(m, FOCUS_NOOP);
    arrange(false);
    return 0;
}

int toggle_view(lua_State *L)
{
    struct monitor *m = selected_monitor;
    focus_top_container(m, FOCUS_LIFT);
    arrange(false);
    return 0;
}

int toggle_floating(lua_State *L)
{
    struct container *sel = selected_container(selected_monitor);
    if (!sel)
        return 0;
    set_container_floating(sel, !sel->floating);
    arrange(LAYOUT_NOOP);
    return 0;
}

int move_client(lua_State *L)
{
    struct wlr_box geom;
    geom.x = server.cursor->x - grabcx;
    geom.y = server.cursor->y - grabcy;
    geom.width = grabc->geom.width;
    geom.height = grabc->geom.height;
    resize(grabc, geom, false);
    return 0;
}

int resize_client(lua_State *L)
{
    struct wlr_box geom;
    geom.x = grabc->geom.x;
    geom.y = grabc->geom.y;
    geom.width = server.cursor->x - grabc->geom.x;
    geom.height = server.cursor->y - grabc->geom.y;
    resize(grabc, geom, false);
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
    arrange(LAYOUT_NOOP);
    // focus new master window
    focus_container(previous, selected_monitor, FOCUS_NOOP);
    return 0;
}

static struct wlr_fbox string_to_wlr_fbox(const char *str)
{
    struct wlr_fbox box;
    char *pend;
    box.x = strtof(str, &pend);
    box.y = strtof(pend, &pend);
    box.width = strtof(pend, &pend);
    box.height = strtof(pend, NULL);
    return box;
}

static void lua_push_wlr_fbox(struct wlr_fbox box)
{
    // create inner array for each number in file
    lua_newtable(L);
    lua_pushnumber(L, box.x);
    lua_pushnumber(L, box.y);
    lua_pushnumber(L, box.width);
    lua_pushnumber(L, box.height);
    lua_rawseti(L, -5, 4);
    lua_rawseti(L, -4, 3);
    lua_rawseti(L, -3, 2);
    lua_rawseti(L, -2, 1);
}

int read_layout(lua_State *L)
{
    const char *layout = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    /* // create array for each file */
    /* lua_newtable(L); */

    char *config_path = get_config_file("layouts");
    /* // workspaces are counted up from 1 */
    /* for (int i = 1; i <= 9; i++) { */
    /*     char filename[NUM_DIGITS]; */
    /*     intToString(filename, i); */

        char file[NUM_CHARS];
        strcpy(file, "");
        join_path(file, config_path);
        join_path(file, layout);
        join_path(file, "layout.lua");

    /*     FILE *fp; */
    /*     if ( (fp = fopen(file, "r")) == NULL) */
    /*         break; // failed to open */

    /*     lua_newtable(L); */
    /*     int j = 1; */
    /*     size_t g; */
    /*     char *line = NULL; */
    /*     while ((getline(&line, &g, fp)) != -1) { */
    /*         struct wlr_fbox box = string_to_wlr_fbox(line); */
    /*         lua_push_wlr_fbox(box); */
    /*         lua_rawseti(L, -2, j); */
    /*         j++; */
    /*     } */
    /*     lua_rawseti(L, -2, i); */
    /*     fclose(fp); */
    /* } */

    /* free(config_path); */

    /* lua_pushstring(L, layout); */
    /* int nret = read_master_layout(L); */
    /* lua_pushstring(L, layout); */
    /* int nret2 = read_boxes(L); */

    if (!file_exists(file))
        return 0;

    if (luaL_loadfile(L, file)) {
        lua_pop(L, 1);
        return 0;
    }

    lua_pcall(L, 0, 0, 0);
    lua_pop(L, 1);
    return 0;
}

int read_master_layout(lua_State *L)
{
    const char *layout = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    char *config_path = get_config_file("layouts");

    char f[NUM_CHARS];
    strcpy(f, "");
    join_path(f, config_path);
    join_path(f, layout);
    join_path(f, "m1");
    if (!file_exists(f))
        return 0;

    // create array for each file
    lua_newtable(L);

    // workspaces are counted up from 1
    for (int i = 1; i <= 9; i++) {
        char number[NUM_DIGITS];
        intToString(number, i);
        char filename[NUM_DIGITS + NUM_CHARS];
        strcpy(filename, "m");
        strcat(filename, number);

        char file[NUM_CHARS];
        strcpy(file, "");
        join_path(file, config_path);
        join_path(file, layout);
        join_path(file, filename);

        FILE *fp;
        if ( (fp = fopen(file, "r")) == NULL)
            break; // failed to open

        lua_newtable(L);
        int j = 1;
        size_t g;
        char *line = NULL;
        while ((getline(&line, &g, fp)) != -1) {
            struct wlr_fbox box = string_to_wlr_fbox(line);
            lua_push_wlr_fbox(box);
            lua_rawseti(L, -2, j);
            j++;
        }
        lua_rawseti(L, -2, i);
        fclose(fp);
    }

    free(config_path);
    return 1;
}

int read_boxes(lua_State *L)
{
    const char *layout = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    char *config_path = get_config_file("layouts");

    char filename[NUM_CHARS];
    strcpy(filename, "r");

    char file[NUM_CHARS]; strcpy(file, "");
    join_path(file, config_path);
    join_path(file, layout);
    join_path(file, filename);

    FILE *fp;
    if ( (fp = fopen(file, "r")) == NULL)
        return 0; // failed to open
    lua_newtable(L);

    int i = 1;
    size_t g;
    char *line = NULL;
    while ((getline(&line, &g, fp)) != -1) {

        lua_newtable(L);
        struct wlr_list b = split_string(line, " ");
        for (int j = 0; j < b.length; j++) {
            int k = strtol(b.items[j], NULL, 10);
            lua_pushinteger(L, k);
            lua_rawseti(L, -2, j+1);
        }

        lua_rawseti(L, -2, i);
        i++;
    }
    fclose(fp);

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
