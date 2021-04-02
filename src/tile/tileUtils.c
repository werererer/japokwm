#include "tile/tileUtils.h"
#include <client.h>
#include <assert.h>
#include <execinfo.h>
#include <string.h>
#include <sys/param.h>
#include <wayland-util.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <stdlib.h>
#include <wlr/util/log.h>

#include "container.h"
#include "monitor.h"
#include "root.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "utils/gapUtils.h"
#include "utils/parseConfigUtils.h"
#include "event_handler.h"

static void arrange_container(struct container *con, int arrange_position,
        struct wlr_box root_geom, int inner_gap);

void arrange()
{
    struct monitor *m;
    wl_list_for_each(m, &mons, link) {
        arrange_monitor(m);
    }

    update_cursor(&server.cursor);
}

static int get_all_container_count(struct workspace *ws)
{
    int n_all = 0;
    struct container *con;
    wl_list_for_each(con, &focus_stack, flink) {
        if (con->focus_position == INVALID_POSITION)
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;

        n_all++;
    }
    return n_all;
}

static int get_layout_container_area_count(struct workspace *ws)
{
    struct layout *lt = &ws->layout[0];
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);

    int len = luaL_len(L, -1);
    int container_area_count = get_container_area_count(ws);
    int n_area = MAX(MIN(len, container_area_count), 1);

    lua_rawgeti(L, -1, n_area);

    // TODO refactor
    len = luaL_len(L, -1);
    n_area = MAX(MIN(len, n_area), 1);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_layout_ref);
    lua_pop(L, 1);
    return n_area;
}

static void update_layout_counters(struct layout *lt)
{
    struct workspace *ws = get_workspace(&server.workspaces, lt->ws_id);

    lt->n_all = get_all_container_count(ws);
    lt->n_area = get_layout_container_area_count(ws);
    lt->n_master_abs = get_master_container_count(ws);
    lt->n_floating = get_floating_container_count(ws);
    lt->n_tiled = lt->n_area + lt->n_master_abs-1;
    lt->n_visible = lt->n_tiled + lt->n_floating;
    lt->n_hidden = lt->n_all - lt->n_visible;
}

static struct wlr_fbox lua_unbox_layout_geom(lua_State *L, int i) {
    struct wlr_fbox geom;

    if (luaL_len(L, -1) < i)
        wlr_log(WLR_ERROR, "index to high: index %i len %lli", i, luaL_len(L, -1));

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
static void apply_nmaster_transformation(struct wlr_box *box, struct layout *lt, int position)
{
    if (position > lt->nmaster)
        return;

    // get layout
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_master_layout_data_ref);
    int len = luaL_len(L, -1);
    int g = MIN(lt->n_master_abs, lt->nmaster);
    g = MAX(MIN(len, g), 1);
    lua_rawgeti(L, -1, g);
    int k = MIN(position, g);
    struct wlr_fbox geom = lua_unbox_layout_geom(L, k);
    lua_pop(L, 1);
    lua_pop(L, 1);

    struct wlr_box obox = get_absolute_box(geom, *box);
    memcpy(box, &obox, sizeof(struct wlr_box));
}

static struct wlr_box get_geom_in_layout(lua_State *L, struct layout *lt, struct wlr_box geom, int arrange_position)
{
    // relative position
    int n = MAX(0, arrange_position+1 - lt->nmaster) + 1;

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_ref);
    struct wlr_fbox rel_geom = lua_unbox_layout_geom(L, n);
    lua_pop(L, 1);

    struct wlr_box box = get_absolute_box(rel_geom, geom);

    // TODO fix this function, hard to read
    apply_nmaster_transformation(&box, lt, arrange_position+1);
    return box;
}

int get_slave_container_count(struct workspace *ws)
{
    struct layout *lt = &ws->layout[0];
    int abs_count = get_tiled_container_count(ws);
    return MAX(abs_count - lt->nmaster, 0);
}

int get_floating_container_count(struct workspace *ws)
{
    struct layout *lt = &ws->layout[0];

    // there are no floating windows when using arrange by focus
    if (lt->options.arrange_by_focus)
        return 0;

    int n = 0;
    struct container *con;
    wl_list_for_each(con, &containers, mlink) {
        if (!con->floating)
            continue;
        if (!existon(con, &server.workspaces, ws->id))
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;
        n++;
    }
    return n;
}

