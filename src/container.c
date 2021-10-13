#include "container.h"

#include <lua.h>
#include <stdlib.h>
#include <wayland-util.h>
#include <string.h>
#include <assert.h>

#include "client.h"
#include "list_sets/container_stack_set.h"
#include "list_sets/list_set.h"
#include "popup.h"
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
#include "workspace.h"
#include "rules/rule.h"
#include "list_sets/focus_stack_set.h"
#include "options.h"

static void add_container_to_workspace(struct container *con, struct workspace *ws);

static struct container_property *create_container_property()
{
    struct container_property *property = calloc(1, sizeof(*property));
    return property;
}

static void destroy_container_property(void *property_ptr)
{
    struct container_property *property = property_ptr;
    free(property);
}

struct container *create_container(struct client *c, struct monitor *m, bool has_border)
{
    struct container *con = calloc(1, sizeof(*con));
    con->client = c;
    c->con = con;

    con->alpha = 1.0f;
    con->has_border = has_border;
    con->focusable = true;

    con->properties = g_ptr_array_new_with_free_func(destroy_container_property);
    for (int i = 0; i < server.workspaces->len; i++) {
        struct container_property *container_property = create_container_property();
        g_ptr_array_add(con->properties, container_property);
    }
    container_set_workspace_id(con, m->tagset->selected_ws_id);

    return con;
}

void destroy_container(struct container *con)
{
    g_ptr_array_unref(con->properties);
    free(con);
}

void add_container_to_tile(struct container *con)
{
    assert(!con->is_on_tile);
    add_container_to_workspace(con, get_workspace(con->client->ws_id));

    struct monitor *m = container_get_monitor(con);
    if (m) {
        struct event_handler *ev = server.event_handler;
        call_create_container_function(ev, get_position_in_container_focus_stack(con));
    }

    con->is_on_tile = true;

    apply_rules(server.default_layout->options->rules, con);
}

void remove_container_from_tile(struct container *con)
{
    assert(con->is_on_tile);
    if (con->on_scratchpad)
        remove_container_from_scratchpad(con);

    struct workspace *ws = get_workspace(con->client->ws_id);

    workspace_remove_container_from_focus_stack(ws, con);

    switch (con->client->type) {
        case LAYER_SHELL:
            workspace_remove_container_from_visual_stack_layer(ws, con);
            break;
        case X11_UNMANAGED:
            remove_container_from_stack(ws, con);
            workspace_remove_container(ws, con);
            break;
        default:
            remove_container_from_stack(ws, con);
            workspace_remove_container(ws, con);
            break;
    }

    con->is_on_tile = false;
    workspace_update_names(&server, server.workspaces);
}

void container_damage_borders(struct container *con, struct monitor *m, struct wlr_box *geom)
{
    if (!con)
        return;
    if (!geom)
        return;

    if (!m)
        return;

    int border_width = container_get_border_width(con);
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
        bool whole)
{
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        output_damage_surface(m, get_wlrsurface(con->client), geom, whole);
        container_damage_borders(con, m, geom);
    }
}

