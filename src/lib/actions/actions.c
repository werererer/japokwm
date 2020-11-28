#include "lib/actions/actions.h"

#include <lua.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "client.h"
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

static struct client *grabc = NULL;
static int grabcx, grabcy; /* client-relative */

static void set_client_floating(struct client *c, bool floating);
static void pointer_focus(struct client *c, struct wlr_surface *surface,
        double sx, double sy, uint32_t time);

static void pointer_focus(struct client *c, struct wlr_surface *surface,
        double sx, double sy, uint32_t time)
{
    /* Use top level surface if nothing more specific given */
    if (c && !surface)
        surface = get_wlrsurface(c);

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

    if (c->type == X11_UNMANAGED)
        return;

    if (sloppyFocus)
        focus_client(selected_client(), c, false);
}

int arrange_this(lua_State *L)
{
    bool reset = lua_toboolean(L, -1);
    lua_pop(L, 1);
    arrange(selected_monitor, reset);
    return 0;
}

int toggle_consider_layer_shell(lua_State *L)
{
    root.consider_layer_shell = !root.consider_layer_shell;
    arrange(selected_monitor, false);
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
    arrange(selected_monitor, true);
    return 0;
}

int focus_on_stack(lua_State *L)
{
    struct client *c, *sel = selected_client();
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    if (!sel)
        return 0;
    if (i > 0) {
        int j = 1;
        wl_list_for_each(c, &sel->link, link) {
            if (visibleon(c, selected_monitor))
                break;  /* found it */
            j++;
        }
    } else {
        wl_list_for_each_reverse(c, &sel->link, link) {
            if (visibleon(c, selected_monitor))
                break;  /* found it */
        }
    }
    /* If only one client is visible on selMon, then c == sel */
    focus_client(sel, c, true);
    return 0;
}

int focus_on_hidden_stack(lua_State *L)
{
    struct client *c, *sel = selected_client();
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    if (!sel)
        return 0;
    if (i > 0) {
        int j = 1;
        wl_list_for_each(c, &sel->link, link) {
            if (hiddenon(c, selected_monitor))
                break;  /* found it */
            j++;
        }
    } else {
        wl_list_for_each_reverse(c, &sel->link, link) {
            if (hiddenon(c, selected_monitor))
                break;  /* found it */
        }
    }
    if (sel && c) {
        if (sel == c)
            return 0;
        // replace current client with a hidden one
        wl_list_remove(&c->link);
        wl_list_insert(&sel->link, &c->link);
        wl_list_remove(&sel->link);
        wl_list_insert(clients.prev, &sel->link);
    }
    /* If only one client is visible on selMon, then c == sel */
    focus_client(sel, c, true);
    arrange(selected_monitor, false);
    return 0;
}

int move_resize(lua_State *L)
{
    int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    grabc = xytoclient(server.cursor->x, server.cursor->y);
    if (!grabc)
        return 0;

    /* Float the window and tell motionnotify to grab it */
    set_client_floating(grabc, true);
    switch (server.cursorMode = ui) {
        case CURSOR_MOVE:
            grabcx = server.cursor->x - grabc->geom.x;
            grabcy = server.cursor->y - grabc->geom.y;
            wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
                    "fleur", server.cursor);
            wlr_seat_pointer_notify_clear_focus(server.seat);
            arrange(selected_monitor, false);
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
            arrange(selected_monitor, false);
            break;
        default:
            break;
    }
    return 0;
}

static void set_client_floating(struct client *c, bool floating)
{
    if (c->floating == floating)
        return;
    c->floating = floating;
    lift_client(c);
    arrange(selected_monitor, false);
}

