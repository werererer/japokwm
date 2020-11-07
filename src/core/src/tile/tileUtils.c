#include "tile/tileUtils.h"
#include <client.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <wayland-util.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_box.h>
#include <stdlib.h>

#include "monitor.h"
#include "tagset.h"
#include "utils/coreUtils.h"
#include "parseConfig.h"
#include "tile/tileTexture.h"
#include "utils/gapUtils.h"
#include "utils/parseConfigUtils.h"

struct containersInfo containersInfo;

/* *
 * the wlr_fbox has includes the window size in percent.
 * It will we mulitiplicated with the screen width and height
 * */
static struct wlr_box getAbsoluteBox(struct monitor *m, struct wlr_fbox b)
{
    struct wlr_box w = m->tagset.w;
    w.x = w.width * b.x + w.x;
    w.y = w.height * b.y + w.y;
    w.width = w.width * b.width;
    w.height = w.height * b.height;
    return w;
}

// TODO reduce function size
void arrange(struct monitor *m, bool reset)
{
    /* Get effective monitor geometry to use for window area */
    struct tagset *tagset = &m->tagset;
    m->m = *wlr_output_layout_get_box(output_layout, m->wlr_output);

    tagset->w = m->m;
    containerSurroundGaps(&tagset->w, outerGap);
    if (selLayout(tagset).funcId) {
        struct client *c = NULL;

        int n = tiledClientCount(m);
        /* call arrange function
         * if previous layout is different or reset -> reset layout */
        if (strcmp(prevLayout.symbol, selLayout(tagset).symbol) != 0 || reset) {
            /* prevLayout = selLayout(tagset); */
            /* lua_rawgeti(L, LUA_REGISTRYINDEX, selLayout(tagset).funcId); */
            /* lua_pushinteger(L, n); */
            /* lua_pcall(L, 1, 0, 0); */
        }
        lua_getglobal(L, "update");
        lua_pushinteger(L, n);
        lua_pcall(L, 1, 1, 0);
        containersInfo.n = lua_rawlen(L, -1);
        containersInfo.id = luaL_ref(L, LUA_REGISTRYINDEX);

        updateHiddenStatus();
        int i = 1;
        wl_list_for_each(c, &clients, link) {
            if (!visibleon(c, m) || c->floating)
                continue;
            struct wlr_fbox con;
            // get lua container
            // TODO: create function
            lua_rawgeti(L, LUA_REGISTRYINDEX, containersInfo.id);
            lua_rawgeti(L, -1, MIN(i, containersInfo.n));
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
            lua_pop(L, 1);
            printf("con: %f\n", con.x);
            printf("con: %f\n", con.y);
            printf("con: %f\n", con.width);
            printf("con: %f\n", con.height);

            struct wlr_box b = getAbsoluteBox(m, con);
            resize(c, b.x, b.y, b.width, b.height, false);
            i++;
        }

        if (overlay) {
            createNewOverlay();
        } else {
            wlr_list_clear(&renderData.textures);
        }

    }
}


void arrangeThis(bool reset)
{
    arrange(selMon, reset);
}

void resize(struct client *c, int x, int y, int w, int h, bool interact)
{
    /*
     * Note that I took some shortcuts here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at
     * the new size, then commit any movement that was prepared.
     */
    struct wlr_box box;
    box.x = x;
    box.y = y;
    box.width = w;
    box.height = h;

    containerSurroundGaps(&box, innerGap);
    c->geom = box;
    applybounds(c, box);

    /* wlroots makes this a no-op if size hasn't changed */
    switch (c->type) {
        case XDGShell:
            c->resize = wlr_xdg_toplevel_set_size(c->surface.xdg,
                    c->geom.width - 2 * c->bw, c->geom.height - 2 * c->bw);
            break;
        case LayerShell:
            wlr_layer_surface_v1_configure(c->surface.layer,
                    c->geom.width - 2 * c->bw, c->geom.height - 2 * c->bw);
            break;
        case X11Managed:
        case X11Unmanaged:
            wlr_xwayland_surface_configure(c->surface.xwayland,
                    c->geom.x, c->geom.y,
                    c->geom.width - 2 * c->bw, c->geom.height - 2 * c->bw);
    }
}

void updateHiddenStatus()
{
    struct client *c;
    int i = 0;
    wl_list_for_each(c, &clients, link) {
        if (existon(c, selMon))
        {
            if (i++ < containersInfo.n)
                c->hidden = false;
            else
                c->hidden = true;
        }
    }
}

void updateLayout()
{
    setSelLayout(&selMon->tagset, getConfigLayout(L, "layout"));
    arrange(selMon, true);
}

int thisTiledClientCount()
{
    return tiledClientCount(selMon);
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

int clientPos()
{
    struct monitor *m = selMon;
    struct client *c;
    int n = 0;

    wl_list_for_each(c, &clients, link) {
        if (visibleon(c, m) && !c->floating) {
            if (c == selClient())
                return n;
            n++;
        }
    }
    return 0;
}
