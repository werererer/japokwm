#include "actions.h"
#include "client.h"
#include "ipc-server.h"
#include "monitor.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "xdg-shell-protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>
#include "popup.h"

static struct client *grabc = NULL;
static int grabcx, grabcy; /* client-relative */

static void setFloating(struct client *c, bool floating);
static void pointerfocus(struct client *c, struct wlr_surface *surface,
        double sx, double sy, uint32_t time);

static void pointerfocus(struct client *c, struct wlr_surface *surface,
        double sx, double sy, uint32_t time)
{
    /* Use top level surface if nothing more specific given */
    if (c && !surface)
        surface = getWlrSurface(c);

    /* If surface is NULL, clear pointer focus */
    if (!surface) {
        wlr_seat_pointer_notify_clear_focus(server.seat);
        return;
    }

    /* If surface is already focused, only notify of motion */
    if (surface == server.seat->pointer_state.focused_surface) {
        wlr_seat_pointer_notify_motion(server.seat, time, sx, sy);
        return;
    }
    /* Otherwise, let the client know that the mouse cursor has entered one
     * of its surfaces, and make keyboard focus follow if desired. */
    wlr_seat_pointer_notify_enter(server.seat, surface, sx, sy);

    if (c->type == X11Unmanaged)
        return;

    if (sloppyFocus)
        focusClient(selClient(), c, false);
}

