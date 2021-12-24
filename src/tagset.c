#include "tagset.h"

#include <assert.h>

#include "bitset/bitset.h"
#include "client.h"
#include "list_sets/container_stack_set.h"
#include "list_sets/list_set.h"
#include "server.h"
#include "tag.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "tile/tileUtils.h"
#include "ipc-server.h"
#include "monitor.h"
#include "cursor.h"
#include "scratchpad.h"
#include "list_sets/focus_stack_set.h"
#include "list_sets/container_stack_set.h"
#include "root.h"
#include "server.h"

static void tagset_assign_tag(struct tag *sel_ws, struct tag *tag, bool active);
static void tagset_unset_tag(struct tag *sel_ws, struct tag *tag);
static void tagset_set_tag(struct tag *sel_ws, struct tag *tag);
static void tag_damage(struct tag *tag);
static void tagset_assign_tags(struct tag *tag, BitSet *tags);

static bool tag_is_damaged(struct tag *tag);

static void tagset_tag_connect(struct tag *sel_ws, struct tag *tag);

static void tagset_assign_tag(struct tag *sel_ws, struct tag *tag, bool active)
{
    if (!sel_ws)
        return;
    int ws_id = tag->id;
    bitset_assign(sel_ws->tags, ws_id, active);
    tag_damage(sel_ws);
}

static void tagset_unset_tag(struct tag *sel_ws, struct tag *tag)
{
    tagset_assign_tag(sel_ws, tag, false);
}

static void tagset_set_tag(struct tag *sel_ws, struct tag *tag)
{
    tagset_assign_tag(sel_ws, tag, true);
}

static void tag_damage(struct tag *tag)
{
    tag->damaged = true;
}

static bool tag_is_damaged(struct tag *tag)
{
    if (!tag)
        return false;
    return tag->damaged;
}

static void tagset_assign_tags(struct tag *tag, BitSet *tags)
{
    tag_set_tags(tag, tags);
}

void tagset_set_tags(struct tag *sel_tag, BitSet *tags)
{
    // we need to copy to not accidentially destroy the same thing we are
    // working with which can happen through assign_bitset
    BitSet *tags_copy = bitset_copy(tags);

    tag_set_prev_tags(sel_tag, sel_tag->tags);

    tagset_tags_disconnect(sel_tag);
    tagset_assign_tags(sel_tag, tags);
    tagset_tags_connect(sel_tag);
    tag_damage(sel_tag);
    tagset_load_tags(sel_tag, sel_tag->tags);
    update_reduced_focus_stack(sel_tag);
    arrange();
    tag_focus_most_recent_container(sel_tag);

    bitset_destroy(tags_copy);
    ipc_event_tag();
}

// you should use tagset_write_to_tags to unload tags first else
void tagset_load_tags()
{
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);

        struct tag *sel_ws = monitor_get_active_tag(m);

        if (!tag_is_damaged(sel_ws))
            return;

        struct container_set *dest = sel_ws->visible_con_set;
        struct container_set *src = sel_ws->con_set;

        container_set_clear(dest);
        container_set_append(m, dest, src);
    }
}

// remove all references from tag to tagset and bounce a tag back to
// its original tagset or if already on it remove it from it
static void tagset_tag_disconnect(struct tag *sel_ws, struct tag *tag)
{
    if (!sel_ws)
        return;
    // ws->current_m = NULL;

    struct monitor *ws_m = tag_get_selected_monitor(tag);
    if (ws_m && ws_m != server_get_selected_monitor()) {
        tag_set_current_monitor(tag, tag_get_selected_monitor(tag));
        // move the tag back
        tagset_set_tag(sel_ws, tag);
    }
}

void tagset_tags_reconnect(struct tag *tag)
{
    tagset_load_tags();
    tagset_tags_connect(tag);
}

