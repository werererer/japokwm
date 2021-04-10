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
#include "workspace.h"
#include "ipc-server.h"

static void add_container_to_workspace(struct container *con, struct workspace *ws);

struct container *create_container(struct client *c, struct monitor *m, bool has_border)
{
    struct container *con = calloc(1, sizeof(struct container));
    con->client = c;
    c->con = con;

    // must be done for the container damage event
    con->commit.notify = commit_notify;
    wl_signal_add(&get_wlrsurface(c)->events.commit, &con->commit);

    con->has_border = has_border;
    con->focusable = true;
    add_container_to_workspace(con, get_workspace(m->ws_id));

    struct workspace *ws = get_workspace_in_monitor(m);
    struct layout *lt = ws->layout;
    struct event_handler *ev = &lt->options.event_handler;

    int position = find_in_composed_list(&ws->container_lists, &cmp_ptr, con);
    call_create_container_function(ev, position);
    return con;
}

void destroy_container(struct container *con)
{
    // surfaces cant commit anything anymore if their container is destroyed
    wl_list_remove(&con->commit.link);

    struct workspace *ws = get_workspace_in_monitor(con->m);

    remove_in_composed_list(&ws->focus_stack_lists, cmp_ptr, con);

    switch (con->client->type) {
        case LAYER_SHELL:
            remove_in_composed_list(&server.layer_visual_stack_lists,
                    cmp_ptr, con);
            break;
        case X11_UNMANAGED:
            remove_in_composed_list(&server.normal_visual_stack_lists,
                    cmp_ptr, con);
            wlr_list_remove(&ws->independent_containers, cmp_ptr, con);
            break;
        default:
            remove_in_composed_list(&server.normal_visual_stack_lists,
                    cmp_ptr, con);
            remove_in_composed_list(&ws->container_lists, cmp_ptr, con);
            break;
    }

    free(con);
}