static void container_damage(struct container *con, bool whole)
{
    for (int i = 0; i < server.mons->len; i++) {
        struct wlr_box *con_geom = container_get_current_geom(con);

        if (!con_geom)
            continue;

        damage_container_area(con, con_geom, whole);
    }

    struct client *c = con->client;
    if (c->resized || c->moved_workspace) {
        damage_container_area(con, &con->prev_geom, whole);
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

struct container *monitor_get_focused_container(struct monitor *m)
{
    if (!m)
        return NULL;

    struct workspace *ws = monitor_get_active_workspace(m);
    struct container *con = workspace_get_focused_container(ws);
    return con;
}

struct container *xy_to_container(double x, double y)
{
    struct monitor *m = xy_to_monitor(x, y);
    if (!m)
        return NULL;

    struct workspace *ws = monitor_get_active_workspace(m);
    GPtrArray *stack_set = workspace_get_complete_stack_copy(ws);
    for (int i = 0; i < stack_set->len; i++) {
        struct container *con = g_ptr_array_index(stack_set, i);
        if (!con->focusable)
            continue;
        if (!container_viewable_on_monitor(m, con))
            continue;
        if (!wlr_box_contains_point(container_get_current_geom(con), x, y))
            continue;

        g_ptr_array_unref(stack_set);
        return con;
    }

    g_ptr_array_unref(stack_set);
    return NULL;
}

static void add_container_to_workspace(struct container *con, struct workspace *ws)
{
    if (!ws || !con)
        return;

    struct monitor *m = workspace_get_monitor(ws);
    set_container_monitor(con, m);
    switch (con->client->type) {
        case LAYER_SHELL:
            // layer shell programs aren't pushed to the stack because they use the
            // layer system to set the correct render position
            if (con->client->surface.layer->current.layer
                    == ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND) {
                con->focusable = false;
            }
            workspace_add_container_to_visual_stack_layer(ws, con);
            break;
        case X11_UNMANAGED:
            add_container_to_stack(ws, con);
            workspace_add_container_to_focus_stack(ws, 0, con);
            break;
        case XDG_SHELL:
        case X11_MANAGED:
            {
                int position = workspace_get_new_position(ws);
                workspace_add_container_to_containers(ws, position, con);
                add_container_to_stack(ws, con);
                int new_focus_position = workspace_get_new_focus_position(ws);
                workspace_add_container_to_focus_stack(ws, new_focus_position, con);
                break;
            }
    }
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
    struct wlr_box *con_geom = container_get_current_geom(con);
    if (!con_geom)
        return;
    con_geom->width = MAX(MIN_CONTAINER_WIDTH, con_geom->width);
    con_geom->height = MAX(MIN_CONTAINER_HEIGHT, con_geom->height);

    int border_width = container_get_border_width(con);

    if (con_geom->x >= box.x + box.width)
        con_geom->x = box.x + box.width - con_geom->width;
    if (con_geom->y >= box.y + box.height)
        con_geom->y = box.y + box.height - con_geom->height;
    if (con_geom->x + con_geom->width + 2 * border_width <= box.x)
        con_geom->x = box.x;
    if (con_geom->y + con_geom->height + 2 * border_width <= box.y)
        con_geom->y = box.y;
}

void commit_notify(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, commit);

    if (!c)
        return;

    struct container *con = c->con;
    if (con->is_on_tile) {
        container_damage_part(c->con);
    }
}

void focus_container(struct container *con)
{
    if (!con)
        return;
    if (!con->focusable)
        return;
    if (con->is_xwayland_popup)
        return;
    if (container_get_hidden(con))
        return;

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);

    if (!container_viewable_on_monitor(m, con))
        return;

    struct container *sel = monitor_get_focused_container(m);

    /* Put the new client atop the focus stack */
    workspace_repush_on_focus_stack(ws, con, 0);

    struct container *new_sel = monitor_get_focused_container(m);

    ipc_event_window();

    call_on_focus_function(server.event_handler,
            get_position_in_container_focus_stack(con));

    struct client *old_c = sel ? sel->client : NULL;
    struct client *new_c = new_sel ? new_sel->client : NULL;
    struct seat *seat = input_manager_get_default_seat();
    focus_client(seat, old_c, new_c);
    workspace_update_names(&server, server.workspaces);
}

void focus_on_stack(struct monitor *m, int i)
{
    struct container *sel = monitor_get_focused_container(m);

    if (!sel)
        return;

    struct tagset *tagset = monitor_get_active_tagset(m);

    if (sel->client->type == LAYER_SHELL) {
        struct workspace *ws = get_workspace(tagset->selected_ws_id);
        struct container *con = get_container(ws, 0);
        focus_container(con);
        return;
    }

    GPtrArray *visible_container_list = tagset_get_global_floating_copy(tagset);
    guint sel_index;
    g_ptr_array_find(visible_container_list, sel, &sel_index);
    struct container *con =
        get_relative_item_in_list(visible_container_list, sel_index, i);
    g_ptr_array_unref(visible_container_list);

    focus_container(con);
    lift_container(con);
}