void tagset_tags_disconnect(struct tag *sel_ws)
{
    if (!sel_ws)
        return;

    tagset_tag_disconnect(sel_ws, sel_ws);

    for (GList *iter = g_hash_table_get_keys(sel_ws->tags->bytes);
            iter;
            iter = iter->next) {
        int ws_id = *(int *) iter->data;
        bool bit = g_hash_table_lookup(sel_ws->tags->bytes, &ws_id);

        if (!bit)
            continue;

        struct tag *tag = get_tag(ws_id);
        tagset_tag_disconnect(sel_ws, tag);
    }
}

static void tagset_tag_connect(struct tag *sel_ws, struct tag *tag)
{
    tag_set_current_monitor(tag, tag_get_selected_monitor(sel_ws));

    if (sel_ws->id == tag->id) {
        // ws->m = tag_get_monitor(sel_ws);
    }
}

void tagset_tags_connect(struct tag *sel_ws)
{
    for (GList *iter = g_hash_table_get_keys(sel_ws->tags->bytes);
            iter;
            iter = iter->next) {
        int ws_id = *(int *) iter->data;
        bool bit = g_hash_table_lookup(sel_ws->tags->bytes, &ws_id);


        if (!bit)
            continue;

        struct tag *tag = get_tag(ws_id);
        tagset_tag_connect(sel_ws, tag);
    }
}

void monitor_focus_tags(struct monitor *m, struct tag *tag, BitSet *tags)
{
    assert(tags != NULL);

    tag_set_selected_monitor(tag, m);

    push_tagset(tag, tags);
}

void tag_write_to_focus_stacks(struct tag *tag)
{
    if (!tag)
        return;

    focus_set_write_to_parent(tag->focus_set, tag->visible_focus_set);
}

bool is_reduced_focus_stack(struct tag *tag, struct container *con)
{
    struct monitor *m = tag_get_monitor(tag);
    bool viewable = container_viewable_on_monitor(m, con);
    bool visible = visible_on(m, tag->tags, con);
    if (viewable || visible) {
        return true;
    }
    return false;
}

bool _is_reduced_focus_stack(
        void *tag_ptr,
        GPtrArray *src_list,
        struct container *con
        )
{
    struct tag *tag = tag_ptr;
    struct monitor *m = tag_get_monitor(tag);
    bool viewable = container_viewable_on_monitor(m, con);
    bool visible = exist_on(m, tag->tags, con);
    if (viewable || visible) {
        return true;
    }
    return false;
}

void update_reduced_focus_stack(struct tag *tag)
{
    lists_clear(tag->visible_focus_set->focus_stack_lists);
    lists_append_list_under_condition(
            tag->visible_focus_set->focus_stack_lists,
            tag->focus_set->focus_stack_lists,
            _is_reduced_focus_stack,
            tag);
}

bool is_local_focus_stack(struct tag *tag, struct container *con)
{
    struct monitor *m = tag_get_monitor(tag);
    if (!container_is_managed(con)) {
        return false;
    }
    if (!exist_on(m, tag->tags, con)) {
        return false;
    }
    return true;
}

bool _is_local_focus_stack(
        void *tag_ptr,
        GPtrArray *src_list,
        struct container *con
        )
{
    struct tag *tag = tag_ptr;
    bool is_local = is_local_focus_stack(tag, con);
    return is_local;
}

bool is_visual_visible_stack(struct tag *tag, struct container *con)
{
    struct monitor *m = tag_get_monitor(tag);
    if (container_potentially_viewable_on_monitor(m, con)) {
        return true;
    }
    return false;
}

static bool _is_visual_visible_stack(
        void *tag_ptr,
        GPtrArray *src_list,
        struct container *con
        )
{
    struct tag *tag = tag_ptr;
    bool is_visible = is_visual_visible_stack(tag, con);
    return is_visible;
}

