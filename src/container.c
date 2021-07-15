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
#include "tagset.h"
#include "ipc-server.h"
#include "layer_shell.h"

static void add_container_to_workspace(struct container *con, struct workspace *ws);

struct container *create_container(struct client *c, struct monitor *m, bool has_border)
{
    printf("create_container\n");
    struct container *con = calloc(1, sizeof(struct container));
    con->client = c;
    c->con = con;

    con->alpha = 1.0f;
    con->has_border = has_border;
    con->focusable = true;
    con->client->ws_id = m->tagset->selected_ws_id;

    apply_rules(con);
    return con;
}

void destroy_container(struct container *con)
{
    free(con);
}

void add_container_to_tile(struct container *con)
{
    printf("add_container_to_tile\n");
    assert(!con->is_tiled);
    add_container_to_workspace(con, get_workspace(con->client->ws_id));

    struct monitor *m = container_get_monitor(con);
    struct layout *lt = get_layout_in_monitor(m);
    struct event_handler *ev = lt->options.event_handler;
    call_create_container_function(ev, get_position_in_container_stack(con));

    con->is_tiled = true;
}

void remove_container_from_tile(struct container *con)
{
    assert(con->is_tiled);
    if (con->on_scratchpad)
        remove_container_from_scratchpad(con);

    struct workspace *ws = get_workspace(con->client->ws_id);

    list_set_remove_container_from_focus_stack(ws->list_set, con);

    switch (con->client->type) {
        case LAYER_SHELL:
            remove_in_composed_list(server.layer_visual_stack_lists, cmp_ptr, con);
            break;
        case X11_UNMANAGED:
            remove_in_composed_list(server.normal_visual_stack_lists, cmp_ptr, con);
            list_set_remove_independent_container(ws->list_set, con);
            break;
        default:
            remove_in_composed_list(server.normal_visual_stack_lists, cmp_ptr, con);
            list_set_remove_container(ws->list_set, con);
            break;
    }

    con->is_tiled = false;
}

void container_damage_borders(struct container *con, struct wlr_box *geom)
{
    if (!con)
        return;
    if (!geom)
        return;

    struct monitor *m = container_get_monitor(con);
    int border_width = con->client->bw;
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
    container_damage_borders(con, geom);
}