// TODO refactor
void focus_on_hidden_stack(struct monitor *m, int i)
{
    struct container *sel = monitor_get_focused_container(m);

    if (!sel)
        return;
    if (sel->client->type == LAYER_SHELL)
        return;
    if (container_is_unmanaged(sel))
        return;

    struct tagset *tagset = monitor_get_active_tagset(m);
    GPtrArray *hidden_containers = tagset_get_hidden_list_copy(tagset);
    struct container *con = get_relative_item_in_list(hidden_containers, 0, i);

    if (!con)
        return;

    GPtrArray *visible_containers = tagset_get_visible_list_copy(tagset);

    if (container_is_floating(sel)) {
        // TODO implement this
        // it could be something like this but I don't know
        container_set_floating(con, NULL, true);
        container_set_floating(sel, NULL, false);

        struct wlr_box *sel_geom = container_get_floating_geom(sel);
        container_set_floating_geom(con, sel_geom);
    }

    guint sel_index;
    if (!g_ptr_array_find(visible_containers, sel, &sel_index)) {
        return;
    }
    g_ptr_array_remove_index(visible_containers, sel_index);
    g_ptr_array_remove(hidden_containers, con);
    g_ptr_array_insert(visible_containers, sel_index, con);

    if (i < 0) {
        g_ptr_array_insert(hidden_containers, 0, sel);
    } else {
        g_ptr_array_add(hidden_containers, sel);
    }

    GPtrArray *result_list = g_ptr_array_new();
    wlr_list_cat(result_list, visible_containers);
    wlr_list_cat(result_list, hidden_containers);

    g_ptr_array_unref(visible_containers);
    g_ptr_array_unref(hidden_containers);

    struct workspace *ws = monitor_get_active_workspace(m);
    GPtrArray *tiled_list = workspace_get_tiled_list(ws);
    sub_list_write_to_parent_list1D(tiled_list, result_list);
    workspace_write_to_workspaces(ws);
    workspace_write_to_focus_stacks(ws);
    g_ptr_array_unref(result_list);

    arrange();
    focus_container(con);
}

void swap_on_hidden_stack(struct monitor *m, int i)
{
    struct container *sel = monitor_get_focused_container(m);

    if (!sel)
        return;
    if (sel->client->type == LAYER_SHELL)
        return;
    if (container_is_unmanaged(sel))
        return;

    struct tagset *tagset = monitor_get_active_tagset(m);
    GPtrArray *hidden_containers = tagset_get_hidden_list_copy(tagset);
    struct container *con = get_relative_item_in_list(hidden_containers, 0, i);

    if (!con)
        return;

    GPtrArray *visible_containers = tagset_get_visible_list_copy(tagset);

    if (container_is_floating(sel)) {
        // TODO implement this
        // it could be something like this but I don't know
        container_set_floating(con, NULL, true);
        container_set_floating(sel, NULL, false);

        struct wlr_box *sel_geom = container_get_floating_geom(sel);
        container_set_floating_geom(con, sel_geom);
    }

    guint sel_index;
    if (!g_ptr_array_find(visible_containers, sel, &sel_index)) {
        return;
    }
    guint hidden_index;
    if (!g_ptr_array_find(hidden_containers, con, &hidden_index)) {
        return;
    }
    struct container *tmp_con = g_ptr_array_index(visible_containers, sel_index);
    g_ptr_array_index(visible_containers, sel_index) =
        g_ptr_array_index(hidden_containers, hidden_index);
    g_ptr_array_index(hidden_containers, hidden_index) = tmp_con;

    GPtrArray *result_list = g_ptr_array_new();
    wlr_list_cat(result_list, visible_containers);
    wlr_list_cat(result_list, hidden_containers);

    g_ptr_array_unref(visible_containers);
    g_ptr_array_unref(hidden_containers);

    struct workspace *ws = monitor_get_active_workspace(m);
    GPtrArray *tiled_list = workspace_get_tiled_list(ws);
    sub_list_write_to_parent_list1D(tiled_list, result_list);
    workspace_write_to_workspaces(ws);
    workspace_write_to_focus_stacks(ws);
    g_ptr_array_unref(result_list);

    arrange();
    focus_container(con);
}

