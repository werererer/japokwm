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
#include "tileTexture.h"

static void arrange_container(struct container *con, int arrange_position,
        struct wlr_box root_geom, int inner_gap);

static void create_messages(struct monitor *m)
{
    wlr_list_clear(&render_data.textures, free);
    int y_offset = 0;
    for (int i = 0; i < server.messages.length; i++) {
        char *message = server.messages.items[i];

        float color[4] = {1.0, 0.0, 0.0, 1.0};
        float text_color[4] = {1.0, 1.0, 1.0, 1.0};
        struct wlr_box geom = {0, y_offset, m->geom.width, 100};
        struct pos_texture *ptexture =
            create_textbox(&geom, color, text_color, message);
        y_offset += geom.height;
        wlr_list_push(&render_data.textures, ptexture);
    }
}

void arrange()
{
    for (int i = 0; i < server.mons.length; i++) {
        struct monitor *m = server.mons.items[i];
        arrange_monitor(m);
    }

    create_messages(selected_monitor);

    update_cursor(&server.cursor);
}

static void set_layout_ref(struct layout *lt, int n_area)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);

    lua_rawgeti(L, -1, n_area);
    // TODO refactor
    int len = luaL_len(L, -1);
    n_area = MAX(MIN(len, n_area), 1);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_layout_ref);

    lua_pop(L, 1);
}

static int get_layout_container_area_count(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);

    int len = luaL_len(L, -1);
    int container_area_count = get_container_area_count(tagset);
    int n_area = MAX(MIN(len, container_area_count), 1);

    lua_pop(L, 1);
    return n_area;
}

static int get_layout_container_max_area_count(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);

    int len = luaL_len(L, -1);

    lua_rawgeti(L, -1, len);


    // TODO refactor
    int max_n_area = luaL_len(L, -1);

    lua_pop(L, 2);
    return max_n_area;
}

static void update_layout_counters(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    tagset->n_all = get_container_count(tagset);
    lt->n_area = get_layout_container_area_count(tagset);
    set_layout_ref(lt, lt->n_area);
    lt->n_area_max = get_layout_container_max_area_count(tagset);
    lt->n_master_abs = get_master_container_count(tagset);
    lt->n_floating = get_floating_container_count(tagset);
    lt->n_tiled = lt->n_area + lt->n_master_abs-1;
    lt->n_tiled_max = lt->n_area_max + lt->n_master_abs-1;
    lt->n_visible = lt->n_tiled + lt->n_floating;
    lt->n_hidden = tagset->n_all - lt->n_visible;
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
static void apply_nmaster_layout(struct wlr_box *box, struct layout *lt, int position)
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

static struct wlr_box get_nth_geom_in_layout(lua_State *L, struct layout *lt, 
        struct wlr_box root_geom, int arrange_position)
{
    // relative position
    int n = MAX(0, arrange_position+1 - lt->nmaster) + 1;

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_ref);
    struct wlr_fbox rel_geom = lua_unbox_layout_geom(L, n);
    lua_pop(L, 1);

    struct wlr_box box = get_absolute_box(rel_geom, root_geom);

    // TODO fix this function, hard to read
    apply_nmaster_layout(&box, lt, arrange_position+1);
    return box;
}

int get_slave_container_count(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);
    int abs_count = get_tiled_container_count(tagset);
    return MAX(abs_count - lt->nmaster, 0);
}

int get_floating_container_count(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    // there are no floating windows when using arrange by focus
    if (lt->options.arrange_by_focus)
        return 0;

    int n = 0;

    for (int i = 0; i < tagset->list_set.floating_containers.length; i++) {
        struct container *con = get_container(tagset, i);
        if (con->client->type == LAYER_SHELL)
            continue;
        n++;
    }
    return n;
}

int get_master_container_count(struct tagset *ts)
{
    int abs_count = get_tiled_container_count(ts);
    int slave_container_count = get_slave_container_count(ts);
    return MAX(abs_count - slave_container_count, 0);
}

// amount of slave containers plus the one master area
int get_container_area_count(struct tagset *ts)
{
    return get_slave_container_count(ts) + 1;
}

void arrange_monitor(struct monitor *m)
{
    m->geom = *wlr_output_layout_get_box(server.output_layout, m->wlr_output);
    set_root_geom(m->root, m->geom);

    struct tagset *tagset = monitor_get_active_tagset(m);
    struct layout *lt = tagset_get_layout(tagset);
    container_surround_gaps(&m->root->geom, lt->options.outer_gap);

    update_layout_counters(tagset);
    call_update_function(lt->options.event_handler, lt->n_area);

    struct wlr_list *visible_container_lists = get_visible_lists(&tagset->list_set);
    struct wlr_list *tiled_containers = get_tiled_list(&tagset->list_set);
    struct wlr_list *hidden_containers = get_hidden_list(&tagset->list_set); 

    update_hidden_status_of_containers(m, visible_container_lists,
            tiled_containers, hidden_containers);

    if (!lt->options.arrange_by_focus) {
        for (int i = 0; i < tagset->list_set.floating_containers.length; i++) {
            struct container *con = tagset->list_set.floating_containers.items[i];
            if (con->geom_was_changed) {
                resize(con, con->prev_floating_geom);
                con->geom_was_changed = false;
            }
        }
    }

    arrange_containers(tagset, m->root->geom, tiled_containers);

    root_damage_whole(m->root);
}

