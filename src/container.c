#include "container.h"

#include <lua.h>
#include <stdlib.h>
#include <wayland-util.h>
#include <string.h>
#include <assert.h>

#include "container.h"
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
#include "tag.h"
#include "rules/rule.h"
#include "list_sets/focus_stack_set.h"
#include "options.h"

static void add_container_to_tag(struct container *con, struct tag *tag);

static struct container_property *create_container_property(struct container *con)
{
    struct container_property *property = calloc(1, sizeof(*property));
    property->con = con;
    return property;
}

static void destroy_container_property(void *property_ptr)
{
    struct container_property *property = property_ptr;
    free(property);
}

bool container_property_is_floating(struct container_property *property)
{
    if (!property)
        return false;
    return property->floating;
}

struct wlr_box container_property_get_floating_geom(struct container_property *property)
{
    struct container *con = property->con;
    struct wlr_box geom = property->floating_geom;

    if (con->client->type == LAYER_SHELL) {
        geom = con->global_geom;
    }
    return geom;
}

void container_property_set_floating_geom(struct container_property *property,
        struct wlr_box geom)
{
    struct container *con = property->con;
    struct wlr_box *old_geom = &property->floating_geom;

    if (con->client->type == LAYER_SHELL) {
        old_geom = &con->global_geom;
    }

    *old_geom = geom;
}

void container_property_set_floating(struct container_property *property, bool floating)
{
    if (!property)
        return;
    property->floating = floating;
    struct container *con = property->con;
    if (!container_property_is_floating(property)) {
        struct monitor *m = server_get_selected_monitor();
        if (container_get_monitor(con) != server_get_selected_monitor() || con->on_scratchpad) {
            struct tag *sel_tag = monitor_get_active_tag(m);
            container_set_tag_id(con, sel_tag->id);
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

    container_update_size(con);
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
    for (int i = 0; i < server_get_tag_key_count(); i++) {
        struct container_property *container_property = create_container_property(con);
        g_ptr_array_add(con->properties, container_property);
    }

    struct tag *tag = monitor_get_active_tag(m);
    container_set_tag_id(con, tag->id);

    return con;
}

void destroy_container(struct container *con)
{
    if (server.grab_c == con) {
        server.grab_c = NULL;
    }

    g_ptr_array_unref(con->properties);
    free(con);
}

void add_container_to_tile(struct container *con)
{
    assert(!con->is_on_tile);
    add_container_to_tag(con, get_tag(con->tag_id));

    struct monitor *m = container_get_monitor(con);
    if (m) {
        struct event_handler *ev = server.event_handler;
        call_create_container_function(ev, get_position_in_container_focus_stack(con));
    }

    con->is_on_tile = true;

    tag_update_names(server_get_tags());
    ipc_event_tag();
}

void remove_container_from_tile(struct container *con)
{
    assert(con->is_on_tile);
    if (con->on_scratchpad)
        remove_container_from_scratchpad(con);

    struct tag *tag = get_tag(con->tag_id);

    tag_remove_container_from_focus_stack(tag, con);

    switch (con->client->type) {
        case LAYER_SHELL:
            tag_remove_container_from_visual_stack_layer(tag, con);
            break;
        case X11_UNMANAGED:
            remove_container_from_stack(tag, con);
            tag_remove_container(tag, con);
            break;
        default:
            remove_container_from_stack(tag, con);
            tag_remove_container(tag, con);
            break;
    }

    con->is_on_tile = false;
    tag_update_names(server_get_tags());
    ipc_event_tag();
}

void container_damage_borders_at_monitor(struct container *con, struct monitor *m)
{
    if (!con)
        return;
    if (!m)
        return;

    struct wlr_box *borders;
    borders = (struct wlr_box[4]) {
        container_get_current_border_geom(con, WLR_EDGE_TOP),
        container_get_current_border_geom(con, WLR_EDGE_LEFT),
        container_get_current_border_geom(con, WLR_EDGE_RIGHT),
        container_get_current_border_geom(con, WLR_EDGE_BOTTOM),
    };

    for (int i = 0; i < 4; i++) {
        struct wlr_box border = borders[i];
        double ox = border.x;
        double oy = border.y;
        wlr_output_layout_output_coords(server.output_layout, m->wlr_output, &ox, &oy);
        struct wlr_box obox = {
            .x = ox,
            .y = oy,
            .width = border.width,
            .height = border.height,
        };
        scale_box(&obox, m->wlr_output->scale);
        wlr_output_damage_add_box(m->damage, &obox);
    }
}

void container_damage_borders(struct container *con)
{
    if (!con)
        return;

    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        container_damage_borders_at_monitor(con, m);
    }
}

static void damage_container_area(struct container *con, struct wlr_box geom,
        bool whole)
{
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        output_damage_surface(m, get_wlrsurface(con->client), &geom, whole);
    }
    container_damage_borders(con);
}