void lift_container(struct container *con)
{
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;

    struct monitor *m = container_get_monitor(con);
    struct workspace *ws = monitor_get_active_workspace(m);
    remove_container_from_stack(ws, con);
    add_container_to_stack(ws, con);
}

void repush(int pos1, int pos2)
{
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    GPtrArray *tiled_containers = workspace_get_tiled_list_copy(ws);

    if (pos1 >= tiled_containers->len)
        return;
    if (pos2 >= tiled_containers->len)
        return;

    struct container *con = g_ptr_array_index(tiled_containers, pos1);
    g_ptr_array_unref(tiled_containers);

    // TODO: this doesn't work if tiled by focus
    workspace_repush(ws, con, pos2);

    arrange();

    struct layout *lt = get_layout_in_monitor(m);
    if (lt->options->arrange_by_focus) {
        arrange();
    }
}

void container_fix_position_to_begin(struct container *con)
{
    if (!con)
        return;

    struct monitor *m = container_get_monitor(con);

    if (!m)
        return;

    struct workspace *ws = monitor_get_active_workspace(m);
    if (!container_is_floating(con)) {
        workspace_remove_container_from_floating_stack_locally(ws, con);
        workspace_add_container_to_containers_locally(ws, 0, con);
    } else {
        workspace_remove_container_from_containers_locally(ws, con);
        workspace_add_container_to_floating_stack_locally(ws, 0, con);
    }
}

void container_fix_position(struct container *con)
{
    if (!con)
        return;

    struct monitor *m = container_get_monitor(con);

    if (!m)
        return;

    struct workspace *ws = monitor_get_active_workspace(m);
    GPtrArray *tiled_containers = workspace_get_tiled_list_copy(ws);
    struct layout *lt = workspace_get_layout(ws);

    if (!container_is_floating(con)) {
        g_ptr_array_remove(tiled_containers, con);
        int last_pos = lt->n_tiled;
        g_ptr_array_insert(tiled_containers, last_pos, con);
    }
    workspace_write_to_workspaces(ws);
    workspace_write_to_focus_stacks(ws);
    g_ptr_array_unref(tiled_containers);
}

void container_set_floating(struct container *con, void (*fix_position)(struct container *con), bool floating)
{
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;
    if (container_is_floating(con) == floating)
        return;

    struct monitor *m = container_get_monitor(con);
    if (!m)
        m = server_get_selected_monitor();

    struct container_property *property = container_get_property(con);
    property->floating = floating;

    if (fix_position)
        fix_position(con);

    if (!container_is_floating(con)) {
        struct workspace *ws = container_get_workspace(con);
        struct tagset *tagset = workspace_get_selected_tagset(ws);
        if (tagset) {
            struct monitor *m = server_get_selected_monitor();
            struct workspace *sel_ws = monitor_get_active_workspace(m);
            container_set_workspace_id(con, sel_ws->id);
        }

        if (con->on_scratchpad) {
            remove_container_from_scratchpad(con);
        }
    } else {
        container_set_floating_geom(con, container_get_tiled_geom(con));
        container_set_hidden(con, false);
    }

    lift_container(con);
    con->client->resized = true;
    container_damage_whole(con);
}

void container_set_hidden(struct container *con, bool b)
{
    struct tagset *tagset = container_get_tagset(con);
    struct workspace *ws = tagset_get_workspace(tagset);
    container_set_hidden_at_workspace(con, b, ws);
}

