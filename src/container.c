#include "container.h"

#include <lua.h>
#include <stdlib.h>
#include <wayland-util.h>
#include <string.h>
#include <assert.h>

#include "client.h"
#include "server.h"
#include "monitor.h"
#include "tile/tileUtils.h"
#include "render/render.h"
#include "options.h"
#include "utils/parseConfigUtils.h"
#include "scratchpad.h"

static void add_container_to_monitor(struct container *con, struct monitor *m);
static void add_container_to_focus_stack(struct container *con, int ws_id);
static void add_container_to_stack(struct container *con);

struct container *create_container(struct client *c, struct monitor *m, bool has_border)
{
    struct container *con = calloc(1, sizeof(struct container));
    con->client = c;
    c->con = con;

    set_container_monitor(con, m);

    con->has_border = has_border;
    con->focusable = true;
    add_container_to_monitor(con, con->m);

    struct workspace *ws = get_workspace_in_monitor(m);
    struct layout *lt = ws->layout;
    struct event_handler *ev = &lt->options.event_handler;

    int position = wlr_list_find_in_composed_list(&ws->container_lists, &cmp_ptr, con);
    call_create_container_function(ev, position);
    return con;
}

void destroy_container(struct container *con)
{
    struct workspace *ws = get_workspace_in_monitor(con->m);

    wlr_list_remove_in_composed_list(&ws->focus_stack_lists, cmp_ptr, con);

    switch (con->client->type) {
        case LAYER_SHELL:
            wlr_list_remove_in_composed_list(&server.layer_visual_stack_lists,
                    cmp_ptr, con);
            break;
        case X11_UNMANAGED:
            wlr_list_remove_in_composed_list(&server.normal_visual_stack_lists,
                    cmp_ptr, con);
            wl_list_remove(&con->ilink);
            remove_container_from_stack(ws->id, con);
            break;
        default:
            wlr_list_remove_in_composed_list(&server.normal_visual_stack_lists,
                    cmp_ptr, con);
            remove_container_from_stack(ws->id, con);
            break;
    }

    free(con);
}

static void damage_border(struct monitor *m, struct wlr_box *geom, int border_width)
{
    double ox = geom->x - border_width;
    double oy = geom->y - border_width;
    wlr_output_layout_output_coords(server.output_layout, m->wlr_output, &ox, &oy);
    int w = geom->width;
    int h = geom->height;

    struct wlr_box *borders;
    borders = (struct wlr_box[4]) {
        {ox, oy, w + 2 * border_width, border_width},             /* top */
            {ox, oy + border_width, border_width, h},                 /* left */
            {ox + border_width + w, oy + border_width, border_width, h},     /* right */
            {ox, oy + border_width + h, w + 2 * border_width, border_width}, /* bottom */
    };

    for (int i = 0; i < 4; i++) {
        scale_box(&borders[i], m->wlr_output->scale);
        wlr_output_damage_add_box(m->damage, &borders[i]);
    }
}

static void damage_container_area(struct container *con, struct wlr_box *geom, 
        struct monitor *m, bool whole)
{
    output_damage_surface(m, get_wlrsurface(con->client), geom, whole);
    damage_border(m, geom, con->client->bw);
}

static void container_damage(struct container *con, bool whole)
{
    struct monitor *m;
    wl_list_for_each(m, &mons, link) {
        damage_container_area(con, &con->geom, m, whole);
    }

    struct client *c = con->client;
    if (c->resized || c->moved_workspace) {
        struct monitor *m;
        wl_list_for_each(m, &mons, link) {
            damage_container_area(con, &con->prev_geom, m, whole);
        }
        c->resized = false;
        c->moved_workspace = false;
    }
}

void container_damage_part(struct container *con)
{
    container_damage(con, false);
}

void container_damage_whole(struct container *con)
{
    container_damage(con, true);
}

struct container *container_position_to_hidden_container(int ws_id, int position)
{
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);
    if (position >= ws->hidden_containers.length)
        return NULL;
    return ws->hidden_containers.items[position];
}