void tagset_move_sticky_containers(struct tag *tag)
{
    GPtrArray *list = list_create_filtered_sub_list(tag->con_set->tiled_containers, container_exists);
    for (int i = 0; i < list->len; i++) {
        struct container *con = g_ptr_array_index(list, i);
        container_move_sticky_containers(con, tag->id);
    }
    g_ptr_array_unref(list);
}

static void restore_floating_containers(struct tag *tag)
{
    GPtrArray *floating_list = tagset_get_floating_list_copy(tag);
    if (!floating_list)
        return;
    for (int i = 0; i < floating_list->len; i++) {
        struct container *con = g_ptr_array_index(floating_list, i);
        struct wlr_box *con_geom = container_get_current_geom(con);
        container_set_current_geom(con, con_geom);
    }
}

void focus_tagset(struct tag *tag, BitSet *tags)
{
    if(!tag)
        return;

    BitSet *tags_copy = bitset_copy(tags);

    tagset_assign_tags(tag, tags_copy);

    struct monitor *prev_m = server_get_selected_monitor();
    struct tag *prev_ws = monitor_get_active_tag(prev_m);

    struct monitor *ws_m = tag_get_selected_monitor(tag);
    struct monitor *m = ws_m ? ws_m : server_get_selected_monitor();
    monitor_set_selected_tag(m, tag);
    server_set_selected_monitor(m);

    if (prev_m == m && prev_ws != tag) {
        tagset_tags_disconnect(prev_ws);
    }
    tagset_tags_connect(tag);
    tag_damage(tag);
    tagset_load_tags();
    restore_floating_containers(tag);
    update_reduced_focus_stack(tag);
    ipc_event_tag();

    tagset_move_sticky_containers(tag);
    arrange();
    tag_focus_most_recent_container(tag);
    root_damage_whole(m->root);

    struct seat *seat = input_manager_get_default_seat();
    cursor_rebase(seat->cursor);

    bitset_destroy(tags_copy);
}

void tag_write_to_tags(struct tag *tag)
{
    if (!tag)
        return;

    container_set_write_to_parent(tag->con_set, tag->visible_con_set);
}

static void _set_previous_tagset(struct tag *tag)
{
    if (!tag)
        return;

    bitset_assign_bitset(&server.previous_bitset, tag->tags);
    server.previous_tag = tag->id;
}

void push_tagset(struct tag *sel_ws, BitSet *tags)
{
    // struct monitor *ws_m = tag_get_selected_monitor(sel_ws);
    // struct monitor *m = ws_m ? ws_m : server_get_selected_monitor();
    struct tag *tag = server_get_selected_tag();
    if (tag != sel_ws) {
        _set_previous_tagset(tag);
    }
    if (tag == sel_ws) { 
        tag_set_prev_tags(sel_ws, sel_ws->tags);
    }

    focus_tagset(sel_ws, tags);
}

void tagset_focus_tag(struct tag *tag)
{
    BitSet *tags = bitset_copy(tag->tags);
    tagset_focus_tags(tag, tags);
    bitset_destroy(tags);
}

void tagset_toggle_add(struct monitor *m, BitSet *bitset)
{
    if (!m)
        return;

    BitSet *new_bitset = bitset_copy(bitset);
    bitset_xor(new_bitset, monitor_get_tags(m));

    struct tag *tag = monitor_get_active_tag(m);
    tagset_set_tags(tag, new_bitset);

    bitset_destroy(new_bitset);
}

void tagset_focus_tags(struct tag *tag, struct BitSet *bitset)
{
    struct monitor *ws_m = tag_get_selected_monitor(tag);
    ws_m = ws_m ? ws_m : server_get_selected_monitor();
    monitor_focus_tags(ws_m, tag, bitset);
}

void tagset_reload(struct tag *sel_ws)
{
    if (!sel_ws)
        return;
    tag_damage(sel_ws);
    tagset_load_tags();
}