void container_set_hidden_at_workspace(struct container *con, bool b, struct workspace *ws)
{
    struct container_property *property =
        container_get_property_at_workspace(con, ws);
    if (!property)
        return;
    property->hidden = b;
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
    container_set_workspace(con, ws);
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
    struct wlr_box *con_geom = container_get_current_geom(con);
    struct wlr_box geom = *con_geom;
    geom.x = cursor->x - offsetx;
    geom.y = cursor->y - offsety;

    if (con->on_scratchpad) {
        remove_container_from_scratchpad(con);
    }
    container_set_current_geom(con, &geom);
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = workspace_get_layout(ws);
    container_set_border_width(con, lt->options->float_border_px);
    container_damage(con, true);
}

struct container_property *container_get_property(struct container *con)
{
    struct tagset *tagset = container_get_tagset(con);
    struct workspace *ws = tagset_get_workspace(tagset);
    if (!ws)
        return NULL;
    while (ws->id >= con->properties->len) {
        struct container_property *property = create_container_property();
        g_ptr_array_add(con->properties, property);
    }

    struct container_property *property =
        g_ptr_array_index(con->properties, ws->id); 
    return property;
}

struct container_property *container_get_property_at_workspace(
        struct container *con,
        struct workspace *ws)
{
    if (!ws)
        return NULL;
    struct container_property *property =
        g_ptr_array_index(con->properties, ws->id); 
    return property;
}

void container_set_current_geom(struct container *con, struct wlr_box *geom)
{
    struct tagset *tagset = container_get_tagset(con);
    struct layout *lt = tagset_get_layout(tagset);
    if (container_is_tiled(con) || lt->options->arrange_by_focus) {
        container_set_tiled_geom(con, geom);
    } else {
        container_set_floating_geom(con, geom);
    }
}

void container_set_tiled_geom(struct container *con, struct wlr_box *geom)
{
    struct container_property *property =
        container_get_property(con);

    if (!property)
        return;

    struct wlr_box *con_geom = &property->geom;

    if (con->client->type == LAYER_SHELL) {
        con_geom = &con->global_geom;
    }

    // TODO: find out whether this works
    bool preserve_ratio = con->ratio != 0;
    if (preserve_ratio) {
        /* calculated biggest container where con->geom.width and
         * con->geom.height = con->geom.width * con->ratio is inside geom.width
         * and geom.height
         * */
        float max_height = geom->height/con->ratio;
        con_geom->width = MIN(geom->width, max_height);
        con_geom->height = con_geom->width * con->ratio;
        // TODO make a function out of that 
        // center in x direction
        con_geom->x += (geom->width - con_geom->width)/2;
        // center in y direction
        con_geom->y += (geom->height - con_geom->height)/2;
    }


    con->prev_geom = *con_geom;
    *con_geom = *geom;
    container_update_size(con);
}

void container_set_floating_geom(struct container *con, struct wlr_box *geom)
{
    struct container_property *property =
        container_get_property(con);

    if (!property)
        return;

    struct wlr_box *con_geom = &property->floating_geom;
    if (con->client->type == LAYER_SHELL) {
        con_geom = &con->global_geom;
    }

    con->prev_geom = *con_geom;
    *con_geom = *geom;
    container_update_size(con);
}

struct wlr_box *container_get_tiled_geom(struct container *con)
{
    struct tagset *tagset = container_get_tagset(con);
    struct workspace *ws = tagset_get_workspace(tagset);
    if (!ws)
        return NULL;

    struct container_property *property =
        g_ptr_array_index(con->properties, ws->id); 

    struct wlr_box *geom = &property->geom;

    if (con->client->type == LAYER_SHELL) {
        geom = &con->global_geom;
    }
    return geom;
}

struct wlr_box *container_get_floating_geom(struct container *con)
{
    struct tagset *tagset = container_get_tagset(con);
    struct workspace *ws = tagset_get_workspace(tagset);
    if (!ws) {
        return NULL;
    }

    struct container_property *property =
        g_ptr_array_index(con->properties, ws->id); 

    struct wlr_box *geom = &property->floating_geom;

    if (con->client->type == LAYER_SHELL) {
        geom = &con->global_geom;
    }
    return geom;
}