struct container *container_position_to_hidden_container_in_focus_stack(int ws_id, int position)
{
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);

    return get_in_composed_list(&ws->focus_stack_lists, position);
}

struct container *get_container(int ws_id, int i)
{
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);

    if (!ws)
        return NULL;

    return get_in_composed_list(&ws->container_lists, i);
}

struct container *get_visible_container(int ws_id, int i)
{
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);

    if (!ws)
        return NULL;

    return get_in_composed_list(&ws->visible_container_lists, i);
}

struct container *get_hidden_container(int ws_id, int i)
{
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);

    if (!ws)
        return NULL;

    return ws->hidden_containers.items[i];
}

struct container *get_focused_container(struct monitor *m)
{
    assert(m != NULL);

    return get_container_on_focus_stack(m->ws_ids[0], 0);
}

struct container *get_container_on_focus_stack(int ws_id, int i)
{
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);

    if (!ws)
        return NULL;

    return get_in_composed_list(&ws->focus_stack_lists, i);
}

struct container *get_relative_focus_container(int ws_id, struct container *con, int i)
{
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);
    struct layout *lt = ws->layout;

    int position = wlr_list_find_in_composed_list(&ws->focus_stack_lists, cmp_ptr, con);
    int new_position = (position + i) % (lt->n_visible);
    while (new_position < 0) {
        new_position += lt->n_visible;
    }

    return get_container_on_focus_stack(ws_id, new_position);
}

struct container *get_relative_visible_container(int ws_id, struct container *con, int i)
{
    assert(con != NULL);

    struct workspace *ws = get_workspace(&server.workspaces, ws_id);

    if (!ws)
        return NULL;

    struct layout *lt = get_layout_on_workspace(ws->id);

    int position = wlr_list_find_in_composed_list(&ws->visible_container_lists,
            cmp_ptr, con);
    int new_position = (position + i) % (lt->n_visible);
    while (new_position < 0) {
        new_position += lt->n_visible;
    }
    printf("new_position: %i\n", new_position);

    return get_visible_container(ws_id, new_position);
}

struct container *get_relative_hidden_container(int ws_id, int i)
{
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);

    return get_relative_item_in_list(&ws->hidden_containers, 0, i);
}

struct container *get_relative_hidden_container_in_focus_stack(int ws_id, int i)
{
    struct layout *lt = get_layout_on_workspace(ws_id);
    int n_hidden_containers = lt->n_hidden;

    if (n_hidden_containers == 0)
        return NULL;

    int new_position = (i) % (n_hidden_containers);
    while (new_position < 0)
        new_position += n_hidden_containers;
    new_position += lt->n_visible;

    struct container *con = 
        container_position_to_hidden_container_in_focus_stack(ws_id, new_position);
    return con;
}

struct container *xy_to_container(double x, double y)
{
    struct monitor *m = xy_to_monitor(x, y);
    if (!m)
        return NULL;

    for (int i = 0; i < length_of_composed_list(&server.visual_stack_lists); i++) {
        struct container *con =
            get_in_composed_list(&server.visual_stack_lists, i);
        if (!con->focusable)
            continue;
        if (!visible_on(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (!wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }

    return NULL;
}

void remove_container_from_stack(int ws_id, struct container *con)
{
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);

    if (!ws)
        return;

    wlr_list_remove_in_composed_list(&ws->container_lists, cmp_ptr, con);
}

void remove_container_from_focus_stack(struct container *con)
{
    struct workspace *ws = get_workspace_in_monitor(con->m);

    if (!ws)
        return;

    wlr_list_remove_in_composed_list(&ws->focus_stack_lists, cmp_ptr, con);
}

void add_container_to_containers(struct container *con, int ws_id, int i)
{
    if (!con)
        return;
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);
    if (!ws)
        return;

    if (con->floating) {
        wlr_list_insert(&ws->floating_containers, i, con);
        return;
    }
    if (con->hidden) {
        wlr_list_insert(&ws->hidden_containers, i, con);
        return;
    }
    wlr_list_insert(&ws->tiled_containers, i, con);
}

static void add_container_to_focus_stack(struct container *con, int ws_id)
{
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);

