#include "container.h"

#include <lua.h>
#include <stdlib.h>
#include <wayland-util.h>

#include "client.h"
#include "parseConfig.h"
#include "server.h"
#include "monitor.h"
#include "tile/tileUtils.h"
#include "render/render.h"

static void add_container_to_monitor(struct container *con, struct monitor *m);
static void add_container_to_monitor_containers(struct container *con, int i);
static void add_container_to_focus_stack(struct container *con);
static void add_container_to_monitor_stack(struct container *con);

struct container *create_container(struct client *c, struct monitor *m, bool has_border)
{
    struct container *con = calloc(1, sizeof(struct container));
    con->m = m;
    con->client = c;
    con->has_border = has_border;
    con->focusable = true;
    c->con = con;
    add_container_to_monitor(con, con->m);
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

static void container_damage(struct container *con, bool whole)
{
    struct monitor *m;
    wl_list_for_each(m, &mons, link) {
        output_damage_surface(m, get_wlrsurface(con->client), con->geom.x, con->geom.y, whole);

        double ox, oy;
        int w, h;
        ox = con->geom.x - con->client->bw;
        oy = con->geom.y - con->client->bw;
        wlr_output_layout_output_coords(server.output_layout, m->wlr_output, &ox, &oy);
        w = con->geom.width;
        h = con->geom.height;

        struct wlr_box *borders;
        borders = (struct wlr_box[4]) {
            {ox, oy, w + 2 * con->client->bw, con->client->bw},             /* top */
                {ox, oy + con->client->bw, con->client->bw, h},                 /* left */
                {ox + con->client->bw + w, oy + con->client->bw, con->client->bw, h},     /* right */
                {ox, oy + con->client->bw + h, w + 2 * con->client->bw, con->client->bw}, /* bottom */
        };

        for (int i = 0; i < 4; i++) {
            scale_box(&borders[i], m->wlr_output->scale);
            wlr_output_damage_add_box(m->damage, &borders[i]);
        }
        wlr_output_damage_add_whole(m->damage);
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

struct container *container_position_to_container(int position)
{
    // TODO debug
    struct container *con;
    wl_list_for_each(con, &containers, mlink) {
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

    if (!visibleon(con, m->ws[0]))
        return NULL;

    return con;
}

struct container *next_container(struct monitor *m)
{
    if (wl_list_length(&focus_stack) >= 2)
    {
        struct container *con =
            wl_container_of(focus_stack.next->next, con, flink);
        if (!visibleon(con, m->ws[0]))
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
        while (i > 0) {
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

struct container *first_container(struct monitor *m)
{
    if (m->ws[0]->layout[0].n <= 0)
        return NULL;

    struct container *con;
    wl_list_for_each(con, &stack, slink) {
        if (visibleon(con, m->ws[0]))
            break;
    }
    return con;
}

struct client *last_client(struct monitor *m)
{
    if (m->ws[0]->layout[0].n <= 0)
        return NULL;

    struct container *con;
    int i = 1;
    wl_list_for_each(con, &stack, slink) {
        if (!visibleon(con, m->ws[0]))
            continue;
        if (i > m->ws[0]->layout[0].n)
            return con->client;
        i++;
    }
    return NULL;
}

struct container *xytocontainer(double x, double y)
{
    struct monitor *m = xytomon(x, y);

    struct container *con;
    wl_list_for_each(con, &layer_stack, llink) {
        if (!con->focusable)
            continue;
        if (con->client->surface.layer->current.layer >
                ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM)
            continue;
        if (!visibleon(con, m->ws[0]) || !wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }

    wl_list_for_each(con, &stack, slink) {
        if (!con->focusable)
            continue;
        if (!visibleon(con, m->ws[0]) || !wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }

    wl_list_for_each(con, &layer_stack, llink) {
        if (!con->focusable)
            continue;
        if (con->client->surface.layer->current.layer <
                ZWLR_LAYER_SHELL_V1_LAYER_TOP)
            continue;
        if (!visibleon(con, m->ws[0]) || !wlr_box_contains_point(&con->geom, x, y))
            continue;

        return con;
    }
    return NULL;
}

static void add_container_to_monitor_containers(struct container *con, int i)
{
    struct monitor *m = con->m;
    if (!con)
        return;

    if (!con->floating) {
        /* Insert container container*/
        struct container *con2 = get_container(m, i);
        if (con2)
            wl_list_insert(&con2->mlink, &con->mlink);
        else
            wl_list_insert(&containers, &con->mlink);
    } else {
        /* Insert container after the last non floating container */
        struct container *con2;
        wl_list_for_each_reverse(con2, &containers, mlink) {
            if (!con2->floating)
                break;
        }

        if (wl_list_empty(&containers))
            wl_list_insert(containers.prev, &con->mlink);
        else
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

static void add_container_to_monitor_stack(struct container *con)
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
            add_container_to_monitor_containers(con, 0);
            add_container_to_monitor_stack(con);
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


void apply_bounds(struct container *con, struct wlr_box bbox)
{
    /* set minimum possible */
    con->geom.width = MAX(1, con->geom.width);
    con->geom.height = MAX(1, con->geom.height);

    if (con->geom.x >= bbox.x + bbox.width)
        con->geom.x = bbox.x + bbox.width - con->geom.width;
    if (con->geom.y >= bbox.y + bbox.height)
        con->geom.y = bbox.y + bbox.height - con->geom.height;
    if (con->geom.x + con->geom.width + 2 * con->client->bw <= bbox.x)
        con->geom.x = bbox.x;
    if (con->geom.y + con->geom.height + 2 * con->client->bw <= bbox.y)
        con->geom.y = bbox.y;
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
        bool same_id = strcmp(app_id, r.id) == 0;
        bool id_empty = strcmp(title, "") == 0;
        bool same_title = strcmp(title, r.title) == 0;
        bool title_empty = strcmp(title, "") == 0;
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

    struct monitor *m = con->m;
    struct container *fcon = focused_container(m);

    if (a == FOCUS_LIFT)
        lift_container(con);

    /* Put the new client atop the focus stack */
    wl_list_remove(&con->flink);
    add_container_to_focus_stack(con);

    struct container *new_focus_con = focused_container(m);

    struct client *c = fcon ? fcon->client : NULL;
    struct client *c2 = new_focus_con ? new_focus_con->client : NULL;
    focus_client(c, c2);
}

void lift_container(struct container *con)
{
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;

    wl_list_remove(&con->slink);
    add_container_to_monitor_stack(con);
}

void set_container_floating(struct container *con, bool floating)
{
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;
    if (con->floating == floating)
        return;

    con->floating = floating;

    if (con->floating) {
        lift_container(con);
        wl_list_remove(&con->mlink);
        add_container_to_monitor_containers(con, -1);
        con->client->bw = selected_monitor->ws[0]->layout->options.float_border_px;
    } else {
        lift_container(con);
        wl_list_remove(&con->mlink);
        add_container_to_monitor_containers(con, -1);
        con->client->bw = selected_monitor->ws[0]->layout->options.tile_border_px;
    }
}

void set_container_monitor(struct container *con, struct monitor *m)
{
    if (con->m == m)
        return;

    con->m = m;
    con->client->ws = m->ws[0];
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
    resize(con, geom, false);
}

void resize_container(struct container *con, struct wlr_cursor *cursor, int offsetx, int offsety)
{
    struct wlr_box geom = con->geom;

    geom.width = absolute_x_to_container_relative(con, cursor->x - offsetx);
    geom.height = absolute_y_to_container_relative(con, cursor->y - offsety);
    resize(con, geom, false);
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
