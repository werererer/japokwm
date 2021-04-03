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
static void add_container_to_focus_stack(struct container *con);
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

    struct layout *lt = get_layout_in_monitor(con->m);
    struct event_handler *ev = &lt->options.event_handler;
    call_create_container_function(ev, con->position);
    return con;
}

void destroy_container(struct container *con)
{
    wl_list_remove(&con->flink);

    switch (con->client->type) {
        case LAYER_SHELL:
            wl_list_remove(&con->llink);
            break;
        case X11_UNMANAGED:
            wl_list_remove(&con->slink);
            wl_list_remove(&con->ilink);
            wl_list_remove(&con->mlink);
            break;
        default:
            wl_list_remove(&con->slink);
            wl_list_remove(&con->mlink);
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

struct container *container_position_to_container(int ws_id, int position)
{
    struct container *con;
    wl_list_for_each(con, &containers, mlink) {
        if (!exist_on(con, &server.workspaces, ws_id))
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;
        if (con->position == position)
            return con;
    }
    return NULL;
}

struct container *container_position_to_hidden_container(int ws_id, int position)
{
    struct container *con;
    wl_list_for_each(con, &containers, mlink) {
        if (!hidden_on(con, &server.workspaces, ws_id))
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;

        if (con->position == position)
            return con;
    }
    return NULL;
}

struct container *container_position_to_hidden_container_in_focus_stack(int ws_id, int position)
{
    struct container *con;
    wl_list_for_each(con, &focus_stack, flink) {
        if (!hidden_on(con, &server.workspaces, ws_id))
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;
        if (con->focus_position == INVALID_POSITION)
            continue;

        if (con->focus_position == position)
            return con;
    }
    return NULL;
}


struct container *get_focused_container(struct monitor *m)
{
    assert(m != NULL);

    return container_focus_position_to_container(m->ws_ids[0], 0);
}

struct container *get_container(int i)
{
    if (abs(i) > wl_list_length(&containers))
        return NULL;

    struct container *con;
    if (i >= 0) {
        struct wl_list *pos = &containers;
        while (i >= 0) {
            if (pos->next)
                pos = pos->next;
            i--;
        }
        con = wl_container_of(pos, con, mlink);
    } else { // i < 0
        struct wl_list *pos = &containers;
        while (i < 0) {
            pos = pos->prev;
            i++;
        }
        con = wl_container_of(pos, con, mlink);
    }
    return con;
}

struct container *container_focus_position_to_container(int ws_id, int position)
{
    struct container *con;
    wl_list_for_each(con, &focus_stack, flink) {
        if (con->client->type == LAYER_SHELL)
            continue;
        if (con->focus_position == position)
            return con;
    }
    return NULL;
}

struct container *get_relative_focus_container(int ws_id, struct container *con, int i)
{
    struct layout *lt = get_layout_on_workspace(ws_id);

    int new_position = (con->focus_position + i) % (lt->n_visible);
    while (new_position < 0) {
        new_position += lt->n_visible;
    }

    return container_focus_position_to_container(ws_id, new_position);
}

struct container *get_relative_container(int ws_id, struct container *con, int i)
{
    assert(con != NULL);

    struct layout *lt = get_layout_on_workspace(ws_id);

    int new_position = (con->position + i) % (lt->n_visible);
    while (new_position < 0) {
        new_position += lt->n_visible;
    }

    return container_position_to_container(ws_id, new_position);
}

struct container *get_relative_hidden_container(int ws_id, int i)
{
    struct layout *lt = get_layout_on_workspace(ws_id);
    int n_hidden_containers = lt->n_hidden;

    if (n_hidden_containers == 0)
        return NULL;

    int new_position = (i) % (n_hidden_containers);
    while (new_position < 0)
        new_position += n_hidden_containers;
    new_position += lt->n_visible;

    struct container *con = container_position_to_hidden_container(ws_id, new_position);
    return con;
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

    struct container *con;
    wl_list_for_each(con, &layer_stack, llink) {
        if (!con->focusable)
            continue;
        if (con->client->surface.layer->current.layer >
                ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM)
            continue;
        if (!visible_on(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (!wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }

    wl_list_for_each(con, &stack, slink) {
        if (!con->focusable)
            continue;
        if (!visible_on(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (!wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }

    wl_list_for_each(con, &layer_stack, llink) {
        if (!con->focusable)
            continue;
        if (con->client->surface.layer->current.layer <
                ZWLR_LAYER_SHELL_V1_LAYER_TOP)
            continue;
        if (!visible_on(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (!wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }
    return NULL;
}

void add_container_to_containers(struct container *con, int i)
{
    if (!con)
        return;

    /* Insert container at position i and push back container at i*/
    if (i == 0) {
        wl_list_insert(&containers, &con->mlink);
    } else {
        struct container *con2 = get_container(i-1);
        if (!con2) {
            wl_list_insert(&containers, &con->mlink);
        }
        wl_list_insert(&con2->mlink, &con->mlink);
    }
}

static void add_container_to_focus_stack(struct container *con)
{
    if (con->on_top) {
        wl_list_insert(&focus_stack, &con->flink);
        return;
    }
    if (!con->focusable) {
        wl_list_insert(focus_stack.prev, &con->flink);
        return;
    }
    if (wl_list_empty(&focus_stack)) {
        wl_list_insert(&focus_stack, &con->flink);
        return;
    }

    /* find the topmost container that is not on top. If found insert before it
       so that con becomes the new topmost container. If not found all other
       containers are on top. Therefore c is the last item and and con needs to
       be appended*/
    struct container *c;
    bool found = false;
    wl_list_for_each(c, &focus_stack, flink) {
        if (!c->on_top) {
            found = true;
            break;
        }
    }
    if (found)
        wl_list_insert(c->flink.prev, &con->flink);
    else
        wl_list_insert(c->flink.next, &con->flink);
}

static void add_container_to_stack(struct container *con)
{
    if (!con)
        return;

    if (con->floating) {
        wl_list_insert(&stack, &con->slink);
        return;
    }

    if (wl_list_empty(&stack)) {
        wl_list_insert(&stack, &con->slink);
        return;
    }

    /* Insert container after the last floating container */
    struct container *con2;
    int stack_length = wl_list_length(&stack);
    int i = 0;
    wl_list_for_each(con2, &stack, slink) {
        if (!con2->floating)
            break;
        i++;
        // needs to break early because wl_list_for_each will mess up con2 if
        // it continues after reaching the last item in stack
        if (i >= stack_length)
            break;
    }

    wl_list_insert(&con2->slink, &con->slink);
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
            wl_list_insert(&layer_stack, &con->llink);
            break;
        case XDG_SHELL:
        case X11_MANAGED:
        case X11_UNMANAGED:
            add_container_to_containers(con, 0);
            add_container_to_stack(con);
            break;
    }

    if (con->client->type == LAYER_SHELL)
        con->focusable = con->client->surface.layer->current.layer != ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
    add_container_to_focus_stack(con);
}

struct wlr_box get_center_box(struct wlr_box ref)
{
    return (struct wlr_box) {
            .x = ref.width/4,
            .y = ref.height/4,
            .width = ref.width/2,
            .height = ref.height/2
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
            lua_pushinteger(L, con->position);
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
    wl_list_remove(&con->flink);
    add_container_to_focus_stack(con);
    update_hidden_status_of_containers(m);
    update_container_focus_positions(m);

    struct container *new_focus_con = get_focused_container(m);

    struct client *old_c = fcon ? fcon->client : NULL;
    struct client *new_c = new_focus_con ? new_focus_con->client : NULL;
    focus_client(old_c, new_c);
}

void focus_most_recent_container(int ws_id, enum focus_actions a)
{
    struct container *con = container_focus_position_to_container(ws_id, 0);

    if (!con) {
        con = container_position_to_container(ws_id, 0);
        if (!con)
            return;
    }

    focus_container(con, a);
}

// TODO refactor this (a bit too messy)
static struct container *focus_on_hidden_stack_if_arrange_normally(int i)
{
    struct monitor *m = selected_monitor;
    struct container *sel = get_focused_container(m);

    if (!sel)
        return NULL;
    if (sel->client->type == LAYER_SHELL)
        return NULL;

    struct layout *lt = get_layout_in_monitor(m);

    struct container *con = get_relative_hidden_container(m->ws_ids[0], i);

    if (!con)
        return NULL;

    if (sel->floating) {
        set_container_floating(con, true);
        set_container_floating(sel, false);
        set_container_geom(con, sel->geom);
    }

    if (i >= 0) {
        /* replace selected container with a hidden one and move the selected
         * container to the end of containers */
        wl_list_remove(&con->mlink);
        wl_list_insert(&sel->mlink, &con->mlink);
        con->hidden = false;

        wl_list_remove(&sel->mlink);
        wl_list_insert(containers.prev, &sel->mlink);
        sel->hidden = true;
    } else if (i < 0) {
        struct container *last = container_position_to_container(m->ws_ids[0], lt->n_visible-1);
        wl_list_remove(&con->mlink);
        wl_list_insert(&sel->mlink, &con->mlink);

        /* replace current container with a hidden one and move the selected
         * container to the first position that is not visible */

        if (last == sel) {
            /* if the selected container is the last visible container it will
             * be placed after con because it will be the last container because
             * it replaces the position of the selected one */
            last = con;
        }

        if (!last) {
            return NULL;
        }

        wl_list_remove(&sel->mlink);
        wl_list_insert(&last->mlink, &sel->mlink);
        con->hidden = false;
        sel->hidden = true;
    }

    return con;
}

static void focus_on_stack_normally(int i)
{
    struct monitor *m = selected_monitor;
    struct container *sel = get_focused_container(m);

    if (!sel)
        return;
    if (sel->client->type == LAYER_SHELL) {
        struct container *con = container_position_to_container(m->ws_ids[0], 0);
        focus_container(con, FOCUS_NOOP);
        return;
    }

    struct container *con = get_relative_container(m->ws_ids[0], sel, i);
    if (!con)
        return;

    /* If only one client is visible on selMon, then c == sel */
    focus_container(con, FOCUS_LIFT);
}

static void focus_on_stack_if_arrange_by_focus(int i)
{
    struct monitor *m = selected_monitor;
    struct container *sel = get_focused_container(m);

    if (!sel)
        return;
    if (sel->client->type == LAYER_SHELL) {
        struct container *con = container_focus_position_to_container(m->ws_ids[0], 0);
        focus_container(con, FOCUS_NOOP);
        return;
    }

    struct container *con = get_relative_focus_container(m->ws_ids[0], sel, i);
    if (!con)
        return;

    /* If only one client is visible on selMon, then c == sel */
    focus_container(con, FOCUS_LIFT);
    arrange();
}

void focus_on_stack(int i)
{
    struct monitor *m = selected_monitor;
    struct layout *lt = get_layout_in_monitor(m);

    if (lt->options.arrange_by_focus)
        focus_on_stack_normally(i);
    else
        focus_on_stack_normally(i);
}

void focus_on_hidden_stack(int i)
{
    struct monitor *m = selected_monitor;
    struct container *sel = get_focused_container(m);

    if (!sel)
        return;
    if (sel->client->type == LAYER_SHELL)
        return;

    struct layout *lt = get_layout_in_monitor(m);

    struct container *con = get_relative_hidden_container(m->ws_ids[0], i);

    if (!con)
        return;

    if (sel->floating) {
        set_container_floating(con, true);
        set_container_floating(sel, false);
        set_container_geom(con, sel->geom);
    }

    struct container *last = container_position_to_container(m->ws_ids[0], lt->n_visible-1);
    wl_list_remove(&con->mlink);
    wl_list_insert(&sel->mlink, &con->mlink);
    con->hidden = false;
    sel->hidden = true;
    if (i >= 0) {
        /* replace selected container with a hidden one and move the selected
         * container to the end of the containers array */
        wl_list_remove(&sel->mlink);
        wl_list_insert(containers.prev, &sel->mlink);
    } else if (i < 0) {

        /* replace current container with a hidden one and move the selected
         * container to the first position that is not visible */
        if (last == sel) {
            /* if the selected container is the last visible container it will
             * be placed after con because it will be the last container because
             * it replaces the position of the selected one */
            last = con;
        }

        if (!last) {
            return;
        }

        wl_list_remove(&sel->mlink);
        wl_list_insert(&last->mlink, &sel->mlink);
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

    wl_list_remove(&con->slink);
    add_container_to_stack(con);
}

void repush(int pos1, int pos2)
{
    struct monitor *m = selected_monitor;

    struct container *con1 = container_position_to_container(m->ws_ids[0], pos2);

    if (!con1)
        return;
    if (con1->floating)
        return;

    struct container *master = wl_container_of(containers.next, master, mlink);

    struct container *con2 = container_position_to_container(selected_monitor->ws_ids[0], pos1);

    if (!con2)
        return;
    if (con2 == con1)
        return;

    wl_list_remove(&con2->mlink);
    wl_list_insert(&con1->mlink, &con2->mlink);
    wl_list_remove(&con1->mlink);
    wl_list_insert(&con2->mlink, &con1->mlink);

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
    struct layout *lt = get_layout_in_monitor(m);

    con->floating = floating;

    if (!con->floating) {
        set_container_workspace(con, m->ws_ids[0]);
        if (con->on_scratchpad) {
            remove_container_from_scratchpad(con);
        }
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

    if (ws->m == NULL)
        ws->m = con->m;
    set_container_monitor(con, ws->m);
    con->client->ws_id = ws_id;

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
    wl_list_remove(&con2->mlink);
    add_container_to_containers(con2, con1->position);

    wl_list_remove(&con1->mlink);
    add_container_to_containers(con1, con2->position);

    swap_container_properties(con1, con2);
}

void swap_container_focus_positions(struct container *con1, struct container *con2)
{
    int focus_stack_position1 = con1->focus_position;
    con1->focus_position = con2->focus_position;
    con2->focus_position = focus_stack_position1;
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
