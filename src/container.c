#include "container.h"

#include <lua.h>
#include <stdlib.h>
#include <wayland-util.h>
#include <string.h>
#include <assert.h>

#include "client.h"
#include "container.h"
#include "cursor.h"
#include "list_sets/container_stack_set.h"
#include "list_sets/list_set.h"
#include "popup.h"
#include "server.h"
#include "monitor.h"
#include "tile/tileUtils.h"
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
#include "lib/lib_container.h"
#include "lib/lib_layout.h"
#include "lib/lib_geom.h"
#include "root.h"

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

void scale_box(struct wlr_box *box, float scale)
{
    box->x *= scale;
    box->y *= scale;
    box->width *= scale;
    box->height *= scale;
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

struct wlr_box get_absolute_box(struct wlr_box ref, struct wlr_box box)
{
    struct wlr_box b;
    b.x = scale_integer_to_percent(ref.x) * box.width + box.x;
    b.y = scale_integer_to_percent(ref.y) * box.height + box.y;
    b.width = scale_integer_to_percent(ref.width) * box.width;
    b.height = scale_integer_to_percent(ref.height) * box.height;
    return b;
}

struct wlr_box get_relative_box(struct wlr_box box, struct wlr_box ref)
{
    struct wlr_box b;
    b.x = scale_percent_to_integer((float)box.x / ref.width);
    b.y = scale_percent_to_integer((float)box.y / ref.height);
    b.width = scale_percent_to_integer((float)box.width / ref.width);
    b.height = scale_percent_to_integer((float)box.height / ref.height);
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

// TODO: look if this function is still needed
void apply_bounds(struct wlr_box *geom, struct wlr_box box)
{
    /* set minimum possible */
    geom->width = MAX(MIN_CONTAINER_WIDTH, geom->width);
    geom->height = MAX(MIN_CONTAINER_HEIGHT, geom->height);

    if (geom->x >= box.x + box.width)
        geom->x = box.x + box.width - geom->width;
    if (geom->y >= box.y + box.height)
        geom->y = box.y + box.height - geom->height;
    if (geom->x + geom->width <= box.x)
        geom->x = box.x;
    if (geom->y + geom->height <= box.y)
        geom->y = box.y;
}

void commit_notify(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, commit);

    if (!c)
        return;

    struct container *con = c->con;
    if (con->is_on_tile) {
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

    struct wlr_scene_node *node = container_get_scene_node(con);
    if (floating) {
        wlr_scene_node_reparent(node, server.scene_floating);
    } else {
        wlr_scene_node_reparent(node, server.scene_tiled);
    }

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

    container_update_size(con);
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

    struct wlr_box rel_geom = property->geom;
    struct monitor *m = container_get_monitor(con);
    struct wlr_box m_geom = monitor_get_active_geom(m);

    struct wlr_box geom = get_absolute_box(rel_geom, m_geom);
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
    struct direction_value d = container_get_border_width(con);
    switch (dir) {
        case WLR_EDGE_TOP:
            geom.height = d.top;
            break;
        case WLR_EDGE_BOTTOM:
            geom.y += geom.height - d.bottom;
            geom.height = d.bottom;
            break;
        case WLR_EDGE_LEFT:
            geom.width = d.left;
            break;
        case WLR_EDGE_RIGHT:
            geom.x += geom.width - d.right;
            geom.width = d.right;
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
    if (!con->has_border) {
        return geom;
    }
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

// TODO refactor the name it doesn't represent what this does perfectly
// returns the newly accquired hidden edges
static enum wlr_edges container_update_hidden_edges(struct container *con, struct wlr_box *borders, enum wlr_edges hidden_edges)
{
    struct monitor *m = container_get_monitor(con);

    enum wlr_edges containers_hidden_edges = WLR_EDGE_NONE;
    struct wlr_box con_geom = container_get_current_geom(con);
    // int border_width = container_get_border_width(con);
    // hide edges if needed
    if (hidden_edges & WLR_EDGE_LEFT) {
        if (con_geom.x == m->root->geom.x) {
            containers_hidden_edges |= WLR_EDGE_LEFT;
        }
    }
    if (hidden_edges & WLR_EDGE_RIGHT) {
        if (is_approx_equal(con_geom.x + con_geom.width, m->root->geom.x + m->root->geom.width, 3)) {
            containers_hidden_edges |= WLR_EDGE_RIGHT;
        }
    }
    if (hidden_edges & WLR_EDGE_TOP) {
        if (con_geom.y == m->root->geom.y) {
            containers_hidden_edges |= WLR_EDGE_TOP;
        }
    }
    if (hidden_edges & WLR_EDGE_BOTTOM) {
        if (is_approx_equal(con_geom.y + con_geom.height, m->root->geom.y + m->root->geom.height, 3)) {
            containers_hidden_edges |= WLR_EDGE_BOTTOM;
        }
    }

    container_set_hidden_edges(con, containers_hidden_edges);
    return containers_hidden_edges;
}
void container_update_border(struct container *con)
{
    // optimization to not update the border if it's not needed
    container_update_border_geometry(con);
    container_update_border_color(con);
}

void container_update_border_geometry(struct container *con)
{
    container_update_border_visibility(con);
    // the visibility always has to be updated because has_border might have changed
    if (!con->has_border)
        return;

    struct scene_surface *surface = con->client->scene_surface;

    struct wlr_box *borders = (struct wlr_box[4]) {
        container_get_current_border_geom(con, WLR_EDGE_TOP),
        container_get_current_border_geom(con, WLR_EDGE_BOTTOM),
        container_get_current_border_geom(con, WLR_EDGE_LEFT),
        container_get_current_border_geom(con, WLR_EDGE_RIGHT),
    };

    for (int i = 0; i < BORDER_COUNT; i++) {
        struct wlr_scene_rect *border = surface->borders[i];
        struct wlr_box geom = borders[i];
        wlr_scene_node_set_position(&border->node, geom.x, geom.y);
        wlr_scene_rect_set_size(border, geom.width, geom.height);
    }
}

void container_update_border_color(struct container *con)
{
    if (!con->has_border)
        return;
    struct scene_surface *surface = con->client->scene_surface;

    struct monitor *m = container_get_monitor(con);
    struct tag *tag = monitor_get_active_tag(m);
    struct layout *lt = tag_get_layout(tag);

    struct container *sel = monitor_get_focused_container(m);
    const struct color color = (con == sel) ? lt->options->focus_color :
    lt->options->border_color;

    for (int i = 0; i < BORDER_COUNT; i++) {
        struct wlr_scene_rect *border = surface->borders[i];
        float border_color[4];
        color_to_wlr_color(border_color, color);
        wlr_scene_rect_set_color(border, border_color);
    }
}

void container_update_border_visibility(struct container *con)
{
    struct scene_surface *surface = con->client->scene_surface;

    if (!con->has_border) {
        for (int i = 0; i < BORDER_COUNT; i++) {
            struct wlr_scene_rect *border = surface->borders[i];
            wlr_scene_node_set_enabled(&border->node, false);
        }
        return;
    }

    struct monitor *m = container_get_monitor(con);
    struct tag *tag = monitor_get_active_tag(m);
    struct layout *lt = tag_get_layout(tag);

    struct wlr_box *borders = (struct wlr_box[4]) {
        container_get_current_border_geom(con, WLR_EDGE_TOP),
        container_get_current_border_geom(con, WLR_EDGE_BOTTOM),
        container_get_current_border_geom(con, WLR_EDGE_LEFT),
        container_get_current_border_geom(con, WLR_EDGE_RIGHT),
    };

    enum wlr_edges hidden_edges = WLR_EDGE_NONE;
    if (lt->options->smart_hidden_edges) {
        if (tag->visible_con_set->tiled_containers->len <= 1) {
            hidden_edges = container_update_hidden_edges(con, borders,
            lt->options->hidden_edges);
        }
    } else {
        hidden_edges = container_update_hidden_edges(con, borders,
        lt->options->hidden_edges);
    }

    for (int i = 0; i < BORDER_COUNT; i++) {
        struct wlr_scene_rect *border = surface->borders[i];

        bool is_hidden = hidden_edges & (1 << i);
        // hide or show the border
        wlr_scene_node_set_enabled(&border->node, !is_hidden);
    }
}

void resize_container(struct container *con, struct wlr_cursor *cursor, int offsetx, int offsety)
{
    if (!con)
        return;

    struct wlr_box geom = container_get_current_geom(con);

    geom.width = absolute_x_to_container_local(geom, cursor->x - offsetx);
    geom.height = absolute_y_to_container_local(geom, cursor->y - offsety);

    if (con->on_scratchpad) {
        remove_container_from_scratchpad(con);
    }
    if (container_is_tiled(con)) {
        container_set_floating(con, container_fix_position, true);
        arrange();
    }


    container_set_floating_geom(con, geom);
    struct tag *tag = container_get_current_tag(con);
    struct layout *lt = tag_get_layout(tag);
    container_set_border_width(con, direction_value_uniform(lt->options->float_border_px));

    container_update_size(con);
}

void container_resize_in_layout(
        struct container *con,
        struct wlr_cursor *cursor,
        int offsetx,
        int offsety,
        enum wlr_edges grabbed_edges)
{
    if (!con)
        return;
    if (con->on_scratchpad)
        return;
    if (container_is_floating(con))
        return;

    struct container_property *property = container_get_property(con);
    struct wlr_box geom = property->geom;
    // struct wlr_box geom = container_get_current_geom(con);
    printf("geom: %d %d %d %d\n", geom.x, geom.y, geom.width, geom.height);

    // geom.width = absolute_x_to_container_relative(geom, cursor->x - offsetx);
    // geom.height = absolute_y_to_container_relative(geom, cursor->y - offsety);
    struct monitor *m = xy_to_monitor(cursor->x, cursor->y);
    struct wlr_box m_geom = monitor_get_active_geom(m);
    int cursor_x = (cursor->x - offsetx)/m_geom.width*PERCENT_TO_INTEGER_SCALE;
    int cursor_y = (cursor->y - offsety)/m_geom.height*PERCENT_TO_INTEGER_SCALE;
    if (grabbed_edges == WLR_EDGE_LEFT) {
        int dx = (cursor_x) - geom.x;
        geom.x = cursor_x;
        geom.width -= dx;
    }
    if (grabbed_edges == WLR_EDGE_TOP) {
        int dy = (cursor_y) - geom.y;
        geom.y = cursor_y;
        geom.height -= dy;
    }
    if (grabbed_edges == WLR_EDGE_RIGHT) {
        geom.width = cursor_x - geom.x;
    }
    if (grabbed_edges == WLR_EDGE_BOTTOM) {
        geom.height = cursor_y - geom.y;
    }
    printf("new geom: %d %d %d %d\n", geom.x, geom.y, geom.width, geom.height);

    // struct wlr_box test_geom = {
    //     .x = 2000,
    //     .y = 2000,
    //     .width = 8000,
    //     .height = 8000,
    // };
    printf("grabbed edges: %d\n", grabbed_edges);
    resize_container_in_layout(con, geom);
}

void resize_container_in_layout(struct container *con, struct wlr_box geom)
{
    struct tag *tag = container_get_tag(con);
    struct layout *lt = tag_get_layout(tag);

    int position = get_position_in_container_stack(con);
    lua_getglobal(L, "Resize_container_in_layout");
    create_lua_layout(L, lt);
    lua_pushinteger(L, c_idx_to_lua_idx(position));
    create_lua_geometry(L, geom);

    if (lua_call_safe(L, 3, 1, 0) != LUA_OK) {
        return;
    }

    lua_copy_table_safe(L, &lt->lua_layout_copy_data_ref);
    arrange();
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

inline int absolute_x_to_container_local(struct wlr_box geom, int x)
{
    return x - geom.x;
}

inline int absolute_y_to_container_local(struct wlr_box geom, int y)
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

static int get_distance_squared(int x1, int y1, int x2, int y2)
{
    int dx = x1 - x2;
    int dy = y1 - y2;
    return dx * dx + dy * dy;
}

static bool is_coordinate_inside_container(struct container *con, int x, int y)
{
    struct wlr_box box = container_get_current_geom(con);
    return wlr_box_contains_point(&box, x, y);
}

static int minimum_of_four(int a, int b, int c, int d)
{
    int min = a;
    if (b < min)
        min = b;
    if (c < min)
        min = c;
    if (d < min)
        min = d;
    return min;
}

static enum wlr_edges geometry_get_nearest_edge_from_point(
        struct container *con,
        int x,
        int y
        )
{
    struct wlr_box left_geom = container_get_current_border_geom(con, WLR_EDGE_LEFT);
    struct wlr_box right_geom = container_get_current_border_geom(con, WLR_EDGE_RIGHT);
    struct wlr_box top_geom = container_get_current_border_geom(con, WLR_EDGE_TOP);
    struct wlr_box bottom_geom = container_get_current_border_geom(con, WLR_EDGE_BOTTOM);

    struct wlr_box box = container_get_current_geom(con);
    if (!wlr_box_contains_point(&box, x, y)) {
        // this false and shouldn't happen
        assert(false);
        return WLR_EDGE_NONE;
    }

    int left_edge_distance_squared = get_distance_squared(x, 0, left_geom.x + left_geom.width, 0);
    int right_edge_distance_squared = get_distance_squared(x, 0, right_geom.x, 0);
    int top_edge_distance_squared = get_distance_squared(0, y, 0, top_geom.y + top_geom.height);
    int bottom_edge_distance_squared = get_distance_squared(0, y, 0, bottom_geom.y);

    int min = minimum_of_four(left_edge_distance_squared,
            right_edge_distance_squared, top_edge_distance_squared,
            bottom_edge_distance_squared);

    if (min == left_edge_distance_squared) {
        printf("left\n");
        return WLR_EDGE_LEFT;
    } else if (min == right_edge_distance_squared) {
        printf("right\n");
        return WLR_EDGE_RIGHT;
    } else if (min == top_edge_distance_squared) {
        printf("top\n");
        return WLR_EDGE_TOP;
    } else if (min == bottom_edge_distance_squared) {
        printf("bottom\n");
        return WLR_EDGE_BOTTOM;
    } else {
        assert(false);
        return WLR_EDGE_NONE;
    }
    // TODO: finish this
    return WLR_EDGE_NONE;
}

void container_resize_with_cursor(struct cursor *cursor)
{
    int cursor_x = cursor->wlr_cursor->x;
    int cursor_y = cursor->wlr_cursor->y;
    struct wlr_cursor *wlr_cursor = cursor->wlr_cursor;
    struct container *con = xy_to_container(cursor_x, cursor_y);
    if (!con)
        return;
    enum wlr_edges edge = geometry_get_nearest_edge_from_point(con, cursor_x, cursor_y);
    struct wlr_box border = container_get_current_border_geom(con, edge);
    wlr_cursor_warp_closest(cursor->wlr_cursor, NULL,
            border.x + (double)border.width / 2,
            cursor->wlr_cursor->y);
    switch (edge) {
        case WLR_EDGE_LEFT:
            printf("left\n");
            wlr_xcursor_manager_set_cursor_image(cursor->xcursor_mgr,
                    "left_side", wlr_cursor);
            break;
        case WLR_EDGE_RIGHT:
            printf("right\n");
            wlr_xcursor_manager_set_cursor_image(cursor->xcursor_mgr,
                    "right_side", wlr_cursor);
            break;
        case WLR_EDGE_TOP:
            printf("top\n");
            wlr_xcursor_manager_set_cursor_image(cursor->xcursor_mgr,
                    "top_side", wlr_cursor);
            break;
        case WLR_EDGE_BOTTOM:
            wlr_xcursor_manager_set_cursor_image(cursor->xcursor_mgr,
                    "bottom_side", wlr_cursor);
            break;
        default:
            break;
    }
    cursor->cursor_mode = CURSOR_RESIZE_IN_LAYOUT;

    server.grab_c = con;
    server.grabbed_edges = edge;
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

bool container_is_viewable_on_own_monitor(struct container *con)
{
    struct monitor *m = server_get_selected_monitor();
    bool viewable = container_viewable_on_monitor(m, con);
    return viewable;
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

struct wlr_scene_node *container_get_scene_node(struct container *con)
{
    struct client *c = con->client;
    struct scene_surface *surface = c->scene_surface;
    return &surface->surface_tree->node;
}