static void container_damage(struct container *con, bool whole)
{
    for (int i = 0; i < server.mons->len; i++) {
        struct wlr_box con_geom = container_get_current_content_geom(con);
        damage_container_area(con, con_geom, whole);
    }

    struct client *c = con->client;
    if (c->resized || c->moved_tag) {
        damage_container_area(con, con->prev_geom, whole);
        c->resized = false;
        c->moved_tag = false;
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

    struct tag *tag = monitor_get_active_tag(m);
    struct container *con = tag_get_focused_container(tag);
    return con;
}

struct container *xy_to_container(double x, double y)
{
    struct monitor *m = xy_to_monitor(x, y);
    if (!m)
        return NULL;

    struct tag *tag = monitor_get_active_tag(m);
    GPtrArray *stack_set = tag_get_complete_stack_copy(tag);
    for (int i = 0; i < stack_set->len; i++) {
        struct container *con = g_ptr_array_index(stack_set, i);
        if (!con->focusable)
            continue;
        if (!container_viewable_on_monitor(m, con))
            continue;
        struct wlr_box geom = container_get_current_geom(con);
        if (!wlr_box_contains_point(&geom, x, y))
            continue;

        g_ptr_array_unref(stack_set);
        return con;
    }

    g_ptr_array_unref(stack_set);
    return NULL;
}

static void add_container_to_tag(struct container *con, struct tag *tag)
{
    if (!tag || !con)
        return;

    struct monitor *m = tag_get_monitor(tag);
    set_container_monitor(con, m);
    switch (con->client->type) {
        case LAYER_SHELL:
            // layer shell programs aren't pushed to the stack because they use the
            // layer system to set the correct render position
            if (con->client->surface.layer->current.layer
                    == ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND) {
                con->focusable = false;
            }
            tag_add_container_to_visual_stack_layer(tag, con);
            break;
        case X11_UNMANAGED:
            add_container_to_stack(tag, con);
            tag_add_container_to_focus_stack(tag, 0, con);
            break;
        case XDG_SHELL:
        case X11_MANAGED:
            {
                int position = tag_get_new_position(tag);
                tag_add_container_to_containers(tag, position, con);
                add_container_to_stack(tag, con);
                int new_focus_position = tag_get_new_focus_position(tag);
                tag_add_container_to_focus_stack(tag, new_focus_position, con);

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

struct wlr_box apply_bounds(struct container *con, struct wlr_box box)
{
    /* set minimum possible */
    struct wlr_box con_geom = container_get_current_content_geom(con);
    con_geom.width = MAX(MIN_CONTAINER_WIDTH, con_geom.width);
    con_geom.height = MAX(MIN_CONTAINER_HEIGHT, con_geom.height);

    struct direction_value bw = container_get_border_width(con);

    if (con_geom.x >= box.x + box.width)
        con_geom.x = box.x + box.width - con_geom.width;
    if (con_geom.y >= box.y + box.height)
        con_geom.y = box.y + box.height - con_geom.height;
    if (con_geom.x + con_geom.width + bw.left + bw.right <= box.x)
        con_geom.x = box.x;
    if (con_geom.y + con_geom.height + bw.top + bw.bottom <= box.y)
        con_geom.y = box.y;

    container_set_current_content_geom(con, con_geom);
    return con_geom;
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

void configure_notify(struct wl_listener *listener, void *data)
{
    /* NO-OP */
}

void ack_configure(struct wl_listener *listener, void *data)
{
    /* NO-OP */
}

void focus_on_stack(struct monitor *m, int i)
{
    struct container *sel = monitor_get_focused_container(m);

    if (!sel)
        return;

    if (sel->client->type == LAYER_SHELL) {
        struct tag *tag = monitor_get_active_tag(m);
        struct container *con = get_container(tag, 0);
        tag_focus_container(tag, con);
        return;
    }

    struct tag *tag = monitor_get_active_tag(m);
    GPtrArray *visible_container_list = tagset_get_global_floating_copy(tag);
    guint sel_index;
    g_ptr_array_find(visible_container_list, sel, &sel_index);
    struct container *con =
        get_relative_item_in_list(visible_container_list, sel_index, i);
    g_ptr_array_unref(visible_container_list);

    tag_focus_container(tag, con);
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

    struct tag *tag = monitor_get_active_tag(m);
    GPtrArray *hidden_containers = tagset_get_hidden_list_copy(tag);
    struct container *con = get_relative_item_in_list(hidden_containers, 0, i);

    if (!con)
        return;

    GPtrArray *visible_containers = tagset_get_visible_list_copy(tag);

    if (container_is_floating(sel)) {
        // TODO implement this
        // it could be something like this but I don't know
        container_set_floating(con, NULL, true);
        container_set_floating(sel, NULL, false);

        struct wlr_box sel_geom = container_get_floating_geom(sel);
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

    GPtrArray *tiled_list = tag_get_tiled_list(tag);
    sub_list_write_to_parent_list1D(tiled_list, result_list);
    tag_write_to_tags(tag);
    tag_write_to_focus_stacks(tag);
    g_ptr_array_unref(result_list);

    arrange();
    tag_this_focus_container(con);
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

    struct tag *tag = monitor_get_active_tag(m);
    GPtrArray *hidden_containers = tagset_get_hidden_list_copy(tag);
    struct container *con = get_relative_item_in_list(hidden_containers, 0, i);

    if (!con)
        return;

    GPtrArray *visible_containers = tagset_get_visible_list_copy(tag);

    if (container_is_floating(sel)) {
        // TODO implement this
        // it could be something like this but I don't know
        container_set_floating(con, NULL, true);
        container_set_floating(sel, NULL, false);

        struct wlr_box sel_geom = container_get_floating_geom(sel);
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

    GPtrArray *tiled_list = tag_get_tiled_list(tag);
    sub_list_write_to_parent_list1D(tiled_list, result_list);
    tag_write_to_tags(tag);
    tag_write_to_focus_stacks(tag);
    g_ptr_array_unref(result_list);

    arrange();
    tag_this_focus_container(con);
}

void lift_container(struct container *con)
{
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;

    struct monitor *m = container_get_monitor(con);
    struct tag *tag = monitor_get_active_tag(m);
    remove_container_from_stack(tag, con);
    add_container_to_stack(tag, con);
}

void repush(int pos1, int pos2)
{
    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    GPtrArray *tiled_containers = tag_get_tiled_list_copy(tag);

    if (pos1 >= tiled_containers->len)
        return;
    if (pos2 >= tiled_containers->len)
        return;

    struct container *con = g_ptr_array_index(tiled_containers, pos1);
    g_ptr_array_unref(tiled_containers);

    // TODO: this doesn't work if tiled by focus
    tag_repush_tag(tag, con, pos2);

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

    struct tag *tag = monitor_get_active_tag(m);
    if (!container_is_floating(con)) {
        tag_remove_container_from_floating_stack_locally(tag, con);
        tag_add_container_to_containers_locally(tag, 0, con);
    } else {
        tag_remove_container_from_containers_locally(tag, con);
        tag_add_container_to_floating_stack_locally(tag, 0, con);
    }
}

void container_fix_position(struct container *con)
{
    if (!con)
        return;

    struct monitor *m = container_get_monitor(con);

    if (!m)
        return;

    struct tag *tag = monitor_get_active_tag(m);
    GPtrArray *tiled_containers = tag_get_tiled_list_copy(tag);
    struct layout *lt = tag_get_layout(tag);

    if (!container_is_floating(con)) {
        g_ptr_array_remove(tiled_containers, con);
        int last_pos = lt->n_tiled;
        if (tiled_containers->len <= last_pos) {
            g_ptr_array_add(tiled_containers, con);
        } else {
            g_ptr_array_insert(tiled_containers, last_pos, con);
        }
    }
    tag_write_to_tags(tag);
    tag_write_to_focus_stacks(tag);
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

    if (fix_position)
        fix_position(con);

    container_property_set_floating(property, floating);
}

void container_set_hidden(struct container *con, bool b)
{
    struct tag *tag = server_get_selected_tag();
    container_set_hidden_at_tag(con, b, tag);
}

void container_set_hidden_at_tag(struct container *con, bool b, struct tag *tag)
{
    struct container_property *property =
        container_get_property_at_tag(con, tag);
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

    struct tag *tag = monitor_get_active_tag(m);
    container_set_tag(con, tag);
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
    if (!con)
        return;

    struct tag *con_tag = container_get_current_tag(server.grab_c);
    struct wlr_box geom = container_get_current_geom_at_tag(server.grab_c, con_tag);
    geom.x = cursor->x - offsetx;
    geom.y = cursor->y - offsety;

    if (con->on_scratchpad) {
        remove_container_from_scratchpad(con);
    }
    if (container_is_tiled(con)) {
        container_set_floating(con, container_fix_position, true);
        arrange();
    }
    container_set_floating_geom_at_tag(con, geom, con_tag);
    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    struct layout *lt = tag_get_layout(tag);
    container_set_border_width(con, direction_value_uniform(lt->options->float_border_px));

    con->client->resized = true;
    container_damage(con, true);
}

struct container_property *container_get_property(struct container *con)
{
    struct tag *tag = container_get_current_tag(con);
    struct container_property *property = container_get_property_at_tag(con, tag);
    return property;
}

struct container_property *container_get_property_at_tag(
        struct container *con,
        struct tag *tag)
{
    if (!tag)
        return NULL;
    while (tag->id >= con->properties->len) {
        struct container_property *property = create_container_property(con);
        g_ptr_array_add(con->properties, property);
    }

    struct container_property *property =
        g_ptr_array_index(con->properties, tag->id); 
    return property;
}

void container_set_current_geom(struct container *con, struct wlr_box geom)
{
    struct tag *tag = container_get_current_tag(con);
    struct layout *lt = tag_get_layout(tag);
    if (container_is_tiled(con) || lt->options->arrange_by_focus) {
        container_set_tiled_geom(con, geom);
    } else {
        container_set_floating_geom(con, geom);
    }
}

void container_set_tiled_geom(struct container *con, struct wlr_box geom)
{
    struct wlr_box con_geom = container_get_tiled_geom(con);

    if (con->client->type == LAYER_SHELL) {
        con_geom = con->global_geom;
    }

    bool preserve_ratio = con->ratio != 0;
    if (preserve_ratio) {
        /* calculated biggest container where con->geom.width and
         * con->geom.height = con->geom.width * con->ratio is inside geom.width
         * and geom.height
         * */
        int available_height = geom.height;
        int available_width = geom.width;
        if (con->ratio <= 1) {
            // use geom->width
            int proposed_height = geom.width * con->ratio;
            if (proposed_height > available_height) {
                float anti_ratio = 1/con->ratio;
                int proposed_width = geom.height * anti_ratio;
                geom.width = MIN(proposed_width, available_width);
            } else {
                geom.height = MIN(proposed_height, available_height);
            }
        } else {
            float anti_ratio = 1/con->ratio;
            int proposed_width = geom.height * anti_ratio;
            if (proposed_width > available_width) {
                int proposed_height = geom.width * con->ratio;
                geom.height = MIN(proposed_height, available_height);
            } else {
                geom.width = MIN(proposed_width, available_width);
            }
        }
        geom.y += available_height/2 - geom.height/2;
        geom.x += available_width/2 - geom.width/2;
    }

    con->prev_geom = con_geom;

    if (con->client->type == LAYER_SHELL) {
        con->global_geom = geom;
        return;
    }

    struct container_property *property = container_get_property(con);
    if (!property)
        return;
    property->geom = geom;
}

void container_set_floating_geom_at_tag(struct container *con,
        struct wlr_box geom, struct tag *tag)
{
    struct container_property *property =
        container_get_property_at_tag(con, tag);

    if (!property)
        return;

    struct wlr_box *con_geom = &property->floating_geom;
    if (con->client->type == LAYER_SHELL) {
        con_geom = &con->global_geom;
    }

    con->prev_geom = *con_geom;
    *con_geom = geom;
}

void container_set_floating_geom(struct container *con, struct wlr_box geom)
{
    struct tag *tag = container_get_current_tag(con);
    container_set_floating_geom_at_tag(con, geom, tag);
}

void container_set_current_content_geom(struct container *con, struct wlr_box geom)
{
    struct wlr_box box_geom = container_content_geometry_to_box(con, geom);
    container_set_current_geom(con, box_geom);
}

void container_set_tiled_content_geom(struct container *con, struct wlr_box geom)
{
    struct wlr_box box_geom = container_content_geometry_to_box(con, geom);
    container_set_tiled_geom(con, box_geom);
}

void container_set_floating_content_geom(struct container *con, struct wlr_box geom)
{
    struct wlr_box box_geom = container_content_geometry_to_box(con, geom);
    container_set_floating_geom(con, box_geom);
}

void container_set_hidden_edges(struct container *con, enum wlr_edges edges)
{
    if (con->hidden_edges == edges)
        return;
    con->hidden_edges = edges;
    container_update_size(con);
}

enum wlr_edges container_get_hidden_edges(struct container *con)
{
    return con->hidden_edges;
}

struct direction_value direction_value_uniform(int value)
{
    return (struct direction_value){
        .bottom = value,
        .top = value,
        .left = value,
        .right = value,
    };
}

struct wlr_box container_get_tiled_geom_at_tag(struct container *con, struct tag *tag)
{
    struct container_property *property =
        g_ptr_array_index(con->properties, tag->id);

    struct wlr_box geom = property->geom;

    if (con->client->type == LAYER_SHELL) {
        geom = con->global_geom;
    }

    return geom;
}

struct wlr_box container_get_tiled_geom(struct container *con)
{
    struct tag *tag = container_get_current_tag(con);
    if (!tag)
        return (struct wlr_box){0};

    struct wlr_box geom = container_get_tiled_geom_at_tag(con, tag);
    return geom;
}

struct wlr_box container_get_floating_geom_at_tag(struct container *con, struct tag *tag)
{
    struct container_property *property =
        g_ptr_array_index(con->properties, tag->id);

    return container_property_get_floating_geom(property);
}

struct wlr_box container_get_floating_geom(struct container *con)
{
    struct tag *tag = container_get_current_tag(con);
    if (!tag)
        return (struct wlr_box){0};

    struct wlr_box geom = container_get_floating_geom_at_tag(con, tag);
    return geom;
}

struct wlr_box container_get_current_geom_at_tag(struct container *con, struct tag *tag)
{
    if (!con)
        return (struct wlr_box){0};
    if (!tag)
        return (struct wlr_box){0};

    struct wlr_box geom;
    struct layout *lt = tag_get_layout(tag);
    if (container_is_unmanaged(con)) {
        geom = container_get_floating_geom_at_tag(con, tag);
    } else if (container_is_tiled(con) || (lt && lt->options->arrange_by_focus)) {
        geom = container_get_tiled_geom_at_tag(con, tag);
    } else {
        geom = container_get_floating_geom_at_tag(con, tag);
    }
    return geom;
}

struct wlr_box container_get_current_geom(struct container *con)
{
    struct tag *tag = container_get_current_tag(con);
    struct wlr_box geom = container_get_current_geom_at_tag(con, tag);
    return geom;
}

struct wlr_box container_get_tiled_content_geom_at_tag(struct container *con, struct tag *tag)
{
    struct wlr_box box_geom = container_get_tiled_geom_at_tag(con, tag);
    struct wlr_box geom = container_box_to_content_geometry(con, box_geom);
    return geom;
}

struct wlr_box container_get_floating_content_geom_at_tag(struct container *con, struct tag *tag)
{
    struct wlr_box box_geom = container_get_floating_geom_at_tag(con, tag);
    struct wlr_box geom = container_content_geometry_to_box(con, box_geom);
    return geom;
}

struct wlr_box container_get_tiled_content_geom(struct container *con)
{
    struct wlr_box box_geom = container_get_tiled_geom(con);
    struct wlr_box geom = container_box_to_content_geometry(con, box_geom);
    return geom;
}

struct wlr_box container_get_floating_content_geom(struct container *con)
{
    struct wlr_box content_geom = container_get_floating_geom(con);
    struct wlr_box geom = container_box_to_content_geometry(con, content_geom);
    return geom;
}

struct wlr_box container_get_current_content_geom(struct container *con)
{
    struct wlr_box box_geom = container_get_current_geom(con);
    struct wlr_box geom = container_box_to_content_geometry(con, box_geom);
    return geom;
}

struct wlr_box container_get_current_border_geom(struct container *con, enum wlr_edges dir)
{
    struct wlr_box geom = container_get_current_geom(con);
    enum wlr_edges hidden_edges = container_get_hidden_edges(con);
    switch (dir) {
        case WLR_EDGE_TOP:
            {
                if (!(hidden_edges & WLR_EDGE_TOP)) {
                    struct direction_value d = container_get_border_width(con);
                    geom.height = d.top;
                }
                break;
            }
        case WLR_EDGE_BOTTOM:
            {
                if (!(hidden_edges & WLR_EDGE_BOTTOM)) {
                    struct direction_value d = container_get_border_width(con);
                    geom.y += geom.height - d.bottom;
                    geom.height = d.bottom;
                }
                break;
            }
            break;
        case WLR_EDGE_LEFT:
            {
                if (!(hidden_edges & WLR_EDGE_LEFT)) {
                    struct direction_value d = container_get_border_width(con);
                    geom.width = d.left;
                }
                break;
            }
            break;
        case WLR_EDGE_RIGHT:
            {
                if (!(hidden_edges & WLR_EDGE_RIGHT)) {
                    struct direction_value d = container_get_border_width(con);
                    geom.x += geom.width - d.right;
                    geom.width = d.right;
                }
                break;
            }
            break;
        default:
            break;
    }
    return geom;
}

struct wlr_box container_content_geometry_to_box(struct container *con,
        struct wlr_box geom)
{
    struct direction_value borders = container_get_border_width(con);
    enum wlr_edges hidden_edges = container_get_hidden_edges(con);
    struct wlr_box box_geom = geom;
    if (!(hidden_edges & WLR_EDGE_LEFT)) {
        box_geom.x -= borders.left;
        box_geom.width += borders.left;
    }
    if (!(hidden_edges & WLR_EDGE_RIGHT)) {
        box_geom.width += borders.right;
    }
    if (!(hidden_edges & WLR_EDGE_TOP)) {
        box_geom.y -= borders.top;
        box_geom.height += borders.top;
    }
    if (!(hidden_edges & WLR_EDGE_BOTTOM)) {
        box_geom.height += borders.bottom;
    }
    return box_geom;
}

struct wlr_box container_box_to_content_geometry(struct container *con,
        struct wlr_box geom)
{
    struct direction_value borders = container_get_border_width(con);
    enum wlr_edges hidden_edges = container_get_hidden_edges(con);
    struct wlr_box content_geom = geom;
    if (!(hidden_edges & WLR_EDGE_LEFT)) {
        content_geom.x += borders.left;
        content_geom.width -= borders.left;
    }
    if (!(hidden_edges & WLR_EDGE_RIGHT)) {
        content_geom.width -= borders.right;
    }
    if (!(hidden_edges & WLR_EDGE_TOP)) {
        content_geom.y += borders.top;
        content_geom.height -= borders.top;
    }
    if (!(hidden_edges & WLR_EDGE_BOTTOM)) {
        content_geom.height -= borders.bottom;
    }
    return content_geom;
}

bool container_get_hidden(struct container *con)
{
    struct container_property *property = container_get_property(con);
    if (!property)
        return false;
    return property->hidden;
}

void container_set_border_width(struct container *con, struct direction_value border_width)
{
    if (!con)
        return;

    struct container_property *property = container_get_property(con);
    if (!property)
        return;

    property->border_width = border_width;
}

struct direction_value container_get_border_width(struct container *con)
{
    struct container_property *property = container_get_property(con);
    if (!property)
        return direction_value_uniform(0);
    struct tag *tag = server_get_selected_tag();
    struct layout *lt = tag_get_layout(tag);
    if (lt && lt->options->arrange_by_focus)
        return direction_value_uniform(0);
    return property->border_width;
}

void resize_container(struct container *con, struct wlr_cursor *cursor, int offsetx, int offsety)
{
    if (!con)
        return;

    struct wlr_box geom = container_get_current_geom(con);

    geom.width = absolute_x_to_container_relative(geom, cursor->x - offsetx);
    geom.height = absolute_y_to_container_relative(geom, cursor->y - offsety);

    if (con->on_scratchpad) {
        remove_container_from_scratchpad(con);
    }
    if (container_is_tiled(con)) {
        container_set_floating(con, container_fix_position, true);
        arrange();
    }

    container_damage_borders(con);

    container_set_floating_geom(con, geom);
    struct tag *tag = container_get_current_tag(con);
    struct layout *lt = tag_get_layout(tag);
    container_set_border_width(con, direction_value_uniform(lt->options->float_border_px));

    container_update_size(con);
}

struct monitor *container_get_monitor(struct container *con)
{
    if (!con)
        return NULL;
    if (con->client->m) {
        return con->client->m;
    }

    struct tag *tag = get_tag(con->tag_id);
    if (!tag)
        return NULL;

    struct monitor *m = tag_get_monitor(tag);
    return m;
}

inline int absolute_x_to_container_relative(struct wlr_box geom, int x)
{
    return x - geom.x;
}

inline int absolute_y_to_container_relative(struct wlr_box geom, int y)
{
    return y - geom.y;
}

int get_position_in_container_focus_stack(struct container *con)
{
    if (!con)
        return INVALID_POSITION;

    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    int position = find_in_composed_list(tag->visible_focus_set->focus_stack_visible_lists, cmp_ptr, con);
    return position;
}

int get_position_in_container_stack(struct container *con)
{
    if (!con)
        return INVALID_POSITION;

    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    guint position = 0;
    g_ptr_array_find(tag->con_set->tiled_containers, con, &position);
    return position;
}

struct tag *container_get_current_tag(struct container *con)
{
    // we prioritize the selected monitor/therefore the selected tag
    struct monitor *sel_m = server_get_selected_monitor();
    struct tag *sel_tag = monitor_get_active_tag(sel_m);
    if (tagset_exist_on(sel_m, con)) {
        return sel_tag;
    }

    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        // this prevents the selected monitor from being checked twice to
        // improve performance
        if (m == sel_m) {
            continue;
        }

        if (tagset_exist_on(m, con)) {
            struct tag *tag = monitor_get_active_tag(m);
            return tag;
        }
    }

    struct tag *tag = container_get_tag(con);
    return tag;
}

struct container *get_container_from_container_stack_position(int i)
{
    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    struct container *con = get_container(tag, i);
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

void container_set_just_tag_id(struct container *con, int tag_id)
{
    if (con->tag_id == tag_id)
        return;

    // TODO optimize this
    struct tag *prev_tag = get_tag(con->tag_id);
    con->tag_id = tag_id;

    tagset_reload(prev_tag);
    struct tag *tag = get_tag(tag_id);
    tagset_reload(tag);
}

void container_set_tag_id(struct container *con, int tag_id)
{
    // TODO optimize this
    struct tag *prev_tag = get_tag(con->tag_id);
    con->tag_id = tag_id;
    bitset_reset_all(con->client->sticky_tags);
    bitset_set(con->client->sticky_tags, con->tag_id);

    struct tag *tag = get_tag(tag_id);
    if (!tag_get_monitor(tag)) {
        tag_set_current_monitor(tag, server_get_selected_monitor());
    }

    tagset_reload(prev_tag);
}

void container_set_tag(struct container *con, struct tag *tag)
{
    if (!con)
        return;
    if (!tag)
        return;
    if (con->tag_id == tag->id)
        return;

    container_set_tag_id(con, tag->id);
}

void move_container_to_tag(struct container *con, struct tag *tag)
{
    if (!tag)
        return;
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;

    container_set_tag(con, tag);
    con->client->moved_tag = true;
    container_damage_whole(con);

    struct tag *old_tag = container_get_current_tag(con);

    printf("old tag new con: %p\n", tag_get_focused_container(tag));
    printf("tag new con: %p\n", tag_get_focused_container(tag));

    arrange();
    tagset_reload(tag);

    update_reduced_focus_stack(old_tag);
    update_reduced_focus_stack(tag);
    tag_focus_most_recent_container(old_tag);
    tag_focus_most_recent_container(tag);

    tagset_reload(old_tag);
    tagset_reload(tag);
    tag_focus_most_recent_container(old_tag);
    tag_focus_most_recent_container(tag);

    tag_update_names(server_get_tags());
    tag_focus_most_recent_container(old_tag);
    tag_focus_most_recent_container(tag);
    tagset_reload(old_tag);
    tagset_reload(tag);

    ipc_event_tag();
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

struct tag *container_get_tag(struct container *con)
{
    if (!con)
        return NULL;
    struct tag *tag = get_tag(con->tag_id);
    return tag;
}

bool container_is_floating(struct container *con)
{
    struct container_property *property = container_get_property(con);
    return container_property_is_floating(property);
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

bool container_is_floating_on_tag(struct container *con, struct tag *tag)
{
    struct container_property *property = container_get_property_at_tag(con, tag);
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