void container_damage_borders(struct container *con, struct wlr_box *geom)
{
    if (!con)
        return;
    if (!geom)
        return;

    struct monitor *m = con->m;
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
    for (int i = 0; i < server.mons.length; i++) {
        struct monitor *m = server.mons.items[i];
        damage_container_area(con, &con->geom, m, whole);
    }

    struct client *c = con->client;
    if (c->resized || c->moved_workspace) {
        for (int i = 0; i < server.mons.length; i++) {
            struct monitor *m = server.mons.items[i];
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

    struct workspace *ws = get_workspace(m->ws_id);

    if (!ws)
        return NULL;

    return get_in_composed_list(&ws->focus_stack_lists, 0);
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
        if (!visible_on(con, get_workspace(m->ws_id)))
            continue;
        if (!wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }

    return NULL;
}

void add_container_to_composed_list(struct wlr_list *lists, struct container *con, int i)
{
    if (!con)
        return;
    struct workspace *ws = get_workspace(con->m->ws_id);
    if (!ws)
        return;

    struct wlr_list *hidden_containers = get_hidden_list(ws);
    struct wlr_list *tiled_containers = get_tiled_list(ws);
    struct wlr_list *floating_containers = get_floating_list(ws);
    if (con->floating) {
        wlr_list_insert(floating_containers, i, con);
        return;
    }
    if (con->hidden) {
        wlr_list_insert(hidden_containers, i, con);
        return;
    }
    wlr_list_insert(tiled_containers, i, con);
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
            add_container_to_stack(con);
            if (con->client->surface.layer->current.layer
                    == ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND) {
                con->focusable = false;
            }
            break;
        case X11_UNMANAGED:
            wlr_list_insert(&ws->independent_containers, 0, con);
            add_container_to_stack(con);
            break;
        case XDG_SHELL:
        case X11_MANAGED:
            add_container_to_containers(con, ws, 0);
            add_container_to_stack(con);
            break;
    }
    add_container_to_focus_stack(con, ws);
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

    for (int i = 0; i < server.default_layout->options.rule_count; i++) {
        const struct rule r = server.default_layout->options.rules[i];
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
    struct container *sel = get_focused_container(m);

    if (a == FOCUS_LIFT)
        lift_container(con);

    /* Put the new client atop the focus stack */
    struct workspace *ws = get_workspace_in_monitor(m);
    remove_in_composed_list(&ws->focus_stack_lists, cmp_ptr, con);
    add_container_to_focus_stack(con, get_workspace(m->ws_id));

    struct container *new_sel = get_focused_container(m);

    container_damage_borders(sel, &sel->geom);
    container_damage_borders(new_sel, &new_sel->geom);

    struct client *old_c = sel ? sel->client : NULL;
    struct client *new_c = new_sel ? new_sel->client : NULL;
    focus_client(old_c, new_c);
}

void focus_on_stack(struct monitor *m, int i)
{
    struct container *sel = get_focused_container(m);

    if (!sel)
        return;

    struct workspace *ws = get_workspace(m->ws_id);
    struct wlr_list *visible_container_lists = get_visible_lists(ws);

    if (sel->client->type == LAYER_SHELL) {
        struct container *con = get_container(ws, 0);
        focus_container(con, FOCUS_NOOP);
        return;
    }

    int sel_index = find_in_composed_list(visible_container_lists, cmp_ptr, sel);
    struct container *con =
        get_relative_item_in_composed_list(visible_container_lists, sel_index, i);

    /* If only one client is visible on selMon, then c == sel */
    focus_container(con, FOCUS_LIFT);
    ipc_event_window();
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
    struct wlr_list *hidden_containers = get_hidden_list(ws);
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
    wlr_list_remove(hidden_containers, cmp_ptr, con);

    struct wlr_list *visible_container_lists = get_visible_lists(ws);
    struct wlr_list *focus_list = find_list_in_composed_list(
            visible_container_lists, cmp_ptr, sel);
    int sel_index = wlr_list_find(focus_list, cmp_ptr, sel);

    wlr_list_insert(focus_list, sel_index+1, con);
    wlr_list_del(focus_list, sel_index);

    if (i < 0) {
        wlr_list_insert(hidden_containers, 0, sel);
    } else {
        wlr_list_push(hidden_containers, sel);
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

    remove_in_composed_list(&server.normal_visual_stack_lists, cmp_ptr, con);
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

void fix_position(struct container *con)
{
    if (!con)
        return;

    struct workspace *ws = get_workspace_in_monitor(con->m);

    struct wlr_list *tiled_containers = get_tiled_list(ws);
    struct wlr_list *floating_containers = get_floating_list(ws);

    if (!con->floating) {
        wlr_list_remove(floating_containers, cmp_ptr, con);
        int position = MIN(tiled_containers->length, ws->layout->n_tiled_max-1);
        wlr_list_insert(tiled_containers, position, con);
    } else {
        wlr_list_remove(tiled_containers, cmp_ptr, con);
        wlr_list_insert(floating_containers, 0, con);
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

    struct monitor *m = con->m;
    struct workspace *ws = get_workspace_in_monitor(m);
    struct layout *lt = ws->layout;

    con->floating = floating;

    if (fix_position)
        fix_position(con);

    if (!con->floating) {
        set_container_workspace(con, ws);

        if (con->on_scratchpad) {
            remove_container_from_scratchpad(con);
        }

        wlr_list_remove(&server.floating_visual_stack, cmp_ptr, con);
        wlr_list_insert(&server.tiled_visual_stack, 0, con);
    } else {
        wlr_list_remove(&server.tiled_visual_stack, cmp_ptr, con);
        wlr_list_insert(&server.floating_visual_stack, 0, con);
    }

    lift_container(con);
    con->client->bw = lt->options.float_border_px;
    con->client->resized = true;
    container_damage_whole(con);
}

void set_container_hidden_status(struct container *con, bool b)
{
    con->hidden = true;
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

    struct workspace *ws = get_workspace(m->ws_id);
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