void arrange_containers(struct tagset *tagset, struct wlr_box root_geom,
        struct wlr_list *tiled_containers)
{
    struct layout *lt = tagset_get_layout(tagset);

    /* each container will get an inner_gap. If two containers are adjacent the
     * inner_gap is applied twice. To counter this effect we divide the
     * inner_gap by 2 */
    int actual_inner_gap = (int)lt->options.inner_gap/2;

    /* the root_geom must be reduced by the inner_gap to ensure that the
     * outer_gap stays unchanged when each container is surrounded by the
     * inner_gap. */
    container_surround_gaps(&root_geom, -actual_inner_gap);

    if (lt->options.smart_hidden_edges) {
        if (tiled_containers->length <= 1) {
            container_add_gaps(&root_geom, -lt->options.tile_border_px,
                    lt->options.hidden_edges);
        }
    } else {
        container_add_gaps(&root_geom, -lt->options.tile_border_px,
                lt->options.hidden_edges);
    }

    for (int i = 0; i < tiled_containers->length; i++) {
        struct container *con = tiled_containers->items[i];

        /* // the monitor must be on the same monitor as it is tiled on else it is */
        /* // a bug */
        /* assert(container_get_monitor(con) == tagset->m); */

        arrange_container(con, i, root_geom, actual_inner_gap);
    }
}

static void arrange_container(struct container *con, int arrange_position, 
        struct wlr_box root_geom, int inner_gap)
{
    if (con->hidden)
        return;

    struct monitor *m = container_get_monitor(con);
    struct layout *lt = get_layout_in_monitor(m);

    struct wlr_box geom = get_nth_geom_in_layout(L, lt, root_geom, arrange_position);
    container_surround_gaps(&geom, inner_gap);

    // since gaps are halfed we need to multiply it by 2
    container_surround_gaps(&geom, 2*con->client->bw);

    if (con->floating)
        con->geom_was_changed = true;

    resize(con, geom);
}

static void set_container_geom(struct container *con, struct wlr_box geom)
{
    struct monitor *m = container_get_monitor(con);
    struct layout *lt = get_layout_in_monitor(m);

    if (con->floating && !lt->options.arrange_by_focus)
        con->prev_floating_geom = geom;

    con->prev_geom = con->geom;
    con->geom = geom;
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
            if (con->client->surface.xdg->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
                wlr_xdg_toplevel_set_size(con->client->surface.xdg,
                        con->geom.width, con->geom.height);
            }
            break;
        case LAYER_SHELL:
            {
                struct monitor *m = container_get_monitor(con);
                wlr_layer_surface_v1_configure(con->client->surface.layer,
                        m->geom.width,
                        m->geom.height);
            }
            break;
        case X11_UNMANAGED:
        case X11_MANAGED:
            wlr_xwayland_surface_configure(con->client->surface.xwayland,
                    con->geom.x, con->geom.y, con->geom.width,
                    con->geom.height);
    }
}

void update_hidden_status_of_containers(struct monitor *m, 
        struct wlr_list *visible_container_lists, struct wlr_list *tiled_containers,
        struct wlr_list *hidden_containers)
{
    // because the master are is included in n aswell as nmaster we have to
    // subtract the solution by one to count
    struct layout *lt = get_layout_in_monitor(m);

    if (lt->n_tiled > tiled_containers->length) {
        int n_missing = MIN(lt->n_tiled - tiled_containers->length, hidden_containers->length);
        for (int i = 0; i < n_missing; i++) {
            struct container *con = hidden_containers->items[0];

            con->hidden = false;
            wlr_list_del(hidden_containers, 0);
            wlr_list_push(tiled_containers, con);
        }
    } else {
        int tile_containers_length = tiled_containers->length;
        for (int i = lt->n_tiled; i < tile_containers_length; i++) {
            struct container *con = wlr_list_pop(tiled_containers);
            con->hidden = true;
            wlr_list_insert(hidden_containers, 0, con);
        }
    }

    for (int i = 0; i < length_of_composed_list(visible_container_lists); i++) {
        struct container *con = get_in_composed_list(visible_container_lists, i);
        con->hidden = false;
    }
    for (int i = 0; i < hidden_containers->length; i++) {
        struct container *con = hidden_containers->items[i];
        con->hidden = true;
    }
}

int get_container_count(struct tagset *ts)
{
    return length_of_composed_list(&ts->list_set.container_lists);
}

int get_tiled_container_count(struct tagset *ts)
{
    int n = 0;
    struct wlr_list *tiled_containers = get_tiled_list(&ts->list_set);
    struct wlr_list *hidden_containers = get_hidden_list(&ts->list_set);

    n = tiled_containers->length + hidden_containers->length;
    return n;
}
