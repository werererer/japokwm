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

#include "container.h"
#include "parseConfig.h"
#include "root.h"
#include "server.h"
#include "workspaceset.h"
#include "tile/tileTexture.h"
#include "utils/coreUtils.h"
#include "utils/gapUtils.h"
#include "utils/parseConfigUtils.h"

void arrange(enum layout_actions action)
{
    struct monitor *m;
    printf("start\n");
    arrange_monitor(selected_monitor, action);
    wl_list_for_each(m, &mons, link) {
        if (m == selected_monitor)
            continue;
        arrange_monitor(m, action);
    }
    printf("end\n");
}

void arrange_monitor(struct monitor *m, enum layout_actions action)
{
    /* Get effective monitor geometry to use for window area */
    m->geom = *wlr_output_layout_get_box(output_layout, m->wlr_output);
    set_root_area(m);

    if (!overlay)
        container_surround_gaps(&m->root->geom, outer_gap);

    // don't do anything if no tiling function exist
    if (selected_layout(m)->funcId <= 0)
        return;

    int n = tiled_container_count(m);
    /* call arrange function if previous layout is different or reset ->
     * reset layout */
    if (is_same_layout(prev_layout, *selected_layout(m))
            || action == LAYOUT_RESET) {
        prev_layout = *selected_layout(m);
        lua_rawgeti(L, LUA_REGISTRYINDEX, selected_layout(m)->funcId);
        lua_pushinteger(L, n);
        lua_pcall(L, 1, 0, 0);
    }

    /* update layout aquired from was set with the arrange function */
    lua_getglobal(L, "update");
    lua_pushinteger(L, n);
    lua_pcall(L, 1, 1, 0);
    selected_layout(m)->containers_info.n = lua_rawlen(L, -1);
    selected_layout(m)->containers_info.id = luaL_ref(L, LUA_REGISTRYINDEX);
    update_hidden_status(m);

    int i = 0;
    struct container *con;
    wl_list_for_each(con, &containers, mlink) {
        if (!visibleon(con, m))
            continue;
        arrange_container(m, con, i, false);
        con->textPosition = i;
        i++;
    }
    update_overlay();
}

void arrange_container(struct monitor *m, struct container *con, int i, bool preserve)
{
    con->clientPosition = i;
    if (con->floating || con->hidden)
        return;

    // if tiled get tile information from tile function and apply it
    struct wlr_fbox box;
    // get lua container
    lua_rawgeti(L, LUA_REGISTRYINDEX, selected_layout(con->m)->containers_info.id);
    lua_rawgeti(L, -1, MIN(con->clientPosition+1, selected_layout(con->m)->containers_info.n));
    lua_rawgeti(L, -1, 1);
    box.x = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 2);
    box.y = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 3);
    box.width = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 4);
    box.height = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_pop(L, 1);

    con->geom = get_absolute_box(m->root->geom, box);
    if (!overlay)
        container_surround_gaps(&con->geom, inner_gap);
    container_surround_gaps(&con->geom, 2*con->client->bw);
    resize(con, con->geom, preserve);
    selected_layout(con->m)->containers_info.id = luaL_ref(L, LUA_REGISTRYINDEX);
}

void resize(struct container *con, struct wlr_box geom, bool preserve)
{
    /*
     * Note that I took some shortcuts here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at
     * the new size, then commit any movement that was prepared.
     */
    con->geom = geom;
    if (preserve) {
        // if width <= height
        if (con->client->ratio >= 1) {
            con->geom.height = geom.height;
            con->geom.width = con->geom.height / con->client->ratio;
        } else {
            con->geom.width = geom.width;
            con->geom.height = geom.width * con->client->ratio;
        }
    } else {
        con->client->ratio = calc_ratio(con->geom.width, con->geom.height);
        applybounds(con, *wlr_output_layout_get_box(output_layout, NULL));

        /* wlroots makes this a no-op if size hasn't changed */
        switch (con->client->type) {
            case XDG_SHELL:
                con->resize = wlr_xdg_toplevel_set_size(con->client->surface.xdg,
                        con->geom.width, con->geom.height);
                break;
            case LAYER_SHELL:
                wlr_layer_surface_v1_configure(con->client->surface.layer,
                        selected_monitor->geom.width,
                        selected_monitor->geom.height);
                break;
            case X11_MANAGED:
            case X11_UNMANAGED:
                wlr_xwayland_surface_configure(con->client->surface.xwayland,
                        con->geom.x, con->geom.y,
                        con->geom.width, con->geom.height);
        }
    }
}

void update_hidden_status(struct monitor *m)
{
    int i = 0;
    struct container *con;
    wl_list_for_each(con, &containers, mlink) {
        if (!existon(con, m) || con->floating)
            continue;

        if (i < selected_layout(m)->containers_info.n) {
            con->hidden = false;
            i++;
        } else {
            con->hidden = true;
        }
    }
}

int tiled_container_count(struct monitor *m)
{
    struct container *con;
    int n = 0;

    wl_list_for_each(con, &containers, mlink)
        if(!con->floating && existon(con, m))
            n++;
    return n;
}
