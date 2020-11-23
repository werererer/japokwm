#include "tile/tileUtils.h"
#include <client.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <wayland-util.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <stdlib.h>

#include "monitor.h"
#include "tagset.h"
#include "utils/coreUtils.h"
#include "parseConfig.h"
#include "tile/tileTexture.h"
#include "utils/gapUtils.h"
#include "utils/parseConfigUtils.h"
#include "root.h"

struct containersInfo containersInfo;

/* *
 * the wlr_fbox has includes the window size in percent.
 * It will we mulitiplicated with the screen width and height
 * */
struct wlr_box get_absolute_box(struct wlr_box box, struct wlr_fbox b)
{
    struct wlr_box w = box;
    w.x = w.width * b.x + w.x;
    w.y = w.height * b.y + w.y;
    w.width = w.width * b.width;
    w.height = w.height * b.height;
    return w;
}

struct wlr_fbox get_relative_box(struct wlr_box box, struct wlr_box b)
{
    struct wlr_fbox w;
    w.x = (float)box.x / b.width;
    w.y = (float)box.y / b.height;
    w.width = (float)box.width / b.width;
    w.height = (float)box.height / b.height;
    return w;
}

void arrange(struct monitor *m, bool reset)
{
    /* Get effective monitor geometry to use for window area */
    struct tagset *tagset = m->tagset;
    m->m = *wlr_output_layout_get_box(output_layout, m->output);

    setRootArea(m);
    containerSurroundGaps(&root.w, outerGap);
    if (selected_layout(tagset).funcId) {
        struct client *c = NULL;

        int n = tiledClientCount(m);
        /* call arrange function
         * if previous layout is different or reset -> reset layout */
        if (strcmp(prevLayout.symbol, selected_layout(tagset).symbol) != 0 || reset) {
            prevLayout = selected_layout(tagset);
            lua_rawgeti(L, LUA_REGISTRYINDEX, selected_layout(tagset).funcId);
            lua_pcall(L, 0, 0, 0);
        }

        /* update layout aquired from that was set with the arrange function */
        lua_getglobal(L, "update");
        lua_pushinteger(L, n);
        lua_pcall(L, 1, 1, 0);
        containersInfo.n = lua_rawlen(L, -1);
        containersInfo.id = luaL_ref(L, LUA_REGISTRYINDEX);

        update_hidden_status();

        int i = 0;
        wl_list_for_each(c, &clients, link) {
            if (c->hidden || !visibleon(c, m))
                continue;

            c->position = i;
            arrange_client(c);
            if (!c->floating)
                i++;
        }
        update_overlay_count(i);
    }
}

void arrange_client(struct client *c)
{
    if (c->hidden)
        return;

    // if tiled get tile information from tile function and apply it
    struct wlr_fbox con;
    if (!c->floating) {
        // get lua container
        lua_rawgeti(L, LUA_REGISTRYINDEX, containersInfo.id);
        lua_rawgeti(L, -1, MIN(c->position+1, containersInfo.n));
        lua_rawgeti(L, -1, 1);
        con.x = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 2);
        con.y = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 3);
        con.width = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 4);
        con.height = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_pop(L, 1);
        struct wlr_box box = get_absolute_box(root.w, con);
        containerSurroundGaps(&box, innerGap);
        resize(c, box.x, box.y, box.width, box.height, false);
        containersInfo.id = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    update_client_overlay(c);
}

void resize(struct client *c, int x, int y, int w, int h, bool interact)
{
    /*
     * Note that I took some shortcuts here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at
     * the new size, then commit any movement that was prepared.
     */
    struct wlr_box box;
    // layershell based programs usually don't get borders
    box.x = x;
    box.y = y;
    box.width = w;
    box.height = h;

    c->geom = box;
    applybounds(c, box);

    /* wlroots makes this a no-op if size hasn't changed */
    switch (c->type) {
        case XDG_SHELL:
            c->resize = wlr_xdg_toplevel_set_size(c->surface.xdg,
                    c->geom.width, c->geom.height);
            break;
        case LAYER_SHELL:
            wlr_layer_surface_v1_configure(c->surface.layer,
                    c->geom.width, c->geom.height);
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            wlr_xwayland_surface_configure(c->surface.xwayland,
                    c->geom.x, c->geom.y, c->geom.width, c->geom.height);
    }
}

void update_hidden_status()
{
    struct client *c;
    int i = 0;
    wl_list_for_each(c, &clients, link) {
        // floating windows are always visible
        if (c->floating == true) {
            c->hidden = false;
            continue;
        }
        if (existon(c, selected_monitor))
        {
            if (i < containersInfo.n)
                c->hidden = false;
            else
                c->hidden = true;
            i++;
        }
    }
}

int tiledClientCount(struct monitor *m)
{
    struct client *c;
    int n = 0;

    wl_list_for_each(c, &clients, link)
        if (existon(c, m) && !c->floating)
            n++;
    return n;
}
