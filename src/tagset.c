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

static void tagset_assign_workspace(struct tag *sel_ws, struct tag *tag, bool active);
static void tagset_unset_workspace(struct tag *sel_ws, struct tag *tag);
static void tagset_set_workspace(struct tag *sel_ws, struct tag *tag);
static void workspace_damage(struct tag *tag);
static void tagset_assign_workspaces(struct tag *tag, BitSet *workspaces);

static bool workspace_is_damaged(struct tag *tag);

static void tagset_workspace_connect(struct tag *sel_ws, struct tag *tag);

static void tagset_assign_workspace(struct tag *sel_ws, struct tag *tag, bool active)
{
    if (!sel_ws)
        return;
    int ws_id = tag->id;
    bitset_assign(sel_ws->tags, ws_id, active);
    workspace_damage(sel_ws);
}

static void tagset_unset_workspace(struct tag *sel_ws, struct tag *tag)
{
    tagset_assign_workspace(sel_ws, tag, false);
}

static void tagset_set_workspace(struct tag *sel_ws, struct tag *tag)
{
    tagset_assign_workspace(sel_ws, tag, true);
}

static void workspace_damage(struct tag *tag)
{
    tag->damaged = true;
}

static bool workspace_is_damaged(struct tag *tag)
{
    if (!tag)
        return false;
    return tag->damaged;
}

static void tagset_assign_workspaces(struct tag *tag, BitSet *workspaces)
{
    workspace_set_tags(tag, workspaces);
}

void tagset_set_tags(struct tag *sel_ws, BitSet *workspaces)
{
    // we need to copy to not accidentially destroy the same thing we are
    // working with which can happen through assign_bitset
    BitSet *workspaces_copy = bitset_copy(workspaces);

    workspace_set_prev_tags(sel_ws, sel_ws->tags);

    tagset_workspaces_disconnect(sel_ws);
    tagset_assign_workspaces(sel_ws, workspaces);
    tagset_workspaces_connect(sel_ws);
    workspace_damage(sel_ws);
    tagset_load_workspaces(sel_ws, sel_ws->tags);
    update_reduced_focus_stack(sel_ws);
    arrange();
    focus_most_recent_container();

    bitset_destroy(workspaces_copy);
    ipc_event_workspace();
}

// you should use tagset_write_to_workspaces to unload workspaces first else
void tagset_load_workspaces()
{
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);

        struct tag *sel_ws = monitor_get_active_workspace(m);

        if (!workspace_is_damaged(sel_ws))
            return;

        struct container_set *dest = sel_ws->visible_con_set;
        struct container_set *src = sel_ws->con_set;

        container_set_clear(dest);
        container_set_append(m, dest, src);
    }
}

// remove all references from workspace to tagset and bounce a workspace back to
// its original tagset or if already on it remove it from it
static void tagset_workspace_disconnect(struct tag *sel_ws, struct tag *tag)
{
    if (!sel_ws)
        return;
    // ws->current_m = NULL;

    struct monitor *ws_m = workspace_get_selected_monitor(tag);
    if (ws_m && ws_m != server_get_selected_monitor()) {
        workspace_set_current_monitor(tag, workspace_get_selected_monitor(tag));
        // move the workspace back
        tagset_set_workspace(sel_ws, tag);
    }
}

void tagset_workspaces_reconnect(struct tag *tag)
{
    tagset_load_workspaces();
    tagset_workspaces_connect(tag);
}

void tagset_workspaces_disconnect(struct tag *sel_ws)
{
    if (!sel_ws)
        return;

    tagset_workspace_disconnect(sel_ws, sel_ws);

    for (GList *iter = g_hash_table_get_keys(sel_ws->tags->bytes);
            iter;
            iter = iter->next) {
        int ws_id = *(int *) iter->data;
        bool bit = g_hash_table_lookup(sel_ws->tags->bytes, &ws_id);

        if (!bit)
            continue;

        struct tag *tag = get_workspace(ws_id);
        tagset_workspace_disconnect(sel_ws, tag);
    }
}

static void tagset_workspace_connect(struct tag *sel_ws, struct tag *tag)
{
    workspace_set_current_monitor(tag, workspace_get_selected_monitor(sel_ws));

    if (sel_ws->id == tag->id) {
        // ws->m = workspace_get_monitor(sel_ws);
    }
}

