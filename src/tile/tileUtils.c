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

struct containers_info containers_info;

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

    set_root_area(m);
    if (!overlay)
        container_surround_gaps(&root.w, outerGap);
    if (selected_layout(tagset).funcId) {
        struct client *c = NULL;

        int n = tiled_client_count(m);
        /* call arrange function
         * if previous layout is different or reset -> reset layout */
        if (strcmp(prev_layout.symbol, selected_layout(tagset).symbol) != 0 || reset) {
            prev_layout = selected_layout(tagset);
            lua_rawgeti(L, LUA_REGISTRYINDEX, selected_layout(tagset).funcId);
            lua_pushinteger(L, n);
            lua_pcall(L, 1, 0, 0);
        }

        /* update layout aquired from that was set with the arrange function */
        lua_getglobal(L, "update");
        lua_pushinteger(L, n);
        lua_pcall(L, 1, 1, 0);
        containers_info.n = lua_rawlen(L, -1);
        containers_info.id = luaL_ref(L, LUA_REGISTRYINDEX);

        update_hidden_status();

        int i = 0;
        wl_list_for_each(c, &clients, link) {
            if (c->hidden || !visibleon(c, m))
                continue;
            if (c->floating)
                continue;

            arrange_client(c, i);
            i++;
        }
        wl_list_for_each(c, &clients, link) {
            if (c->hidden || !visibleon(c, m))
                continue;
            if (!c->floating)
                continue;

            arrange_client(c, i);
            i++;
        }
        i = 0;
        wl_list_for_each(c, &clients, link) {
            if (c->hidden || !visibleon(c, m))
                continue;

            c->textPosition = i;
            i++;
        }
        update_overlay();
    }
}

void arrange_client(struct client *c, int i)
{
    if (c->hidden)
        return;
    c->clientPosition = i;

    // if tiled get tile information from tile function and apply it
    struct wlr_fbox con;
    if (!c->floating) {
        // get lua container
        lua_rawgeti(L, LUA_REGISTRYINDEX, containers_info.id);
        lua_rawgeti(L, -1, MIN(c->clientPosition+1, containers_info.n));
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
        if (!overlay)
            container_surround_gaps(&box, innerGap);
        resize(c, box.x, box.y, box.width, box.height, false);
        containers_info.id = luaL_ref(L, LUA_REGISTRYINDEX);
    }
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
            if (i < containers_info.n)
                c->hidden = false;
            else
                c->hidden = true;
            i++;
        }
    }
}

int tiled_client_count(struct monitor *m)
{
    struct client *c;
    int n = 0;

    wl_list_for_each(c, &clients, link)
        if (existon(c, m) && !c->floating)
            n++;
    return n;
}