struct wlr_box *container_get_current_geom(struct container *con)
{
    struct wlr_box *geom = NULL;
    struct workspace *ws = container_get_workspace(con);
    struct layout *lt = workspace_get_layout(ws);
    if (container_is_unmanaged(con)) {
        geom = container_get_floating_geom(con);
    } else if (container_is_tiled(con) || lt->options->arrange_by_focus) {
        geom = container_get_tiled_geom(con);
    } else {
        geom = container_get_floating_geom(con);
    }
    return geom;
}

bool container_get_hidden(struct container *con)
{
    struct container_property *property = container_get_property(con);
    if (!property)
        return false;
    return property->hidden;
}

void container_set_border_width(struct container *con, int border_width)
{
    struct container_property *property = container_get_property(con);
    if (!property)
        return;
    property->border_width = border_width;
}

int container_get_border_width(struct container *con)
{
    struct container_property *property = container_get_property(con);
    if (!property)
        return 0;
    return property->border_width;
}

void resize_container(struct container *con, struct wlr_cursor *cursor, int offsetx, int offsety)
{

    struct wlr_box *con_geom = container_get_current_geom(con);
    struct wlr_box geom = *con_geom;

    geom.width = absolute_x_to_container_relative(con_geom, cursor->x - offsetx);
    geom.height = absolute_y_to_container_relative(con_geom, cursor->y - offsety);

    if (con->on_scratchpad) {
        remove_container_from_scratchpad(con);
    }
    container_set_current_geom(con, &geom);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = workspace_get_layout(ws);
    container_set_border_width(con, lt->options->float_border_px);

    container_damage(con, true);
}

struct monitor *container_get_monitor(struct container *con)
{
    if (!con)
        return NULL;
    if (con->client->m) {
        return con->client->m;
    }

    struct workspace *ws = get_workspace(con->client->ws_id);
    if (!ws)
        return NULL;

    struct monitor *m = workspace_get_monitor(ws);
    return m;
}

inline int absolute_x_to_container_relative(struct wlr_box *geom, int x)
{
    return x - geom->x;
}

inline int absolute_y_to_container_relative(struct wlr_box *geom, int y)
{
    return y - geom->y;
}

int get_position_in_container_focus_stack(struct container *con)
{
    if (!con)
        return INVALID_POSITION;

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    int position = find_in_composed_list(ws->visible_focus_set->focus_stack_visible_lists, cmp_ptr, con);
    return position;
}

int get_position_in_container_stack(struct container *con)
{
    if (!con)
        return INVALID_POSITION;

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    guint position = 0;
    g_ptr_array_find(ws->con_set->tiled_containers, con, &position);
    return position;
}