static void container_damage(struct container *con, bool whole)
{
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        damage_container_area(con, &con->geom, m, whole);
    }

    struct client *c = con->client;
    if (c->resized || c->moved_workspace) {
        for (int i = 0; i < server.mons->len; i++) {
            struct monitor *m = g_ptr_array_index(server.mons, i);
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

struct container *get_focused_container(struct monitor *m)
{
    if (!m)
        return NULL;

    struct tagset *tagset = monitor_get_active_tagset(m);

    if (!tagset)
        return NULL;

    return get_in_composed_list(tagset->list_set->focus_stack_lists, 0);
}

struct container *xy_to_container(double x, double y)
{
    struct monitor *m = xy_to_monitor(x, y);
    if (!m)
        return NULL;

    for (int i = 0; i < length_of_composed_list(server.visual_stack_lists); i++) {
        struct container *con =
            get_in_composed_list(server.visual_stack_lists, i);
        if (!con->focusable)
            continue;
        if (!visible_on(monitor_get_active_tagset(m), con))
            continue;
        if (!wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }

    return NULL;
}

static void add_container_to_workspace(struct container *con, struct workspace *ws)
{
    if (!ws || !con)
        return;

    set_container_monitor(con, ws->m);
    switch (con->client->type) {
        case LAYER_SHELL:
            // layer shell programs aren't pushed to the stack because they use the
            // layer system to set the correct render position
            if (con->client->surface.layer->current.layer
                    == ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND) {
                con->focusable = false;
            }
            break;
        case X11_UNMANAGED:
            g_ptr_array_insert(ws->list_set->independent_containers, 0, con);
            add_container_to_stack(con);
            break;
        case XDG_SHELL:
        case X11_MANAGED:
            add_container_to_containers(ws->list_set, con, 0);
            add_container_to_stack(con);
            break;
    }
    list_set_add_container_to_focus_stack(ws->list_set, con);
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
    for (int i = 0; i < server.default_layout->options.rule_count; i++) {
        const struct rule r = server.default_layout->options.rules[i];
        bool same_id = strcmp(r.id, con->client->app_id) == 0;
        bool id_empty = strcmp(r.id, "") == 0;
        bool same_title = strcmp(r.title, con->client->title) == 0;
        bool title_empty = strcmp(r.title, "") == 0;
        if ((same_id || id_empty) && (same_title || title_empty)) {
            lua_geti(L, LUA_REGISTRYINDEX, r.lua_func_ref);
            struct monitor *m = container_get_monitor(con);
            struct workspace *ws = monitor_get_active_workspace(m);
            guint position;
            g_ptr_array_find(ws->list_set->container_lists, con, &position);
            lua_pushinteger(L, position);
            lua_call_safe(L, 1, 0, 0);
        }
    }
}

void commit_notify(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, commit);

    if (!c)
        return;

    struct container *con = c->con;
    if (con->is_tiled) {
        container_damage_part(c->con);
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

    struct monitor *m = container_get_monitor(con);
    struct tagset *tagset = monitor_get_active_tagset(m);

    if (!visible_on(tagset, con))
        return;

    struct container *sel = get_focused_container(m);

    if (a == FOCUS_LIFT)
        lift_container(con);

    /* Put the new client atop the focus stack */
    list_set_remove_container_from_focus_stack(tagset->list_set, con);
    list_set_add_container_to_focus_stack(tagset->list_set, con);

    struct container *new_sel = get_focused_container(m);

    ipc_event_window();

    container_damage_borders(sel, &sel->geom);
    container_damage_borders(new_sel, &new_sel->geom);

    struct layout *lt = tagset_get_layout(tagset);
    call_on_focus_function(lt->options.event_handler,
            get_position_in_container_stack(con));

    struct client *old_c = sel ? sel->client : NULL;
    struct client *new_c = new_sel ? new_sel->client : NULL;
    focus_client(old_c, new_c);
}

void focus_on_stack(struct monitor *m, int i)
{
    struct container *sel = get_focused_container(m);

    if (!sel)
        return;

    struct tagset *tagset = monitor_get_active_tagset(m);
    GPtrArray *visible_container_lists = tagset_get_visible_lists(tagset);

    if (sel->client->type == LAYER_SHELL) {
        struct container *con = get_container(tagset, 0);
        focus_container(con, FOCUS_NOOP);
        return;
    }

    int sel_index = find_in_composed_list(visible_container_lists, cmp_ptr, sel);
    struct container *con =
        get_relative_item_in_composed_list(visible_container_lists, sel_index, i);

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

    struct tagset *tagset = monitor_get_active_tagset(m);
    GPtrArray *hidden_containers = tagset_get_hidden_list(tagset);
    struct container *con = get_relative_item_in_list(hidden_containers, 0, i);

    if (!con)
        return;

    if (sel->floating) {
        set_container_floating(con, NULL, true);
        set_container_floating(sel, NULL, false);
        resize(con, sel->geom);
    }

    con->hidden = false;
    sel->hidden = true;

    /* replace selected container with a hidden one and move the selected
     * container to the end of the containers array */
    g_ptr_array_remove(hidden_containers, con);

    GPtrArray *visible_container_lists = tagset_get_visible_lists(tagset);
    GPtrArray *focus_list = find_list_in_composed_list(
            visible_container_lists, cmp_ptr, sel);
    guint sel_index;
    g_ptr_array_find(focus_list, sel, &sel_index);

    g_ptr_array_insert(focus_list, sel_index+1, con);
    g_ptr_array_remove_index(focus_list, sel_index);

    if (i < 0) {
        g_ptr_array_insert(hidden_containers, 0, sel);
    } else {
        g_ptr_array_add(hidden_containers, sel);
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

    remove_in_composed_list(server.normal_visual_stack_lists, cmp_ptr, con);
    add_container_to_stack(con);
}

void repush(int pos1, int pos2)
{
    /* pos1 > pos2 */
    struct monitor *m = selected_monitor;
    struct tagset *tagset = monitor_get_active_tagset(m);

    struct container *con = g_ptr_array_index(tagset->list_set->tiled_containers, pos1);

    if (!con)
        return;
    if (con->floating)
        return;

    GPtrArray *tiled_containers = tagset_get_tiled_list(tagset);
    g_ptr_array_remove(tiled_containers, con);
    g_ptr_array_insert(tiled_containers, pos2, con);

    arrange();

    struct layout *lt = get_layout_in_monitor(m);
    if (lt->options.arrange_by_focus)
        arrange();
}

void fix_position(struct container *con)
{
    if (!con)
        return;

    struct monitor *m = container_get_monitor(con);
    struct tagset *tagset = monitor_get_active_tagset(m);

    GPtrArray *tiled_containers = tagset_get_tiled_list(tagset);
    GPtrArray *floating_containers = tagset_get_floating_list(tagset);
    struct layout *lt = tagset_get_layout(tagset);

    if (!con->floating) {
        g_ptr_array_remove(floating_containers, con);
        int position = MIN(tiled_containers->len, lt->n_tiled_max-1);
        g_ptr_array_insert(tiled_containers, position, con);
    } else {
        g_ptr_array_remove(tiled_containers, con);
        g_ptr_array_insert(floating_containers, 0, con);
    }
}

void set_container_floating(struct container *con, void (*fix_position)(struct container *con), bool floating)
{
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;
    if (con->floating == floating)
        return;

    struct monitor *m = container_get_monitor(con);
    struct layout *lt = get_layout_in_monitor(m);
    struct workspace *ws = monitor_get_active_workspace(m);

    con->floating = floating;

    if (fix_position)
        fix_position(con);

    if (!con->floating) {
        set_container_workspace(con, ws);

        if (con->on_scratchpad) {
            remove_container_from_scratchpad(con);
        }

        g_ptr_array_remove(server.floating_visual_stack, con);
        g_ptr_array_insert(server.tiled_visual_stack, 0, con);
    } else {
        g_ptr_array_remove(server.tiled_visual_stack, con);
        g_ptr_array_insert(server.floating_visual_stack, 0, con);
    }

    lift_container(con);
    con->client->bw = lt->options.float_border_px;
    con->client->resized = true;
    container_damage_whole(con);
}

void set_container_hidden_status(struct container *con, bool b)
{
    con->hidden = b;
}

void set_container_monitor(struct container *con, struct monitor *m)
{
    assert(m != NULL);
    if (!con)
        return;
    if (con->client->m == m)
        return;

    if (con->client->type == LAYER_SHELL) {
        con->client->m = m;
    }

    struct workspace *ws = monitor_get_active_workspace(m);
    set_container_workspace(con, ws);
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

    geom.width = absolute_x_to_container_relative(con->geom, cursor->x - offsetx);
    geom.height = absolute_y_to_container_relative(con->geom, cursor->y - offsety);

    if (con->on_scratchpad) {
        remove_container_from_scratchpad(con);
    }
    resize(con, geom);
}

void add_container_to_containers(struct list_set *list_set, struct container *con, int i)
{
    assert(con != NULL);

    for (int j = 0; j < list_set->change_affected_list_sets->len; j++) {
        struct list_set *ls = g_ptr_array_index(list_set->change_affected_list_sets, j);
        if (con->floating) {
            g_ptr_array_insert(ls->floating_containers, i, con);
            continue;
        }
        if (con->hidden) {
            g_ptr_array_insert(ls->hidden_containers, i, con);
            continue;
        }
        g_ptr_array_insert(ls->tiled_containers, i, con);
    }
}

void list_set_add_container_to_focus_stack(struct list_set *list_set, struct container *con)
{
    for (int j = 0; j < list_set->change_affected_list_sets->len; j++) {
        struct list_set *ls = g_ptr_array_index(list_set->change_affected_list_sets, j);
        if (con->client->type == LAYER_SHELL) {
            GPtrArray *list = get_layer_list(con->client->surface.layer->current.layer);
            g_ptr_array_insert(list, 0, con);
            return;
        }
        if (con->on_top) {
            g_ptr_array_insert(ls->focus_stack_on_top, 0, con);
            return;
        }
        if (!con->focusable) {
            g_ptr_array_insert(ls->focus_stack_not_focusable, 0, con);
            return;
        }

        g_ptr_array_insert(ls->focus_stack_normal, 0, con);
    }
}

void add_container_to_stack(struct container *con)
{
    if (!con)
        return;

    if (con->client->type == LAYER_SHELL) {
        switch (con->client->surface.layer->current.layer) {
            case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
                g_ptr_array_insert(server.layer_visual_stack_background, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
                g_ptr_array_insert(server.layer_visual_stack_bottom, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
                g_ptr_array_insert(server.layer_visual_stack_top, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
                g_ptr_array_insert(server.layer_visual_stack_overlay, 0, con);
                break;
        }
        return;
    }

    if (con->floating) {
        g_ptr_array_insert(server.floating_visual_stack, 0, con);
        return;
    }

    g_ptr_array_insert(server.tiled_visual_stack, 0, con);
}

void set_container_workspace(struct container *con, struct workspace *ws)
{
    if (!con)
        return;
    if (!ws)
        return;
    struct monitor *m = container_get_monitor(con);
    struct tagset *tagset = monitor_get_active_tagset(m);
    if (tagset->selected_ws_id == ws->id)
        return;

    struct workspace *old_ws = get_workspace(con->client->ws_id);

    list_set_remove_container(old_ws->list_set, con);
    add_container_to_containers(ws->list_set, con, 0);

    list_set_remove_container_from_focus_stack(old_ws->list_set, con);
    list_set_add_container_to_focus_stack(ws->list_set, con);

    con->client->ws_id = ws->id;
    ws->m = old_ws->m;
    if (is_workspace_empty(old_ws)) {
        old_ws->m = NULL;
    }

    if (con->floating)
        con->client->bw = ws->layout->options.float_border_px;
    else
        con->client->bw = ws->layout->options.tile_border_px;
}

void move_container_to_workspace(struct container *con, struct workspace *ws)
{
    printf("move container to workspace\n");
    if (!ws)
        return;
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;

    set_container_workspace(con, ws);
    con->client->moved_workspace = true;
    container_damage_whole(con);

    arrange();
    struct monitor *m = container_get_monitor(con);
    struct tagset *selected_tagset = monitor_get_active_tagset(m);
    focus_most_recent_container(selected_tagset, FOCUS_NOOP);

    ipc_event_workspace();
}

struct monitor *container_get_monitor(struct container *con)
{
    if (!con)
        return NULL;
    if (con->client->m)
        return con->client->m;

    struct workspace *ws = get_workspace(con->client->ws_id);
    struct monitor *m  = ws->m;

    if (ws->tagset) {
        m = ws->tagset->m;
    }
    return m;
}

void list_set_remove_container(struct list_set *list_set, struct container *con)
{
    for (int i = 0; i < list_set->change_affected_list_sets->len; i++) {
        struct list_set *ls = g_ptr_array_index(list_set->change_affected_list_sets, i);
        remove_in_composed_list(ls->container_lists, cmp_ptr, con);
    }
}

void list_set_remove_container_from_focus_stack(struct list_set *list_set, struct container *con)
{
    for (int i = 0; i < list_set->change_affected_list_sets->len; i++) {
        struct list_set *ls = g_ptr_array_index(list_set->change_affected_list_sets, i);
        remove_in_composed_list(ls->focus_stack_lists, cmp_ptr, con);
    }
}

void list_set_remove_independent_container(struct list_set *list_set, struct container *con)
{
    for (int i = 0; i < list_set->change_affected_list_sets->len; i++) {
        struct list_set *ls = g_ptr_array_index(list_set->change_affected_list_sets, i);
        g_ptr_array_remove(ls->independent_containers, con);
    }
}

inline int absolute_x_to_container_relative(struct wlr_box geom, int x)
{
    return x - geom.x;
}

inline int absolute_y_to_container_relative(struct wlr_box geom, int y)
{
    return y - geom.y;
}

int get_position_in_container_stack(struct container *con)
{
    if (!con)
        return INVALID_POSITION;

    struct monitor *m = container_get_monitor(con);
    struct tagset *tagset = monitor_get_active_tagset(m);
    int position = find_in_composed_list(tagset->list_set->container_lists, cmp_ptr, con);
    return position;
}

struct container *get_container_from_container_stack_position(int i)
{
    struct monitor *m = selected_monitor;
    struct tagset *tagset = monitor_get_active_tagset(m);
    struct container *con = get_container(tagset, i);
    return con;
}

bool is_resize_not_in_limit(struct wlr_fbox *geom, struct resize_constraints *resize_constraints)
{
    bool is_width_not_in_limit = geom->width < resize_constraints->min_width ||
        geom->height < resize_constraints->min_height;

    bool is_height_not_in_limit = geom->width > resize_constraints->max_width ||
        geom->height > resize_constraints->max_height;


    return is_width_not_in_limit || is_height_not_in_limit;
}