void tagset_workspaces_connect(struct tag *sel_ws)
{
    for (GList *iter = g_hash_table_get_keys(sel_ws->tags->bytes);
            iter;
            iter = iter->next) {
        int ws_id = *(int *) iter->data;
        bool bit = g_hash_table_lookup(sel_ws->tags->bytes, &ws_id);


        if (!bit)
            continue;

        struct tag *tag = get_workspace(ws_id);
        tagset_workspace_connect(sel_ws, tag);
    }
}

void monitor_focus_tags(struct monitor *m, struct tag *tag, BitSet *workspaces)
{
    assert(workspaces != NULL);

    workspace_set_selected_monitor(tag, m);

    push_tagset(tag, workspaces);
}

void workspace_write_to_focus_stacks(struct tag *tag)
{
    if (!tag)
        return;

    focus_set_write_to_parent(tag->focus_set, tag->visible_focus_set);
}

bool is_reduced_focus_stack(struct tag *tag, struct container *con)
{
    struct monitor *m = workspace_get_monitor(tag);
    bool viewable = container_viewable_on_monitor(m, con);
    bool visible = visible_on(m, tag->tags, con);
    if (viewable || visible) {
        return true;
    }
    return false;
}

bool _is_reduced_focus_stack(
        void *workspace_ptr,
        GPtrArray *src_list,
        struct container *con
        )
{
    struct tag *tag = workspace_ptr;
    struct monitor *m = workspace_get_monitor(tag);
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
    struct monitor *m = workspace_get_monitor(tag);
    if (!container_is_managed(con)) {
        return false;
    }
    if (!exist_on(m, tag->tags, con)) {
        return false;
    }
    return true;
}

bool _is_local_focus_stack(
        void *workspace_ptr,
        GPtrArray *src_list,
        struct container *con
        )
{
    struct tag *tag = workspace_ptr;
    bool is_local = is_local_focus_stack(tag, con);
    return is_local;
}

bool is_visual_visible_stack(struct tag *tag, struct container *con)
{
    struct monitor *m = workspace_get_monitor(tag);
    if (container_potentially_viewable_on_monitor(m, con)) {
        return true;
    }
    return false;
}

static bool _is_visual_visible_stack(
        void *workspace_ptr,
        GPtrArray *src_list,
        struct container *con
        )
{
    struct tag *tag = workspace_ptr;
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

void focus_tagset(struct tag *tag, BitSet *workspaces)
{
    if(!tag)
        return;

    BitSet *workspaces_copy = bitset_copy(workspaces);

    tagset_assign_workspaces(tag, workspaces_copy);

    struct monitor *prev_m = server_get_selected_monitor();
    struct tag *prev_ws = monitor_get_active_workspace(prev_m);

    struct monitor *ws_m = workspace_get_selected_monitor(tag);
    struct monitor *m = ws_m ? ws_m : server_get_selected_monitor();
    monitor_set_selected_workspace(m, tag);
    server_set_selected_monitor(m);

    if (prev_m == m && prev_ws != tag) {
        tagset_workspaces_disconnect(prev_ws);
    }
    tagset_workspaces_connect(tag);
    workspace_damage(tag);
    tagset_load_workspaces();
    restore_floating_containers(tag);
    update_reduced_focus_stack(tag);
    ipc_event_workspace();

    tagset_move_sticky_containers(tag);
    arrange();
    focus_most_recent_container();
    root_damage_whole(m->root);

    struct seat *seat = input_manager_get_default_seat();
    cursor_rebase(seat->cursor);

    bitset_destroy(workspaces_copy);
}

void workspace_write_to_workspaces(struct tag *tag)
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
    server.previous_workspace = tag->id;
}

void push_tagset(struct tag *sel_ws, BitSet *workspaces)
{
    // struct monitor *ws_m = workspace_get_selected_monitor(sel_ws);
    // struct monitor *m = ws_m ? ws_m : server_get_selected_monitor();
    struct tag *tag = server_get_selected_workspace();
    if (tag != sel_ws) {
        _set_previous_tagset(tag);
    }
    if (tag == sel_ws) { 
        workspace_set_prev_tags(sel_ws, sel_ws->tags);
    }

    focus_tagset(sel_ws, workspaces);
}

void tagset_focus_workspace(struct tag *tag)
{
    BitSet *workspaces = bitset_copy(tag->tags);
    tagset_focus_tags(tag, workspaces);
    bitset_destroy(workspaces);
}