struct container *get_container_from_container_stack_position(int i)
{
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct container *con = get_container(ws, i);
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

void container_set_just_workspace_id(struct container *con, int ws_id)
{
    if (con->client->ws_id == ws_id)
        return;

    // TODO optimize this
    struct workspace *prev_ws = get_workspace(con->client->ws_id);
    struct tagset *prev_tagset = workspace_get_active_tagset(prev_ws);
    con->client->ws_id = ws_id;

    tagset_reload(prev_tagset);
    struct workspace *ws = get_workspace(ws_id);
    ws->prev_m = server_get_selected_monitor();
    struct tagset *tagset = workspace_get_active_tagset(ws);
    tagset_reload(tagset);
}

void container_set_workspace_id(struct container *con, int ws_id)
{
    // TODO optimize this
    struct workspace *prev_ws = get_workspace(con->client->ws_id);
    struct tagset *prev_tagset = workspace_get_active_tagset(prev_ws);
    con->client->ws_id = ws_id;
    bitset_reserve(con->client->sticky_workspaces, server.workspaces->len);
    bitset_reset_all(con->client->sticky_workspaces);
    bitset_set(con->client->sticky_workspaces, con->client->ws_id);

    tagset_reload(prev_tagset);
}

void container_set_workspace(struct container *con, struct workspace *ws)
{
    if (!con)
        return;
    if (!ws)
        return;
    struct monitor *m = container_get_monitor(con);
    struct tagset *tagset = monitor_get_active_tagset(m);
    if (tagset->selected_ws_id == ws->id)
        return;

    ws->prev_m = m;
    container_set_workspace_id(con, ws->id);
}

void move_container_to_workspace(struct container *con, struct workspace *ws)
{
    if (!ws)
        return;
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;

    container_set_workspace(con, ws);
    con->client->moved_workspace = true;
    container_damage_whole(con);

    arrange();
    focus_most_recent_container();

    ipc_event_workspace();
}

bool container_is_bar(struct container *con)
{
    switch (con->client->type) {
        case LAYER_SHELL:
            return layer_shell_is_bar(con);
            break;
        default:
            return false;
            break;
    }
}

struct tagset *container_get_tagset(struct container *con)
{
    struct monitor *m = container_get_monitor(con);
    if (!m)
        return NULL;
    struct tagset *tagset = monitor_get_active_tagset(m);
    return tagset;
}

struct workspace *container_get_workspace(struct container *con)
{
    struct workspace *ws = get_workspace(con->client->ws_id);
    return ws;
}

bool container_is_floating(struct container *con)
{
    struct container_property *property = container_get_property(con);
    if (!property)
        return false;
    return property->floating;
}

bool container_is_floating_and_visible(struct container *con)
{
    bool is_floating = container_is_floating(con);
    if (!is_floating)
        return false;
    struct monitor *m = server_get_selected_monitor();
    bool intersects = container_intersects_with_monitor(con, m);
    if (!intersects)
        return false;
    return true;
}

bool container_is_floating_on_workspace(struct container *con, struct workspace *ws)
{
    struct container_property *property = container_get_property_at_workspace(con, ws);
    if (!property)
        return false;
    return property->floating;
}

bool container_is_tiled(struct container *con)
{
    return !container_is_floating(con);
}

bool container_is_tiled_and_visible(struct container *con)
{
    bool is_tiled = container_is_tiled(con);
    bool is_visible = container_is_visible(con);
    return is_tiled && is_visible;
}

bool container_is_hidden(struct container *con)
{
    bool potentially_visible = container_potentially_visible(con);
    if (!potentially_visible) {
        return false;
    }
    return container_get_hidden(con);
}

bool container_is_visible(struct container *con)
{
    bool potentially_visible = container_potentially_visible(con);
    if (!potentially_visible)
        return false;
    return !container_is_hidden(con);
}

bool container_exists(struct container *con)
{
    bool on_scratchpad = container_is_on_scratchpad(con);
    if (on_scratchpad)
        return false;
    return true;
}

bool container_potentially_visible(struct container *con)
{
    struct monitor *m = server_get_selected_monitor();
    bool potentially_visible = container_potentially_viewable_on_monitor(m, con);
    return potentially_visible;
}

bool container_is_unmanaged(struct container *con)
{
    return con->is_unmanaged;
}

bool container_is_managed(struct container *con)
{
    return !container_is_unmanaged(con);
}

bool container_is_tiled_and_managed(struct container *con)
{
    bool is_managed = container_is_managed(con);
    bool is_tiled = container_is_tiled(con);
    return is_managed && is_tiled;
}

bool container_is_on_scratchpad(struct container *con)
{
    return con->on_scratchpad;
}

const char *container_get_app_id(struct container *con)
{
    if (!con)
        return "";

    const char *name = "";
    struct client *c = con->client;
    switch (c->type) {
        case XDG_SHELL:
            if (c->surface.xdg->toplevel->app_id)
                c->app_id = c->surface.xdg->toplevel->app_id;
            break;
        case LAYER_SHELL:
            c->app_id = "";
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            c->app_id = c->surface.xwayland->class;
            break;
    }

    if (con
            && con->client->app_id != NULL
            && g_strcmp0(con->client->app_id, "") != 0
       ) {

        name = con->client->app_id;
    }
    return name;
}