// TODO optimize this function
void motionnotify(uint32_t time)
{
    double sx = 0, sy = 0;
    struct wlr_surface *surface = NULL;
    struct client *c;

    selected_monitor = xytomon(server.cursor->x, server.cursor->y);
    bool action = false;
    /* If we are currently grabbing the mouse, handle and return */
    switch (server.cursorMode) {
        case CURSOR_MOVE:
            action = true;
            /* Move the grabbed client to the new position. */
            resize(grabc, server.cursor->x - grabcx, server.cursor->y - grabcy,
                    grabc->geom.width, grabc->geom.height, true);
            update_client_overlay(grabc);
            return;
            break;
        case CURSOR_RESIZE:
            action = true;
            resize(grabc, grabc->geom.x, grabc->geom.y,
                    server.cursor->x - grabc->geom.x,
                    server.cursor->y - grabc->geom.y, true);
            update_client_overlay(grabc);
            return;
            break;
        default:
            break;
    }

    bool is_popup = false;
    if ((c = selected_client())) {
        switch (c->type) {
            case XDG_SHELL:
                is_popup = !wl_list_empty(&c->surface.xdg->popups);
                if (is_popup) {
                    surface = wlr_xdg_surface_surface_at(
                            c->surface.xdg,
                            /* absolute mouse position to relative in regards to
                             * the client */
                            server.cursor->x - c->geom.x,
                            server.cursor->y - c->geom.y,
                            &sx, &sy);
                }
                break;
            case LAYER_SHELL:
                is_popup = !wl_list_empty(&c->surface.layer->popups);
                if (is_popup) {
                    surface = wlr_layer_surface_v1_surface_at(
                            c->surface.layer,
                            server.cursor->x - c->geom.x,
                            server.cursor->y - c->geom.y,
                            &sx, &sy);
                }
                break;
            default:
                break;
        }

        // if surface and subsurface exit
        if (!surface) {
            is_popup = false;
        } else if (surface == selected_client()->surface.xdg->surface) {
            struct client *c = xytoclient(server.cursor->x, server.cursor->y);
            if (c) {
                is_popup = is_popup && surface == c->surface.xdg->surface;
            }
        }

        if (!surface && !is_popup) {
            struct xdg_popup *popup, *tmp;
            wl_list_for_each_safe(popup, tmp, &popups, link) {
                wlr_xdg_popup_destroy(popup->xdg->base);
            }
            surface = wlr_surface_surface_at(get_wlrsurface(c),
                    server.cursor->x - c->geom.x,
                    server.cursor->y - c->geom.y, &sx, &sy);
        }
    }

    /* If there's no client surface under the server.cursor, set the cursor image to a
     * default. This is what makes the cursor image appear when you move it
     * off of a client or over its border. */
    wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
            "left_ptr", server.cursor);

    // if there is no popup use the selected client's surface
    if (!is_popup) {
        c = xytoclient(server.cursor->x, server.cursor->y);
        surface = get_wlrsurface(c);
    }
    if (!action) {
        pointer_focus(c, surface, sx, sy, time);
    }
}

int tag(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    printf("TAG\n");
    ipc_event_workspace();
    struct client *sel = selected_client();
    if (sel && ui) {
        toggle_add_tag(sel->tagset, position_to_flag(ui));
        focus_top_client(next_client(), true);
        arrange(selected_monitor, false);
    }
    return 0;
}

int toggle_tag(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct client *sel = selected_client();
    if (sel) {
        unsigned int newtags = sel->tagset->selTags[0] ^ ui;
        if (newtags) {
            set_selelected_Tags(sel->tagset, newtags);
            focus_top_client(next_client(), true);
            arrange(selected_monitor, false);
        }
    }
    return 0;
}

int view(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    selected_monitor->tagset->focusedTag = flag_to_position(ui);
    set_selelected_Tags(selected_monitor->tagset, ui);
    focus_top_client(next_client(), false);
    arrange(selected_monitor, false);
    return 0;
}

int toggle_add_view(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    toggle_add_tag(selected_monitor->tagset, ui);
    focus_top_client(next_client(), false);
    arrange(selected_monitor, false);
    return 0;
}


int toggle_view(lua_State *L)
{
    toggle_tagset(selected_monitor->tagset);
    focus_top_client(next_client(), true);
    arrange(selected_monitor, false);
    return 0;
}

int toggle_floating(lua_State *L)
{
    struct client *sel = selected_client();
    if (!sel)
        return 0;
    /* return if fullscreen */
    set_client_floating(sel, !sel->floating /* || sel->isfixed */);
    return 0;
}

int move_client(lua_State *L)
{
    resize(grabc, server.cursor->x - grabcx, server.cursor->y - grabcy,
            grabc->geom.width, grabc->geom.height, 1);
    return 0;
}

int resize_client(lua_State *L)
{
    resize(grabc, grabc->geom.x, grabc->geom.y,
            server.cursor->x - grabc->geom.x,
            server.cursor->y - grabc->geom.y, 1);
    return 0;
}

int quit(lua_State *L)
{
    wl_display_terminate(server.display);
    return 0;
}

int zoom(lua_State *L)
{
    struct client *c, *old = selected_client();

    if (!old || old->floating)
        return 0;

    /* Search for the first tiled window that is not sel, marking sel as
     * NULL if we pass it along the way */
    wl_list_for_each(c, &clients,
            link) if (visibleon(c, selected_monitor) && !c->floating) {
        if (c != old)
            break;
        old = NULL;
    }

    /* Return if no other tiled window was found */
    if (&c->link == &clients)
        return 0;

    /* If we passed sel, move c to the front; otherwise, move sel to the
     * front */
    if (!old)
        old = c;
    wl_list_remove(&old->link);
    wl_list_insert(&clients, &old->link);

    focus_client(next_client(), old, true);
    arrange(selected_monitor, false);
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
    struct client *sel = selected_client();
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
