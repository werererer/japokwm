#include "tile/tileUtils.h"
#include <client.h>
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
#include "tile/tileTexture.h"
#include "utils/coreUtils.h"
#include "utils/gapUtils.h"
#include "utils/parseConfigUtils.h"

void arrange(enum layout_actions action)
{
    struct monitor *m;
    printf("start arrange\n");
    arrange_monitor(selected_monitor, action);
    wl_list_for_each(m, &mons, link) {
        if (m == selected_monitor)
            continue;
        arrange_monitor(m, action);
    }
    printf("end arrange\n");
}

/* update layout and was set in the arrange function */
static void update_layout(lua_State *L, int n, struct monitor *m)
{
    lua_getglobal(L, "Update_layout");
    lua_pushinteger(L, n);
    lua_call_safe(L, 1, 1, 0);
    m->ws->layout.n = lua_rawlen(L, -1);
    if (m->ws->layout.lua_index >= 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, m->ws->layout.lua_index);
    }
    m->ws->layout.lua_index = luaL_ref(L, LUA_REGISTRYINDEX);
}

static struct wlr_fbox lua_unbox_layout(struct lua_State *L, int i) {
    struct wlr_fbox geom;
    lua_rawgeti(L, -1, i);

    lua_rawgeti(L, -1, 1);
    geom.x = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 2);
    geom.y = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 3);
    geom.width = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 4);
    geom.height = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_pop(L, 1);
    return geom;
}

/* update layout and was set in the arrange function */
static void apply_nmaster_transformation(struct wlr_box *box, struct monitor *m, int position, int count)
{
    struct layout lt = m->ws->layout;

    if (position > lt.nmaster)
        return;

    lua_getglobal(L, "Update_nmaster");
    int g = count > lt.nmaster ? lt.nmaster : count;
    lua_pushinteger(L, g);
    lua_call_safe(L, 1, 1, 0);
    int k = MIN(position, g);
    struct wlr_fbox geom = lua_unbox_layout(L, k);
    lua_pop(L, 1);

    struct wlr_box obox = get_absolute_box(geom, *box);
    memcpy(box, &obox, sizeof(struct wlr_box));
}

int get_slave_container_count(struct monitor *m)
{
    struct layout lt = m->ws->layout;
    int abs_count = get_tiled_container_count(m);
    return MAX(abs_count - lt.nmaster, 0);
}

int get_master_container_count(struct monitor *m)
{
    int abs_count = get_tiled_container_count(m);
    int slave_container_count = get_slave_container_count(m);
    return MAX(abs_count - slave_container_count, 0);
}

// amount of slave containers plus the one master area
static int get_default_container_count(struct monitor *m)
{
    return get_slave_container_count(m) + 1;
}

void arrange_monitor(struct monitor *m, enum layout_actions action)
{
    /* Get effective monitor geometry to use for window area */
    m->geom = *wlr_output_layout_get_box(server.output_layout, m->wlr_output);
    set_root_area(m->root, m->geom);

    if (!overlay)
        container_surround_gaps(&m->root->geom, outer_gap);

    // don't do anything if no tiling function exist
    if (m->ws->layout.lua_func_index <= 0)
        return;

    int container_count = get_master_container_count(m);
    int default_container_count = get_default_container_count(m);
    update_layout(L, default_container_count, m);
    update_hidden_containers(m);

    int position = 1;
    struct container *con;
    wl_list_for_each(con, &containers, mlink) {
        if (!visibleon(con, m))
            continue;

        con->position = position;
        // then use the layout that may have been reseted
        arrange_container(con, container_count, false);
        position++;
    }
}

void arrange_container(struct container *con, int container_count, bool preserve)
{
    if (con->floating || con->hidden)
        return;

    struct monitor *m = con->m;
    struct layout lt = m->ws->layout;
    // the 1 added represents the master area
    int n = MAX(0, con->position - lt.nmaster) + 1;

    lua_rawgeti(L, LUA_REGISTRYINDEX, m->ws->layout.lua_index);
    struct wlr_fbox rel_geom = lua_unbox_layout(L, n);

    struct wlr_box box = get_absolute_box(rel_geom, m->root->geom);
    // TODO fix this function, hard to read
    apply_nmaster_transformation(&box, con->m, con->position, container_count);
    m->ws->layout.lua_index = luaL_ref(L, LUA_REGISTRYINDEX);
    printf("box.x: %i\n", box.x);
    printf("box.y: %i\n", box.y);
    printf("box.width: %i\n", box.width);
    printf("box.height: %i\n", box.height);

    if (!overlay)
        container_surround_gaps(&box, inner_gap);

    resize(con, box, preserve);
}

void resize(struct container *con, struct wlr_box geom, bool preserve)
{
    /*
     * Note that I took some shortcuts here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at
     * the new size, then commit any movement that was prepared.
     */
    container_damage_whole(con);
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
        apply_bounds(con, *wlr_output_layout_get_box(server.output_layout, NULL));

        /* wlroots makes this a no-op if size hasn't changed */
        switch (con->client->type) {
            case XDG_SHELL:
                wlr_xdg_toplevel_set_size(con->client->surface.xdg,
                        con->geom.width, con->geom.height);
                break;
            case LAYER_SHELL:
                wlr_layer_surface_v1_configure(con->client->surface.layer,
                        con->m->geom.width,
                        con->m->geom.height);
                break;
            case X11_MANAGED:
            case X11_UNMANAGED:
                wlr_xwayland_surface_configure(con->client->surface.xwayland,
                        con->geom.x, con->geom.y, con->geom.width,
                        con->geom.height);
        }
    }
}

void update_hidden_containers(struct monitor *m)
{
    int i = 1;
    struct container *con;
    // because the master are is included in n aswell as nmaster we have to
    // subtract the solution by one to count
    int count = m->ws->layout.n + m->ws->layout.nmaster-1;
    wl_list_for_each(con, &containers, mlink) {
        if (!existon(con, m) || con->floating)
            continue;

        con->hidden = i > count;
        i++;
    }
}

int get_tiled_container_count(struct monitor *m)
{
    struct container *con;
    int n = 0;

    wl_list_for_each(con, &containers, mlink)
        if(!con->floating && existon(con, m))
            n++;
    return n;
}