bool container_intersects_with_monitor(struct container *con, struct monitor *m)
{
    if (!con)
        return false;
    if (!m)
        return false;

    struct wlr_box tmp_geom;
    struct wlr_box *geom = container_get_current_geom(con);
    return wlr_box_intersection(&tmp_geom, geom, &m->geom);
}

GPtrArray *tagset_get_global_floating_copy(struct tag *tag)
{
    struct layout *lt = tag_get_layout(tag);

    GPtrArray *conditions = g_ptr_array_new();
    g_ptr_array_add(conditions, container_is_tiled_and_visible);
    g_ptr_array_add(conditions, container_is_floating_and_visible);

    GPtrArray *visible_global_floating_list_copy = NULL;
    if (lt->options->arrange_by_focus) {
        // TODO: FIXME
        visible_global_floating_list_copy =
            list_create_filtered_sub_list_with_order(
                tag->focus_set->focus_stack_normal,
                conditions
                );
    } else {
        visible_global_floating_list_copy =
            list_create_filtered_sub_list_with_order(
                tag->con_set->tiled_containers,
                conditions
                );
    }

    g_ptr_array_unref(conditions);

    return visible_global_floating_list_copy;
}

GPtrArray *tag_get_tiled_list_copy(struct tag *tag)
{
    struct layout *lt = tag_get_layout(tag);

    GPtrArray *tiled_list = NULL;
    if (lt->options->arrange_by_focus) {
        tiled_list = g_ptr_array_new();
        list_append_list_under_condition(
                tiled_list,
                tag->visible_focus_set->focus_stack_normal,
                _is_local_focus_stack,
                tag);
        return tiled_list;
    } else {
        tiled_list = list_create_filtered_sub_list(
                tag->visible_con_set->tiled_containers,
                container_is_tiled_and_managed);
        return tiled_list;
    }
}

GPtrArray *tag_get_tiled_list(struct tag *tag)
{
    struct layout *lt = tag_get_layout(tag);
    if (lt->options->arrange_by_focus) {
        return tag->visible_focus_set->focus_stack_normal;
    } else {
        return tag->visible_con_set->tiled_containers;
    }
}

GPtrArray *tagset_get_floating_list_copy(struct tag *tag)
{
    struct layout *lt = tag_get_layout(tag);

    if (!lt)
        return NULL;

    GPtrArray *floating_containers = g_ptr_array_new();
    if (lt->options->arrange_by_focus) {
        floating_containers =
            list_create_filtered_sub_list(
                    tag->visible_focus_set->focus_stack_normal,
                    container_is_floating);
    } else {
        floating_containers =
            list_create_filtered_sub_list(
                    tag->visible_con_set->tiled_containers,
                    container_is_floating);
    }
    return floating_containers;
}

GPtrArray *tagset_get_visible_list_copy(struct tag *tag)
{
    struct layout *lt = tag_get_layout(tag);

    GPtrArray *hidden_list = NULL;
    if (lt->options->arrange_by_focus) {
        hidden_list = list_create_filtered_sub_list(
                tag->visible_focus_set->focus_stack_normal,
                container_is_visible);
    } else {
        hidden_list = list_create_filtered_sub_list(
                tag->visible_con_set->tiled_containers,
                container_is_visible);
    }
    return hidden_list;
}

GPtrArray *tagset_get_hidden_list_copy(struct tag *tag)
{
    struct layout *lt = tag_get_layout(tag);

    if (lt->options->arrange_by_focus) {
        GPtrArray *hidden_list = list_create_filtered_sub_list(
                tag->visible_focus_set->focus_stack_normal,
                container_is_hidden);
        return hidden_list;
    } else {
        GPtrArray *hidden_list = list_create_filtered_sub_list(
                tag->visible_con_set->tiled_containers,
                container_is_hidden);
        return hidden_list;
    }
}