int get_master_container_count(struct workspace *ws)
{
    int abs_count = get_tiled_container_count(ws);
    int slave_container_count = get_slave_container_count(ws);
    return MAX(abs_count - slave_container_count, 0);
}

// amount of slave containers plus the one master area
int get_container_area_count(struct workspace *ws)
{
    return get_slave_container_count(ws) + 1;
}

static void update_container_positions_if_arranged_normally(struct monitor *m)
{
    struct container *con;

    wl_list_for_each(con, &containers, mlink) {
        con->position = INVALID_POSITION;
    }

    int position = 0;
    wl_list_for_each(con, &containers, mlink) {
        if (!visibleon(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (con->floating)
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;

        con->position = position;
        wl_list_remove(&con->mlink);
        add_container_to_containers(con, position);

        // then use the layout that may have been reseted
        position++;
    }

    wl_list_for_each(con, &containers, mlink) {
        if (!visibleon(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (!con->floating)
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;

        con->position = position;
        wl_list_remove(&con->mlink);
        add_container_to_containers(con, position);

        // then use the layout that may have been reseted
        position++;
    }

    wl_list_for_each(con, &containers, mlink) {
        if (!hiddenon(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (con->floating)
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;

        con->position = position;
        wl_list_remove(&con->mlink);
        add_container_to_containers(con, position);

        // then use the layout that may have been reseted
        position++;
    }
}

static void update_container_positions_if_arranged_by_focus(struct monitor *m)
{
    struct container *con;
    wl_list_for_each(con, &containers, mlink) {
        con->position = con->focus_position;
    }
}

void update_container_positions(struct monitor *m)
{
    struct layout *lt = get_layout_in_monitor(m);
    if (lt->options.arrange_by_focus) {
        update_container_positions_if_arranged_by_focus(m);
    } else {
        update_container_positions_if_arranged_normally(m);
    }
}

void update_container_focus_positions(struct monitor *m)
{
    int position = 0;
    struct container *con;

    wl_list_for_each(con, &focus_stack, flink) {
        con->focus_position = INVALID_POSITION;
    }

    wl_list_for_each(con, &focus_stack, flink) {
        if (!existon(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;

        con->focus_position = position;
        // then use the layout that may have been reseted
        position++;
    }
}

void arrange_monitor(struct monitor *m)
{
    set_root_area(m->root, m->geom);

    struct layout *lt = get_layout_in_monitor(m);
    container_surround_gaps(&m->root->geom, lt->options.outer_gap);

    update_layout_counters(lt);
    call_update_function(&lt->options.event_handler, lt->n_area);

    update_hidden_status_of_containers(m);

    update_container_focus_positions(m);
    update_container_positions(m);

    if (!lt->options.arrange_by_focus) {
        for (int i = lt->n_tiled; i < lt->n_visible; i++) {
            struct container *con = container_position_to_container(m->ws_ids[0], i);
            if (!con)
                break;
            assert(con->floating);
            if (con->geom_was_changed) {
                set_container_geom(con, con->prev_floating_geom);
                con->geom_was_changed = false;
            }
        }
    }

    arrange_containers(m->ws_ids[0], m->root->geom);

    root_damage_whole(m->root);
}

void arrange_containers(int ws_id, struct wlr_box root_geom)
{
    struct layout *lt = get_layout_on_workspace(ws_id);

    /* each container will get an inner_gap. If two containers are adjacent the
     * inner_gap is applied twice. To counter this effect we divide the
     * inner_gap by 2 */
    int actual_inner_gap = (int)lt->options.inner_gap/2;

    /* the root_geom must be reduced by the inner_gap to ensure that the
     * outer_gap stays unchanged when each container is surrounded by the
     * inner_gap. */
    container_surround_gaps(&root_geom, -actual_inner_gap);

    if (lt->options.smart_hidden_edges) {
        if (wl_list_length(&containers) <= 1) {
            container_add_gaps(&root_geom, -lt->options.tile_border_px,
                    lt->options.hidden_edges);
        }
    } else {
        container_add_gaps(&root_geom, -lt->options.tile_border_px, lt->options.hidden_edges);
    }

    struct container *con;
    if (lt->options.arrange_by_focus) {
        int i = 0;
        wl_list_for_each(con, &focus_stack, flink) {
            if (con->focus_position == INVALID_POSITION)
                continue;

            arrange_container(con, con->focus_position, root_geom, actual_inner_gap);
            i++;
        }
    } else {
        int i = 0;
        wl_list_for_each(con, &containers, mlink) {
            if (con->position == INVALID_POSITION)
                continue;
            if (con->floating)
                continue;

            arrange_container(con, con->position, root_geom, actual_inner_gap);
            i++;
        }
    }
}

static void arrange_container(struct container *con, int arrange_position, 
        struct wlr_box root_geom, int inner_gap)
{
    if (con->hidden)
        return;

    struct monitor *m = con->m;
    struct workspace *ws = get_workspace_on_monitor(m);
    struct layout *lt = &ws->layout[0];

    struct wlr_box geom = get_geom_in_layout(L, lt, root_geom, arrange_position);
    container_surround_gaps(&geom, inner_gap);

    // since gaps are halfed we need to multiply it by 2
    container_surround_gaps(&geom, 2*con->client->bw);

    if (con->floating)
        con->geom_was_changed = true;

    resize(con, geom);
}

void resize(struct container *con, struct wlr_box geom)
{
    /*
     * Note that I took some shortcuts here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at
     * the new size, then commit any movement that was prepared.
     */

    set_container_geom(con, geom);
    con->client->resized = true;

    bool preserve_ratio = con->ratio != 0;

    if (preserve_ratio) {
        /* calculated biggest container where con->geom.width and
         * con->geom.height = con->geom.width * con->ratio is inside geom.width
         * and geom.height
         * */
        float max_height = geom.height/con->ratio;
        con->geom.width = MIN(geom.width, max_height);
        con->geom.height = con->geom.width * con->ratio;
        // TODO make a function out of that 
        // center in x direction
        con->geom.x += (geom.width - con->geom.width)/2;
        // center in y direction
        con->geom.y += (geom.height - con->geom.height)/2;
    }

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
        case X11_UNMANAGED:
        case X11_MANAGED:
            wlr_xwayland_surface_configure(con->client->surface.xwayland,
                    con->geom.x, con->geom.y, con->geom.width,
                    con->geom.height);
    }
}

void update_hidden_status_of_containers(struct monitor *m)
{
    struct container *con;
    struct workspace *ws = get_workspace_on_monitor(m);
    // because the master are is included in n aswell as nmaster we have to
    // subtract the solution by one to count
    struct layout *lt = &ws->layout[0];

    int i = 0;
    if (ws->layout[0].options.arrange_by_focus) {
        wl_list_for_each(con, &focus_stack, flink) {
            if (con->focus_position == INVALID_POSITION)
                continue;
            if (con->client->type == LAYER_SHELL)
                continue;

            con->hidden = i + 1 > lt->n_tiled;
            i++;
        }
    } else {
        wl_list_for_each(con, &containers, mlink) {
            if (!existon(con, &server.workspaces, ws->id))
                continue;
            if (con->client->type == LAYER_SHELL)
                continue;
            if (con->floating) {
                // all floating windows are visible when arranging normally be
                // aware that even floating containers may be hidden which is
                // why we unhide them here.
                con->hidden = false;
                continue;
            }

            con->hidden = i + 1 > lt->n_tiled;
            i++;
        }
    }
}

static int get_tiled_container_count_if_arranged_by_focus(struct workspace *ws)
{
    struct container *con;
    int n = 0;

    wl_list_for_each(con, &containers, mlink) {
        if (!existon(con, &server.workspaces, ws->id))
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;
        n++;
    }
    return n;
}

static int get_tiled_container_count_if_arranged_normally(struct workspace *ws)
{
    struct container *con;
    int n = 0;

    wl_list_for_each(con, &containers, mlink) {
        if (con->floating)
            continue;
        if (!existon(con, &server.workspaces, ws->id))
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;
        n++;
    }
    return n;
}

int get_tiled_container_count(struct workspace *ws)
{
    int n = 0;
    if (ws->layout[0].options.arrange_by_focus) {
        n = get_tiled_container_count_if_arranged_by_focus(ws);
    } else {
        n = get_tiled_container_count_if_arranged_normally(ws);
    }
    return n;
}