int arrangeThis(lua_State *L)
{
    bool reset = lua_toboolean(L, -1);
    arrange(selMon, reset);
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

int updateLayout(lua_State *L)
{
    struct layout l = getConfigLayout(L, "layout");
    setSelLayout(selMon->tagset, l);
    arrange(selMon, true);
    return 0;
}

int focusOnStack(lua_State *L)
{
    struct client *c, *sel = selClient();
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    if (!sel)
        return 0;
    if (i > 0) {
        int j = 1;
        wl_list_for_each(c, &sel->link, link) {
            if (visibleon(c, selMon))
                break;  /* found it */
            j++;
        }
    } else {
        wl_list_for_each_reverse(c, &sel->link, link) {
            if (visibleon(c, selMon))
                break;  /* found it */
        }
    }
    /* If only one client is visible on selMon, then c == sel */
    focusClient(sel, c, true);
    return 0;
}

int focusOnHiddenStack(lua_State *L)
{
    struct client *c, *sel = selClient();
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    if (!sel)
        return 0;
    if (i > 0) {
        int j = 1;
        wl_list_for_each(c, &sel->link, link) {
            if (hiddenon(c, selMon))
                break;  /* found it */
            j++;
        }
    } else {
        wl_list_for_each_reverse(c, &sel->link, link) {
            if (hiddenon(c, selMon))
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
    focusClient(sel, c, true);
    arrange(selMon, false);
    return 0;
}

int moveResize(lua_State *L)
{
    int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    grabc = xytoclient(server.cursor->x, server.cursor->y);
    if (!grabc)
        return 0;

    /* Float the window and tell motionnotify to grab it */
    setFloating(grabc, true);
    switch (server.cursorMode = ui) {
        case CurMove:
            grabcx = server.cursor->x - grabc->geom.x;
            grabcy = server.cursor->y - grabc->geom.y;
            wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
                    "fleur", server.cursor);
            break;
        case CurResize:
            /* Doesn't work for X11 output - the next absolute motion event
             * returns the cursor to where it started */
            wlr_cursor_warp_closest(server.cursor, NULL,
                    grabc->geom.x + grabc->geom.width,
                    grabc->geom.y + grabc->geom.height);
            wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
                    "bottom_right_corner", server.cursor);
            break;
        default:
            break;
    }
    return 0;
}

static void setFloating(struct client *c, bool floating)
{
    if (c->floating == floating)
        return;
    c->floating = floating;
    arrange(c->mon, false);
}

void motionnotify(uint32_t time)
{
    double sx = 0, sy = 0;
    struct wlr_surface *surface = NULL;
    struct client *c;

    /* If we are currently grabbing the mouse, handle and return */
    switch (server.cursorMode) {
        case CurMove:
            /* Move the grabbed client to the new position. */
            resize(grabc, server.cursor->x - grabcx, server.cursor->y - grabcy,
                    grabc->geom.width, grabc->geom.height, true);
            return;
            break;
        case CurResize:
            resize(grabc, grabc->geom.x, grabc->geom.y,
                    server.cursor->x - grabc->geom.x,
                    server.cursor->y - grabc->geom.y, true);
            return;
            break;
        default:
            break;
    }

    bool isPopup = false;
    if ((c = selClient())) {
        switch (c->type) {
            case XDGShell:
                isPopup = !wl_list_empty(&c->surface.xdg->popups);
                if (isPopup) {
                    surface = wlr_xdg_surface_surface_at(
                            c->surface.xdg,
                            /* absolute mouse position to relative in regards to
                             * the client */
                            server.cursor->x - c->geom.x,
                            server.cursor->y - c->geom.y,
                            &sx, &sy);
                }
                break;
            case LayerShell:
                isPopup = !wl_list_empty(&c->surface.layer->popups);
                if (isPopup) {
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
        if (!surface) {
            struct xdg_popup *popup, *tmp;
            wl_list_for_each_safe(popup, tmp, &popups, link) {
                wlr_xdg_popup_destroy(popup->xdg);
            }
            surface = wlr_surface_surface_at(getWlrSurface(c),
                    server.cursor->x - c->geom.x - c->bw,
                    server.cursor->y - c->geom.y - c->bw, &sx, &sy);
        }
    }

    /* If there's no client surface under the server.cursor, set the cursor image to a
     * default. This is what makes the cursor image appear when you move it
     * off of a client or over its border. */
    if (!surface) {
        wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
                "left_ptr", server.cursor);
    }

    if (!isPopup)
        c = xytoclient(server.cursor->x, server.cursor->y);
    pointerfocus(c, surface, sx, sy, time);
}

int tag(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    printf("TAG\n");
    ipc_event_workspace();
    struct client *sel = selClient();
    if (sel && ui) {
        toggleAddTag(sel->tagset, positionToFlag(ui));
        focusTopClient(nextClient(), true);
        arrange(selMon, false);
    }
    return 0;
}

int toggletag(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct client *sel = selClient();
    if (sel) {
        unsigned int newtags = sel->tagset->selTags[0] ^ ui;
        if (newtags) {
            setSelTags(sel->tagset, newtags);
            focusTopClient(nextClient(), true);
            arrange(selMon, false);
        }
    }
    return 0;
}

int view(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    selMon->tagset->focusedTag = flagToPosition(ui);
    setSelTags(selMon->tagset, ui);
    focusTopClient(nextClient(), false);
    arrange(selMon, false);
    return 0;
}

int toggleAddView(lua_State *L)
{
    unsigned int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    toggleAddTag(selMon->tagset, ui);
    focusTopClient(nextClient(), false);
    arrange(selMon, false);
    return 0;
}


int toggleView(lua_State *L)
{
    toggleTagset(selMon->tagset);
    focusTopClient(nextClient(), true);
    arrange(selMon, false);
    return 0;
}

int toggleFloating(lua_State *L)
{
    struct client *sel = selClient();
    if (!sel)
        return 0;
    /* return if fullscreen */
    setFloating(sel, !sel->floating /* || sel->isfixed */);
    return 0;
}

int moveClient(lua_State *L)
{
    resize(grabc, server.cursor->x - grabcx, server.cursor->y - grabcy,
            grabc->geom.width, grabc->geom.height, 1);
    return 0;
}

int resizeClient(lua_State *L)
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
    struct client *c, *old = selClient();

    if (!old || old->floating)
        return 0;

    /* Search for the first tiled window that is not sel, marking sel as
     * NULL if we pass it along the way */
    wl_list_for_each(c, &clients,
            link) if (visibleon(c, selMon) && !c->floating) {
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

    focusClient(nextClient(), old, true);
    arrange(selMon, false);
    return 0;
}

int readOverlay(lua_State *L)
{
    char file[NUM_CHARS];
    char filename[NUM_DIGITS];
    const char *layout = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    // create array for each file
    lua_newtable(L);

    // tags are counted from 1
    for (int i = 1; i <= 9; i++) {
        intToString(filename, i);
        strcpy(file, "layouts");
        joinPath(file, layout);
        joinPath(file, filename);


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

    return 1;
}

int killClient(lua_State *L)
{
    struct client *sel = selClient();
    if (sel) {
        switch (sel->type) {
            case XDGShell:
                wlr_xdg_toplevel_send_close(sel->surface.xdg);
                break;
            case LayerShell:
                wlr_layer_surface_v1_close(sel->surface.layer);
                break;
            case X11Managed:
            case X11Unmanaged:
                wlr_xwayland_surface_close(sel->surface.xwayland);
        }
    }
    return 0;
}