GPtrArray *tag_get_stack_copy(struct tag *tag)
{
    // TODO: replace with wlr_scene code
    // GPtrArray *tiled_copy = tag_get_tiled_list_copy(tag);
    // GPtrArray *floating_copy = list_create_filtered_sub_list(server.container_stack, container_is_floating);
    // wlr_list_cat(floating_copy, tiled_copy);
    // g_ptr_array_unref(tiled_copy);
    // return floating_copy;
    return NULL;
}

GPtrArray *tag_get_complete_stack_copy(struct tag *tag)
{
    // TODO replace with wlr_scene code
    // GPtrArray *array = g_ptr_array_new();
    // GPtrArray *stack_copy = tag_get_stack_copy(tag);
    //
    // wlr_list_cat(array, server.layer_visual_stack_overlay);
    // wlr_list_cat(array, server.layer_visual_stack_top);
    // wlr_list_cat(array, stack_copy);
    // wlr_list_cat(array, server.layer_visual_stack_bottom);
    // wlr_list_cat(array, server.layer_visual_stack_background);
    //
    // g_ptr_array_unref(stack_copy);
    return NULL;
}

void tag_id_to_tag(BitSet *dest, int ws_id)
{
    bitset_set(dest, ws_id);
}

bool tagset_contains_sticky_client(BitSet *tagset_tags, struct client *c)
{
    BitSet *bitset = bitset_copy(c->sticky_tags);
    bitset_and(bitset, tagset_tags);
    bool contains = bitset_any(bitset);
    bitset_destroy(bitset);
    return contains;
}

bool tagset_contains_client(BitSet *tags, struct client *c)
{
    if (tagset_contains_sticky_client(tags, c)) {
        return true;
    }

    BitSet *bitset = bitset_create();
    struct container *con = c->con;
    tag_id_to_tag(bitset, con->ws_id);
    bitset_and(bitset, tags);
    bool contains = bitset_any(bitset);
    bitset_destroy(bitset);
    return contains;
}

bool container_viewable_on_monitor(struct monitor *m, struct container *con)
{
    if (container_is_hidden(con))
        return false;
    bool intersects_with_monitor = container_intersects_with_monitor(con, m);
    if (!intersects_with_monitor) {
        return false;
    }

    return container_potentially_viewable_on_monitor(m, con);
}

// TODO refactor this function
bool container_potentially_viewable_on_monitor(struct monitor *m,
        struct container *con)
{
    struct tag *tag = monitor_get_active_tag(m);
    if (!tag)
        return false;
    bool visible = tagset_exist_on(m, con);
    if (visible)
        return true;

    bool is_floating = container_is_floating(con);
    if (!is_floating)
        return false;

    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct tag *sel_ws = monitor_get_active_tag(m);
        bool contains_client = tagset_contains_client(sel_ws->tags, con->client);
        if (contains_client) {
            return true;
        }
    }
    return false;
}

bool visible_on(struct monitor *m, BitSet *tags, struct container *con)
{
    if (!con)
        return false;
    if (container_get_hidden(con))
        return false;

    return exist_on(m, tags, con);
}

bool exist_on(struct monitor *m, BitSet *tags, struct container *con)
{
    if (!con)
        return false;
    if (!tags)
        return false;
    struct monitor *con_m = container_get_monitor(con);
    if (!con_m) {
        return false;
    }
    if (con_m != m) {
        return false;
    }

    struct client *c = con->client;

    if (!c)
        return false;

    if (c->type == LAYER_SHELL)
        return true;

    bool contains_client = tagset_contains_client(tags, c);
    return contains_client;
}

bool tagset_exist_on(struct monitor *m, struct container *con)
{
    if (!m)
        return false;
    if (!con)
        return false;
    struct tag *tag = monitor_get_active_tag(m);
    return exist_on(m, tag->tags, con);
}

bool tagset_visible_on(struct monitor *m, struct container *con)
{
    if (!m)
        return false;
    if (!con)
        return false;
    struct tag *tag = monitor_get_active_tag(m);
    return visible_on(m, tag->tags, con);
}
