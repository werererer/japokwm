#include "container.h"

#include <lua.h>
#include <stdlib.h>
#include <wayland-util.h>
#include <string.h>

#include "client.h"
#include "server.h"
#include "monitor.h"
#include "tile/tileUtils.h"
#include "render/render.h"
#include "options.h"
#include "utils/parseConfigUtils.h"

static void add_container_to_monitor(struct container *con, struct monitor *m);
static void add_container_to_focus_stack(struct container *con);
static void add_container_to_stack(struct container *con);

struct container *create_container(struct client *c, struct monitor *m, bool has_border)
{
    struct container *con = calloc(1, sizeof(struct container));
    con->m = m;
    con->client = c;
    con->has_border = has_border;
    con->focusable = true;
    c->con = con;
    add_container_to_monitor(con, con->m);

    struct layout *lt = get_layout_on_monitor(con->m);
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

static void con_damage(struct container *con, struct wlr_box *geom, bool whole)
{
    struct monitor *m = con->m;

    output_damage_surface(m, get_wlrsurface(con->client), geom, whole);

    damage_border(m, geom, con->client->bw);
}

static void container_damage(struct container *con, bool whole)
{
    con_damage(con, &con->geom, whole);

    struct client *c = con->client;
    if (c->resize) {
        con_damage(con, &con->prev_geom, whole);
        c->resize = false;
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
    // TODO debug
    struct container *con;
    wl_list_for_each(con, &containers, mlink) {
        if (!visibleon(con, &server.workspaces, ws_id))
            continue;
        if (con->floating)
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
        if (!hiddenon(con, &server.workspaces, ws_id))
            continue;
        if (con->floating)
            continue;

        if (con->position == position)
            return con;
    }
    return NULL;
}

struct container *focused_container(struct monitor *m)
{
    if (wl_list_empty(&focus_stack))
        return NULL;

    struct container *con = wl_container_of(focus_stack.next, con, flink);

    if (!visibleon(con, &server.workspaces, m->ws_ids[0]))
        return NULL;

    return con;
}

struct container *next_container(struct monitor *m)
{
    if (wl_list_length(&focus_stack) >= 2)
    {
        struct container *con =
            wl_container_of(focus_stack.next->next, con, flink);
        if (!visibleon(con, &server.workspaces, m->ws_ids[0]))
            return NULL;
        else
            return con;
    } else {
        return NULL;
    }
}

struct container *get_container(struct monitor *m, int i)
{
    struct container *con;

    if (abs(i) > wl_list_length(&containers))
        return NULL;
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

struct container *get_container_on_focus_stack(struct monitor *m, int i)
{
    struct container *con;

    if (abs(i) > wl_list_length(&focus_stack))
        return NULL;
    if (i >= 0) {
        struct wl_list *pos = &focus_stack;
        while (i >= 0) {
            if (pos->next)
                pos = pos->next;
            i--;
        }
        con = wl_container_of(pos, con, flink);
    } else { // i < 0
        struct wl_list *pos = &focus_stack;
        while (i < 0) {
            pos = pos->prev;
            i++;
        }
        con = wl_container_of(pos, con, flink);
    }
    return con;
}

struct container *get_relative_container(struct monitor *m, struct container *con, int i)
{
    struct layout *lt = get_layout_on_monitor(m);

    printf("n_abs: %i\n", lt->n_abs);
    int new_position = (con->position + i) % (lt->n_abs);
    while (new_position < 0) {
        new_position += lt->n_abs;
    }

    return container_position_to_container(m->ws_ids[0], new_position);
}

struct container *get_relative_hidden_container(struct monitor *m, int i)
{
    struct layout *lt = get_layout_on_monitor(m);
    int n_hidden_containers = wl_list_length(&containers) - lt->n_abs;

    if (n_hidden_containers == 0) {
        printf("n_hidden_containers == 0\n");
        printf("numbers: length: %i, n_abs: %i", wl_list_length(&containers), lt->n_abs);
        return NULL;
    }

    int new_position = (i) % (n_hidden_containers);
    while (new_position < 0) {
        new_position += n_hidden_containers;
    }
    new_position += lt->n_abs;

    struct container *con = container_position_to_hidden_container(m->ws_ids[0], new_position);
    return con;
}

struct container *xytocontainer(double x, double y)
{
    struct monitor *m = xytomon(x, y);
    if (!m)
        return NULL;

    struct container *con;
    wl_list_for_each(con, &layer_stack, llink) {
        if (!con->focusable)
            continue;
        if (con->client->surface.layer->current.layer >
                ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM)
            continue;
        if (!visibleon(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (!wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }

    wl_list_for_each(con, &stack, slink) {
        if (!con->focusable)
            continue;
        if (!visibleon(con, &server.workspaces, m->ws_ids[0]))
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
        if (!visibleon(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (!wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }
    return NULL;
}

void add_container_to_containers(struct container *con, int i)
{
    struct monitor *m = con->m;
    if (!con)
        return;

    if (!con->floating) {
        /* Insert container at position i and push back container at i*/
        struct container *con2 = get_container(m, i);
        if (con2) {
            wl_list_insert(&con2->mlink, &con->mlink);
            wl_list_remove(&con2->mlink);
            wl_list_insert(&con->mlink, &con2->mlink);
            con->position = con2->stack_position;
        } else {
            wl_list_insert(&containers, &con->mlink);
            con->position = i;
        }
    } else {
        /* Insert container after the last non floating container */
        struct container *con2;
        wl_list_for_each_reverse(con2, &containers, mlink) {
            if (!con2->floating)
                break;
        }

        if (wl_list_empty(&containers)) {
            wl_list_insert(containers.prev, &con->mlink);
            con->position = wl_list_length(containers.prev)-1;
        } else {
            wl_list_insert(&con2->mlink, &con->mlink);
            con->position = con2->position+1;
        }
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

    con->m = m;
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
    struct container *fcon = focused_container(m);

    if (a == FOCUS_LIFT)
        lift_container(con);

    /* Put the new client atop the focus stack */
    wl_list_remove(&con->flink);
    add_container_to_focus_stack(con);

    struct container *new_focus_con = focused_container(m);

    struct client *old_c = fcon ? fcon->client : NULL;
    struct client *new_c = new_focus_con ? new_focus_con->client : NULL;
    focus_client(old_c, new_c);
}

void focus_top_container(struct monitor *m, enum focus_actions a)
{
    struct container *con = get_container_on_focus_stack(m, 0);

    if (!con)
        return;

    focus_container(con, a);
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

    struct container *con1 = get_container(m, pos2);

    if (!con1)
        return;
    if (con1->floating)
        return;

    struct container *master = wl_container_of(containers.next, master, mlink);

    struct container *con2 = get_container(selected_monitor, pos1);

    if (con2 == con1)
        return;

    wl_list_remove(&con2->mlink);
    wl_list_insert(&con1->mlink, &con2->mlink);
    wl_list_remove(&con1->mlink);
    wl_list_insert(&con2->mlink, &con1->mlink);

    arrange();

    struct layout *lt = get_layout_on_monitor(m);
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

    struct monitor *m = selected_monitor;
    struct layout *lt = get_layout_on_monitor(m);

    con->floating = floating;

    // move container to the selected workspace
    if (!con->floating)
        set_container_workspace(con, m->ws_ids[0]);

    lift_container(con);
    con->client->bw = lt->options.float_border_px;
    con->client->resize = true;
    container_damage_whole(con);
}

void set_container_geom(struct container *con, struct wlr_box geom)
{
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

    con->m = ws->m;
    con->client->ws_id = ws_id;

    if (con->floating)
        con->client->bw = ws->layout->options.float_border_px;
    else 
        con->client->bw = ws->layout->options.tile_border_px;
}

void set_container_monitor(struct container *con, struct monitor *m)
{
    if (con->m == m)
        return;

    con->m = m;
    con->client->ws_id = m->ws_ids[0];
}

void move_container(struct container *con, struct wlr_cursor *cursor, int offsetx, int offsety)
{
    struct wlr_box geom = con->geom;
    geom.x = cursor->x - offsetx;
    geom.y = cursor->y - offsety;
    /* geom.x = server.cursor.wlr_cursor->x - geom.x, */
    /* geom.y = server.cursor.wlr_cursor->y - geom.y, */
    /* geom.x = container_relative_x_to_absolute(con, dx); */
    /* geom.y = container_relative_y_to_absolute(con, dy); */
    resize(con, geom);
    container_damage(con, true);
}

void resize_container(struct container *con, struct wlr_cursor *cursor, int offsetx, int offsety)
{
    struct wlr_box geom = con->geom;

    geom.width = absolute_x_to_container_relative(con, cursor->x - offsetx);
    geom.height = absolute_y_to_container_relative(con, cursor->y - offsety);
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

    printf("min_width: %f min_height: %f\n", resize_constraints->min_width, resize_constraints->min_height);
    printf("max_width: %f max_height: %f\n", resize_constraints->max_width, resize_constraints->max_height);

    return is_width_not_in_limit || is_height_not_in_limit;
}