void tagset_toggle_add(struct monitor *m, BitSet *bitset)
{
    if (!m)
        return;

    BitSet *new_bitset = bitset_copy(bitset);
    bitset_xor(new_bitset, monitor_get_workspaces(m));

    struct tag *tag = monitor_get_active_workspace(m);
    tagset_set_tags(tag, new_bitset);

    bitset_destroy(new_bitset);
}

void tagset_focus_tags(struct tag *tag, struct BitSet *bitset)
{
    struct monitor *ws_m = workspace_get_selected_monitor(tag);
    ws_m = ws_m ? ws_m : server_get_selected_monitor();
    monitor_focus_tags(ws_m, tag, bitset);
}

void tagset_reload(struct tag *sel_ws)
{
    if (!sel_ws)
        return;
    workspace_damage(sel_ws);
    tagset_load_workspaces();
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
    struct layout *lt = workspace_get_layout(tag);

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

GPtrArray *workspace_get_tiled_list_copy(struct tag *tag)
{
    struct layout *lt = workspace_get_layout(tag);

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

GPtrArray *workspace_get_tiled_list(struct tag *tag)
{
    struct layout *lt = workspace_get_layout(tag);
    if (lt->options->arrange_by_focus) {
        return tag->visible_focus_set->focus_stack_normal;
    } else {
        return tag->visible_con_set->tiled_containers;
    }
}

GPtrArray *tagset_get_floating_list_copy(struct tag *tag)
{
    struct layout *lt = workspace_get_layout(tag);

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
    struct layout *lt = workspace_get_layout(tag);

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
    struct layout *lt = workspace_get_layout(tag);

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

GPtrArray *workspace_get_stack_copy(struct tag *tag)
{
    GPtrArray *tiled_copy = workspace_get_tiled_list_copy(tag);
    GPtrArray *floating_copy = list_create_filtered_sub_list(server.container_stack, container_is_floating);
    wlr_list_cat(floating_copy, tiled_copy);
    g_ptr_array_unref(tiled_copy);
    return floating_copy;
}

GPtrArray *workspace_get_complete_stack_copy(struct tag *tag)
{
    GPtrArray *array = g_ptr_array_new();
    GPtrArray *stack_copy = workspace_get_stack_copy(tag);

    wlr_list_cat(array, server.layer_visual_stack_overlay);
    wlr_list_cat(array, server.layer_visual_stack_top);
    wlr_list_cat(array, stack_copy);
    wlr_list_cat(array, server.layer_visual_stack_bottom);
    wlr_list_cat(array, server.layer_visual_stack_background);

    g_ptr_array_unref(stack_copy);
    return array;
}

void workspace_id_to_tag(BitSet *dest, int ws_id)
{
    bitset_set(dest, ws_id);
}

bool tagset_contains_sticky_client(BitSet *tagset_workspaces, struct client *c)
{
    BitSet *bitset = bitset_copy(c->sticky_workspaces);
    bitset_and(bitset, tagset_workspaces);
    bool contains = bitset_any(bitset);
    bitset_destroy(bitset);
    return contains;
}

bool tagset_contains_client(BitSet *workspaces, struct client *c)
{
    if (tagset_contains_sticky_client(workspaces, c)) {
        return true;
    }

    BitSet *bitset = bitset_create();
    struct container *con = c->con;
    workspace_id_to_tag(bitset, con->ws_id);
    bitset_and(bitset, workspaces);
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
    struct tag *tag = monitor_get_active_workspace(m);
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
        struct tag *sel_ws = monitor_get_active_workspace(m);
        bool contains_client = tagset_contains_client(sel_ws->tags, con->client);
        if (contains_client) {
            return true;
        }
    }
    return false;
}

bool visible_on(struct monitor *m, BitSet *workspaces, struct container *con)
{
    if (!con)
        return false;
    if (container_get_hidden(con))
        return false;

    return exist_on(m, workspaces, con);
}

bool exist_on(struct monitor *m, BitSet *workspaces, struct container *con)
{
    if (!con)
        return false;
    if (!workspaces)
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

    bool contains_client = tagset_contains_client(workspaces, c);
    return contains_client;
}

bool tagset_exist_on(struct monitor *m, struct container *con)
{
    if (!m)
        return false;
    if (!con)
        return false;
    struct tag *tag = monitor_get_active_workspace(m);
    return exist_on(m, tag->tags, con);
}

bool tagset_visible_on(struct monitor *m, struct container *con)
{
    if (!m)
        return false;
    if (!con)
        return false;
    struct tag *tag = monitor_get_active_workspace(m);
    return visible_on(m, tag->tags, con);
}