    if (con->on_top) {
        wlr_list_insert(&ws->focus_stack_on_top, 0, con);
        return;
    }
    if (!con->focusable) {
        wlr_list_insert(&ws->focus_stack_not_focusable, 0, con);
        return;
    }

    wlr_list_insert(&ws->focus_stack_normal, 0, con);
}

static void add_container_to_stack(struct container *con)
{
    if (!con)
        return;

    if (con->client->type == LAYER_SHELL) {
        switch (con->client->surface.layer->current.layer) {
            case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
                wlr_list_insert(&server.layer_visual_stack_background, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
                wlr_list_insert(&server.layer_visual_stack_bottom, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
                wlr_list_insert(&server.layer_visual_stack_top, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
                wlr_list_insert(&server.layer_visual_stack_overlay, 0, con);
                break;
        }
        return;
    }

    if (con->floating) {
        wlr_list_insert(&server.floating_visual_stack, 0, con);
        return;
    }

    wlr_list_insert(&server.tiled_visual_stack, 0, con);
}

static void add_container_to_monitor(struct container *con, struct monitor *m)
{
    if (!m || !con)
        return;

    set_container_monitor(con, m);
    switch (con->client->type) {
        case LAYER_SHELL:
            // layer shell programs aren't pushed to the stack because they use the
            // layer system to set the correct render position
            add_container_to_stack(con);
            break;
        case XDG_SHELL:
        case X11_MANAGED:
        case X11_UNMANAGED:
            add_container_to_containers(con, con->m->ws_ids[0], 0);
            add_container_to_stack(con);
            break;
    }

    if (con->client->type == LAYER_SHELL)
        con->focusable = con->client->surface.layer->current.layer != ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
    add_container_to_focus_stack(con, m->ws_ids[0]);
}

struct wlr_box get_center_box(struct wlr_box ref)
{
    return (struct wlr_box) {
            .x = ref.width/4,
            .y = ref.height/4,
            .width = ref.width/2,
            .height = ref.height/2,
        };
}

struct wlr_box get_centered_box(struct wlr_box box, struct wlr_box ref)
{
    return (struct wlr_box) {
            .x = (ref.width-box.width)/2 + ref.x,
            .y = (ref.height-box.height)/2 + ref.y,
            .width = box.width,
            .height = box.height,
        };
}

struct wlr_box get_absolute_box(struct wlr_fbox ref, struct wlr_box box)
{
    struct wlr_box b;
    b.x = ref.x * box.width + box.x;
    b.y = ref.y * box.height + box.y;
    b.width = box.width * ref.width;
    b.height = box.height * ref.height;
    return b;
}

struct wlr_fbox get_relative_box(struct wlr_box box, struct wlr_box ref)
{
    struct wlr_fbox b;
    b.x = (float)box.x / ref.width;
    b.y = (float)box.y / ref.height;
    b.width = (float)box.width / ref.width;
    b.height = (float)box.height / ref.height;
    return b;
}

struct wlr_box get_monitor_local_box(struct wlr_box box, struct monitor *m)
{
    double x = box.x;
    double y = box.y;
    wlr_output_layout_output_coords(server.output_layout, m->wlr_output, &x, &y);
    box.x = x;
    box.y = y;
    return box;
}

struct wlr_fbox lua_togeometry(lua_State *L)
{
    struct wlr_fbox geom;
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
    return geom;
}


void apply_bounds(struct container *con, struct wlr_box box)
{
    /* set minimum possible */
    con->geom.width = MAX(MIN_CONTAINER_WIDTH, con->geom.width);
    con->geom.height = MAX(MIN_CONTAINER_HEIGHT, con->geom.height);

    if (con->geom.x >= box.x + box.width)
        con->geom.x = box.x + box.width - con->geom.width;
    if (con->geom.y >= box.y + box.height)
        con->geom.y = box.y + box.height - con->geom.height;
    if (con->geom.x + con->geom.width + 2 * con->client->bw <= box.x)
        con->geom.x = box.x;
    if (con->geom.y + con->geom.height + 2 * con->client->bw <= box.y)
        con->geom.y = box.y;
}

void apply_rules(struct container *con)
{
    const char *app_id, *title;
    /* rule matching */
    switch (con->client->type) {
        case XDG_SHELL:
            app_id = con->client->surface.xdg->toplevel->app_id;
            title = con->client->surface.xdg->toplevel->title;
            title = con->client->surface.xdg->toplevel->title;
            break;
        case LAYER_SHELL:
            app_id = "test";
            title = "test";
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            app_id = con->client->surface.xwayland->class;
            title = con->client->surface.xwayland->title;
            break;
    }
    if (!app_id)
        app_id = "broken";
    if (!title)
        title = "broken";

    for (int i = 0; i < server.default_layout.options.rule_count; i++) {
        const struct rule r = server.default_layout.options.rules[i];
        bool same_id = strcmp(r.id, app_id) == 0;
        bool id_empty = strcmp(r.id, "") == 0;
        bool same_title = strcmp(r.title, title) == 0;
        bool title_empty = strcmp(r.title, "") == 0;
        if ((same_id || id_empty) && (same_title || title_empty)) {
            lua_geti(L, LUA_REGISTRYINDEX, r.lua_func_ref);
            struct monitor *m = con->m;
            struct workspace *ws = get_workspace_in_monitor(m);
            int position = wlr_list_find(&ws->container_lists, cmp_ptr, con);
            lua_pushinteger(L, position);
            lua_call_safe(L, 1, 0, 0);
        }
    }
}

void focus_container(struct container *con, enum focus_actions a)
{
    if (!con)
        return;
    if (!con->focusable)
        return;
    if (con->hidden)
        return;

    struct monitor *m = con->m;
    struct container *fcon = get_focused_container(m);

    if (a == FOCUS_LIFT)
        lift_container(con);

    /* Put the new client atop the focus stack */
    struct workspace *ws = get_workspace_in_monitor(m);
    wlr_list_remove_in_composed_list(&ws->focus_stack_lists, cmp_ptr, con);
    add_container_to_focus_stack(con, m->ws_ids[0]);

    struct container *new_focus_con = get_focused_container(m);

    struct client *old_c = fcon ? fcon->client : NULL;
    struct client *new_c = new_focus_con ? new_focus_con->client : NULL;
    focus_client(old_c, new_c);
}

void focus_most_recent_container(int ws_id, enum focus_actions a)
{
    struct container *con = get_container_on_focus_stack(ws_id, 0);

    if (!con) {
        con = get_container(ws_id, 0);
        if (!con)
            return;
    }

    focus_container(con, a);
}

void focus_on_stack(struct monitor *m, int i)
{
    struct container *sel = get_focused_container(m);

    if (!sel)
        return;
    if (sel->client->type == LAYER_SHELL) {
        struct container *con = get_container(m->ws_ids[0], 0);
        focus_container(con, FOCUS_NOOP);
        return;
    }

    struct container *con = get_relative_visible_container(m->ws_ids[0], sel, i);
    if (!con)
        return;

    /* If only one client is visible on selMon, then c == sel */
    focus_container(con, FOCUS_LIFT);
}

// TODO refactor
void focus_on_hidden_stack(struct monitor *m, int i)
{
    struct container *sel = get_focused_container(m);

    if (!sel)
        return;
    if (sel->client->type == LAYER_SHELL)
        return;

    struct workspace *ws = get_workspace_in_monitor(m);
    struct container *con = get_relative_hidden_container(m->ws_ids[0], i);

    if (!con)
        return;

    if (sel->floating) {
        con->floating = true;
        sel->floating = false;
        /* set_container_floating(con, true); */
        /* set_container_floating(sel, false); */
        set_container_geom(con, sel->geom);


        if (!con->floating) {
            int position = MIN(ws->tiled_containers.length, ws->layout[0].n_tiled_max-1);
            wlr_list_remove(&server.floating_visual_stack, cmp_ptr, con);
            wlr_list_insert(&server.tiled_visual_stack, position, con);
        } else {
            wlr_list_remove(&server.tiled_visual_stack, cmp_ptr, con);
            wlr_list_insert(&server.floating_visual_stack, 0, con);
        }

        if (!sel->floating) {
            int position = MIN(ws->tiled_containers.length, ws->layout[0].n_tiled_max-1);
            wlr_list_remove(&server.floating_visual_stack, cmp_ptr,sel);
            wlr_list_insert(&server.tiled_visual_stack, position,sel);
        } else {
            wlr_list_remove(&server.tiled_visual_stack, cmp_ptr,sel);
            wlr_list_insert(&server.floating_visual_stack, 0,sel);
        }
    }

    con->hidden = false;
    sel->hidden = true;

    /* replace selected container with a hidden one and move the selected
     * container to the end of the containers array */
    wlr_list_remove(&ws->hidden_containers, cmp_ptr, con);

    struct wlr_list *focus_list = wlr_list_find_list_in_composed_list(
            &ws->visible_container_lists, cmp_ptr, sel);
    int focus_position = wlr_list_find(focus_list, cmp_ptr, sel);

    wlr_list_insert(focus_list, focus_position, con);

    wlr_list_remove(focus_list, cmp_ptr, sel);

    if (i < 0) {
        wlr_list_insert(&ws->hidden_containers, 0, sel);
    } else {
        wlr_list_push(&ws->hidden_containers, sel);
    }

    if (!con)
        return;

    focus_container(con, FOCUS_NOOP);
    arrange();
}

void lift_container(struct container *con)
{
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;

    wlr_list_remove_in_composed_list(&server.normal_visual_stack_lists, cmp_ptr, con);
    add_container_to_stack(con);
}

void repush(int pos1, int pos2)
{
    /* pos1 > pos2 */
    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace_in_monitor(m);

    struct container *con = ws->tiled_containers.items[pos1];

    if (!con)
        return;
    if (con->floating)
        return;

    wlr_list_remove(&ws->tiled_containers, cmp_ptr, con);
    wlr_list_insert(&ws->tiled_containers, pos2, con);

    arrange();

    struct layout *lt = get_layout_in_monitor(m);
    if (lt->options.arrange_by_focus)
        arrange();
}

void set_container_floating(struct container *con, bool floating)
{
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;
    if (con->floating == floating)
        return;

    struct monitor *m = con->m;
    struct workspace *ws = get_workspace_in_monitor(m);
    struct layout *lt = ws->layout;

    con->floating = floating;

    if (!con->floating) {
        set_container_workspace(con, m->ws_ids[0]);

        wlr_list_remove(&ws->floating_containers, cmp_ptr, con);

        int position = MIN(ws->tiled_containers.length, ws->layout[0].n_tiled_max-1);
        printf("position: %i\n", position);
        wlr_list_insert(&ws->tiled_containers, position, con);

        if (con->on_scratchpad) {
            remove_container_from_scratchpad(con);
        }

        wlr_list_remove(&server.floating_visual_stack, cmp_ptr, con);
        wlr_list_insert(&server.tiled_visual_stack, position, con);
    } else {
        wlr_list_remove(&ws->tiled_containers, cmp_ptr, con);
        wlr_list_insert(&ws->floating_containers, 0, con);
        wlr_list_remove(&server.tiled_visual_stack, cmp_ptr, con);
        wlr_list_insert(&server.floating_visual_stack, 0, con);
    }

    lift_container(con);
    con->client->bw = lt->options.float_border_px;
    con->client->resized = true;
    container_damage_whole(con);
}

void set_container_geom(struct container *con, struct wlr_box geom)
{
    struct monitor *m = con->m;
    struct layout *lt = get_layout_in_monitor(m);

    if (con->floating && !lt->options.arrange_by_focus)
        con->prev_floating_geom = geom;

    con->prev_geom = con->geom;
    con->geom = geom;
}

void set_container_workspace(struct container *con, int ws_id)
{
    if (!con)
        return;
    struct workspace *ws = get_workspace(&server.workspaces, ws_id);
    if (!ws)
        return;
    if (con->m->ws_ids[0] == ws_id)
        return;

    if (ws->m == NULL)
        ws->m = con->m;
    else
        set_container_monitor(con, ws->m);
    con->client->ws_id = ws_id;

    remove_container_from_containers(con);
    add_container_to_containers(con, ws_id, 0);

    remove_container_from_focus_stack(con);
    add_container_to_focus_stack(con, ws_id);

    focus_most_recent_container(con->m->ws_ids[0], FOCUS_NOOP);

    if (con->floating)
        con->client->bw = ws->layout->options.float_border_px;
    else 
        con->client->bw = ws->layout->options.tile_border_px;
}

void set_container_monitor(struct container *con, struct monitor *m)
{
    assert(m != NULL);
    if (!con)
        return;
    if (con->m == m)
        return;

    if (con->prev_m != m)
        con->prev_m = con->m;
    con->m = m;

    /* ensure that prev_m is not = NULL after this function finished
    successfully */
    if (con->prev_m == NULL)
        con->prev_m = m;

    set_container_workspace(con, m->ws_ids[0]);
}

static void swap_booleans(bool *b1, bool *b2)
{
    bool b = *b2;
    *b2 = *b1;
    *b1 = b;
}

static void swap_integers(int *i1, int *i2)
{
    int b = *i2;
    *i2 = *i1;
    *i1 = b;
}

void swap_container_properties(struct container *con1, struct container *con2)
{
    if (con1->floating != con2->floating) {
        if (con1->floating)
            set_container_geom(con2, con1->geom);
        if (con2->floating)
            set_container_geom(con1, con2->geom);

        swap_booleans(&con1->floating, &con2->floating);
    }

    swap_booleans(&con1->hidden, &con2->hidden);
}

void swap_container_positions(struct container *con1, struct container *con2)
{
    assert(con1->m == con2->m);

    struct workspace *ws = get_workspace_in_monitor(con1->m);
    int position2 = wlr_list_find_in_composed_list(&ws->container_lists, cmp_ptr, con2);

    wlr_list_remove_in_composed_list(&ws->container_lists, cmp_ptr, con2);
    int position1 = wlr_list_find_in_composed_list(&ws->container_lists, cmp_ptr, con1);
    add_container_to_containers(con2, con1->m->ws_ids[0], position1+1);

    wlr_list_del(&ws->tiled_containers, position1);
    add_container_to_containers(con1, con1->m->ws_ids[0], position2);

    swap_container_properties(con1, con2);
}

void move_container(struct container *con, struct wlr_cursor *cursor, int offsetx, int offsety)
{
    struct wlr_box geom = con->geom;
    geom.x = cursor->x - offsetx;
    geom.y = cursor->y - offsety;

    if (con->on_scratchpad) {
        remove_container_from_scratchpad(con);
    }
    resize(con, geom);
    container_damage(con, true);
}

void resize_container(struct container *con, struct wlr_cursor *cursor, int offsetx, int offsety)
{
    struct wlr_box geom = con->geom;

    geom.width = absolute_x_to_container_relative(con, cursor->x - offsetx);
    geom.height = absolute_y_to_container_relative(con, cursor->y - offsety);

    if (con->on_scratchpad) {
        remove_container_from_scratchpad(con);
    }
    resize(con, geom);
}

inline int container_relative_x_to_absolute(struct container *con, int lx)
{
    return con->geom.x + lx;
}

inline int container_relative_y_to_absolute(struct container *con, int ly)
{
    return con->geom.y + ly;
}

inline int absolute_x_to_container_relative(struct container *con, int x)
{
    return x - con->geom.x;
}

inline int absolute_y_to_container_relative(struct container *con, int y)
{
    return y - con->geom.y;
}

bool is_resize_not_in_limit(struct wlr_fbox *geom, struct resize_constraints *resize_constraints)
{
    bool is_width_not_in_limit = geom->width < resize_constraints->min_width ||
        geom->height < resize_constraints->min_height;

    bool is_height_not_in_limit = geom->width > resize_constraints->max_width ||
        geom->height > resize_constraints->max_height;


    return is_width_not_in_limit || is_height_not_in_limit;
}
