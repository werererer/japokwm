#include <assert.h>

#include "bitset/bitset.h"
#include "client.h"
#include "list_sets/container_stack_set.h"
#include "list_sets/list_set.h"
#include "server.h"
#include "workspace.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "tile/tileUtils.h"
#include "ipc-server.h"
#include "monitor.h"
#include "cursor.h"
#include "scratchpad.h"
#include "list_sets/visual_stack_set.h"
#include "list_sets/focus_stack_set.h"
#include "list_sets/container_stack_set.h"
#include "workspace.h"
#include "tagset.h"
#include "workspace.h"
#include "root.h"

static void tagset_assign_workspaces(struct tagset *tagset, BitSet *workspaces);
static void tagset_set_tags(struct tagset *tagset, BitSet *workspaces);
static void tagset_load_workspace(struct tagset *tagset, struct workspace *ws);
static void tagset_unload_workspace(struct tagset *tagset, struct workspace *ws);

static void tagset_workspace_connect(struct tagset *tagset, struct workspace *ws);

static void tagset_subscribe_to_workspace(struct tagset *tagset, struct workspace *ws);

static void tagset_subscribe_to_workspace(struct tagset *tagset, struct workspace *ws)
{
    struct container_set *dest = tagset->con_set;
    struct workspace *sel_ws = tagset_get_workspace(tagset);
    struct container_set *src = sel_ws->con_set;

    container_set_append(ws, dest, src);
}

static void tagset_assign_workspaces(struct tagset *tagset, BitSet *workspaces)
{
     bitset_assign_bitset(&tagset->workspaces, workspaces);
}

void tagset_set_tags(struct tagset *tagset, BitSet *bitset)
{
    tagset_workspaces_disconnect(tagset);
    tagset_assign_workspaces(tagset, bitset);
    tagset_workspaces_connect(tagset);
    update_sub_focus_stack(tagset);
    update_visual_visible_stack(tagset);
    struct workspace *ws = tagset_get_workspace(tagset);
    bitset_assign_bitset(&ws->prev_workspaces, tagset->workspaces);
    arrange();
    focus_most_recent_container(ws);
}


// you should use tagset_write_to_workspaces to unload workspaces first else
void tagset_load_workspaces(struct tagset *tagset, BitSet *workspaces)
{
    assert(tagset != NULL);

    for (size_t i = 0; i < tagset->workspaces->size; i++) {
        bool bit = bitset_test(tagset->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        tagset_load_workspace(tagset, ws);
    }
}

static void tagset_clean_destroyed_tagset(struct tagset *tagset)
{
    if (!tagset)
        return;

    for (size_t i = 0; i < tagset->workspaces->size; i++) {
        struct workspace *ws = get_workspace(i);
        if (ws->tagset == tagset) {
            ws->tagset = NULL;
        }
        if (ws->selected_tagset == tagset) {
            ws->selected_tagset = NULL;
        }
    }
}

// remove all references from workspace to tagset and bounce a workspace back to
// its original tagset or if already on it remove it from it
static void tagset_workspace_disconnect(struct tagset *tagset, struct workspace *ws)
{
    if (!tagset)
        return;
    tagset_unload_workspace(tagset, ws);
    ws->tagset = NULL;
    if (ws->selected_tagset && ws->selected_tagset != tagset) {
        // move the workspace back
        ws->tagset = ws->selected_tagset;
        tagset_load_workspace(ws->tagset, ws);
    }
}

void tagset_workspaces_disconnect(struct tagset *tagset)
{
    if (!tagset)
        return;

    struct workspace *sel_ws = get_workspace(tagset->selected_ws_id);
    tagset_workspace_disconnect(sel_ws->tagset, sel_ws);

    for (size_t i = 0; i < tagset->workspaces->size; i++) {
        bool bit = bitset_test(tagset->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        tagset_workspace_disconnect(tagset, ws);
    }
}

static void tagset_workspace_connect(struct tagset *tagset, struct workspace *ws)
{
    ws->prev_m = tagset->m;

    tagset_unload_workspace(ws->tagset, ws);
    ws->tagset = tagset;
    if (tagset->selected_ws_id == ws->id) {
        ws->selected_tagset = tagset;
    }
    tagset_load_workspace(tagset, ws);
}

void tagset_workspaces_connect(struct tagset *tagset)
{
    if (!tagset)
        return;

    for (size_t i = 0; i < tagset->workspaces->size; i++) {
        bool bit = bitset_test(tagset->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        tagset_workspace_connect(tagset, ws);
    }
}

static void tagset_update_visible_tagset(struct tagset *tagset)
{
    for (size_t i = 0; i < tagset->workspaces->size; i++) {
        bool bit = bitset_test(tagset->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        ws->prev_m = tagset->m;
    }
}

static void tagset_load_workspace(struct tagset *tagset, struct workspace *ws)
{
    assert(tagset != NULL);
    assert(ws != NULL);
    bool bit = bitset_test(tagset->loaded_workspaces, ws->id);

    if (bit)
        return;

    bitset_set(tagset->loaded_workspaces, ws->id);
    tagset_subscribe_to_workspace(tagset, ws);
}

static void tagset_unsubscribe_from_workspace(struct tagset *tagset, struct workspace *ws)
{
    struct container_set *dest = tagset->con_set;
    struct container_set *src = ws->con_set;

    for (int i = 0; i < src->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(src->tiled_containers, i);

        if (con->client->ws_id != ws->id)
            continue;

        g_ptr_array_remove(dest->tiled_containers, con);
    }
}

static void tagset_unload_workspace(struct tagset *tagset, struct workspace *ws)
{
    if (!tagset)
        return;
    assert(ws != NULL);

    bool bit = bitset_test(tagset->loaded_workspaces, ws->id);
    if (!bit)
        return;

    bitset_reset(tagset->loaded_workspaces, ws->id);
    tagset_unsubscribe_from_workspace(tagset, ws);
}

void tagset_unload_workspaces(struct tagset *tagset)
{
    assert(tagset != NULL);

    container_set_clear(tagset->con_set);

    for (int i = 0; i < tagset->loaded_workspaces->size; i++) {
        bool bit = bitset_test(tagset->loaded_workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        tagset_unsubscribe_from_workspace(tagset, ws);
        bitset_reset(tagset->loaded_workspaces, i);
    }
}

struct tagset *create_tagset(struct monitor *m, int selected_ws_id, BitSet *workspaces)
{
    struct tagset *tagset = calloc(1, sizeof(*tagset));
    tagset->m = m;

    tagset->selected_ws_id = selected_ws_id;

    tagset->con_set = create_container_set();
    tagset->visible_focus_set = focus_set_create();
    tagset->local_focus_set = focus_set_create();
    tagset->visible_visual_set = visual_set_create();

    tagset->workspaces = bitset_create(server.workspaces->len);
    tagset->loaded_workspaces = bitset_create(server.workspaces->len);
    tagset_assign_workspaces(tagset, workspaces);

    g_ptr_array_add(server.tagsets, tagset);
    return tagset;
}

void destroy_tagset(struct tagset *tagset)
{
    if (!tagset)
        return;
    struct workspace *selected_workspace = get_workspace(tagset->selected_ws_id);
    if (selected_workspace->tagset == tagset) {
        selected_workspace->tagset = NULL;
    }
    if (selected_workspace->selected_tagset == tagset) {
        selected_workspace->selected_tagset = NULL;
    }

    tagset_clean_destroyed_tagset(tagset);

    g_ptr_array_remove(server.tagsets, tagset);
    bitset_destroy(tagset->workspaces);
    bitset_destroy(tagset->loaded_workspaces);
    destroy_container_set(tagset->con_set);
    focus_set_destroy(tagset->visible_focus_set);
    focus_set_destroy(tagset->local_focus_set);
    visual_set_destroy(tagset->visible_visual_set);
    free(tagset);
}

void tagset_write_to_focus_stacks(struct tagset *tagset)
{
    if (!tagset)
        return;

    struct workspace *ws = tagset_get_workspace(tagset);
    focus_set_write_to_parent(ws->focus_set, tagset->local_focus_set);
    update_reduced_focus_stack(tagset);
}

void update_sub_focus_stack(struct tagset *tagset)
{
    if (!tagset)
        return;
    update_reduced_focus_stack(tagset);
    update_local_focus_stack(tagset);
}

bool is_reduced_focus_stack(struct workspace *ws, struct container *con)
{
    struct monitor *m = workspace_get_monitor(ws);
    bool viewable = container_viewable_on_monitor(m, con);
    bool visible = visible_on(m, ws->prev_workspaces, con);
    if (viewable || visible) {
        return true;
    }
    return false;
}

bool _is_reduced_focus_stack(
        struct workspace *ws,
        GPtrArray *src_list,
        struct container *con
        )
{
    struct monitor *m = workspace_get_monitor(ws);
    bool viewable = container_viewable_on_monitor(m, con);
    bool visible = visible_on(m, ws->prev_workspaces, con);
    if (viewable || visible) {
        return true;
    }
    return false;
}

void update_reduced_focus_stack(struct tagset *tagset)
{
    struct workspace *ws = tagset_get_workspace(tagset);
    lists_clear(tagset->visible_focus_set->focus_stack_lists);
    lists_append_list_under_condition(
            tagset->visible_focus_set->focus_stack_lists,
            ws->focus_set->focus_stack_lists,
            _is_reduced_focus_stack,
            ws);
}

static bool is_local_focus_stack(
        struct workspace *ws,
        GPtrArray *src_list,
        struct container *con
        )
{
    struct monitor *m = workspace_get_monitor(ws);
    if (exist_on(m, ws->prev_workspaces, con)) {
        return true;
    }
    return false;
}

void update_local_focus_stack(struct tagset *tagset)
{
    struct workspace *ws = tagset_get_workspace(tagset);
    lists_clear(tagset->local_focus_set->focus_stack_lists);
    lists_append_list_under_condition(
            tagset->local_focus_set->focus_stack_lists,
            ws->focus_set->focus_stack_lists,
            is_local_focus_stack,
            ws);
}

static bool is_visual_visible_stack(
        struct workspace *ws,
        GPtrArray *src_list,
        struct container *con
        )
{
    struct monitor *m = workspace_get_monitor(ws);
    if (container_potentially_viewable_on_monitor(m, con)) {
        return true;
    }
    return false;
}

void update_visual_visible_stack(struct tagset *tagset)
{
    if (!tagset)
        return;
    struct workspace *ws = tagset_get_workspace(tagset);
    lists_clear(tagset->visible_visual_set->visual_stack_lists);
    lists_append_list_under_condition(
            tagset->visible_visual_set->visual_stack_lists,
            ws->visual_set->visual_stack_lists,
            is_visual_visible_stack,
            ws);
}

void tagset_move_sticky_containers(struct tagset *tagset)
{
    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    GPtrArray *list = list_create_filtered_sub_list(ws->con_set->tiled_containers, container_exists);
    for (int i = 0; i < list->len; i++) {
        struct container *con = g_ptr_array_index(list, i);
        container_move_sticky_containers(con, ws->id);
    }
    g_ptr_array_free(list, FALSE);
}

static void restore_floating_containers(struct tagset *tagset)
{
    GPtrArray *floating_list = tagset_get_floating_list_copy(tagset);
    if (!floating_list)
        return;
    for (int i = 0; i < floating_list->len; i++) {
        struct container *con = g_ptr_array_index(floating_list, i);
        struct wlr_box *con_geom = container_get_current_geom(con);
        container_set_current_geom(con, con_geom);
    }
}

void focus_tagset(struct tagset *tagset)
{
    if(!tagset)
        return;

    struct monitor *m = tagset->m;
    struct monitor *prev_m = selected_monitor;

    selected_monitor = m;

    struct tagset *old_tagset = m->tagset;
    tagset_workspaces_disconnect(old_tagset);
    tagset_workspaces_connect(tagset);
    if (prev_m == m && old_tagset != tagset) {
        destroy_tagset(old_tagset);
    }
    m->tagset = tagset;
    restore_floating_containers(tagset);
    struct workspace *ws = tagset_get_workspace(tagset);
    bitset_assign_bitset(&ws->prev_workspaces, tagset->workspaces);
    update_sub_focus_stack(tagset);
    update_visual_visible_stack(tagset);
    ipc_event_workspace();

    tagset_move_sticky_containers(tagset);
    arrange();
    focus_most_recent_container(ws);
    root_damage_whole(m->root);

    struct seat *seat = input_manager_get_default_seat();
    cursor_rebase(seat->cursor);
}

static void tagset_clear_sel_workspace(struct tagset *tagset)
{
    for (int i = 0; i < tagset->loaded_workspaces->size; i++) {
        bool bit = bitset_test(tagset->loaded_workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        container_set_clear(ws->con_set);
    }
}

void tagset_write_to_workspaces(struct tagset *tagset)
{
    if (!tagset)
        return;

    struct workspace *ws = tagset_get_workspace(tagset);
    container_set_write_to_parent(ws->con_set, tagset->con_set);
}


struct layout *tagset_get_layout(struct tagset *tagset)
{
    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    struct layout *lt = workspace_get_layout(ws);
    return lt;
}

static void _set_previous_tagset(struct tagset *tagset)
{
    if (!tagset)
        return;
    bitset_assign_bitset(&server.previous_bitset, tagset->workspaces);
    server.previous_workspace = tagset->selected_ws_id;
}

void push_tagset(struct tagset *tagset)
{
    struct monitor *m = selected_monitor;

    if (m->tagset != tagset) {
        _set_previous_tagset(m->tagset);
    }

    focus_tagset(tagset);
}

static void handle_too_few_workspaces(uint32_t ws_id)
{
    // no number has more than 11 digits when int is 32 bit long
    char name[12];
    // TODO explain why +1
    sprintf(name, "%d:%d", server.workspaces->len, server.workspaces->len+1);
    struct workspace *ws = create_workspace(name, server.workspaces->len, server.default_layout);
    g_ptr_array_add(server.workspaces, ws);
    for (int i = 0; i < server.tagsets->len; i++) {
        struct tagset *tagset = g_ptr_array_index(server.tagsets, i);
        bitset_push(tagset->workspaces, 0);
        bitset_push(tagset->loaded_workspaces, 0);
    }
}

void tagset_focus_workspace(int ws_id)
{
    if (ws_id >= server.workspaces->len) {
        handle_too_few_workspaces(ws_id);
    }

    BitSet *bitset = bitset_create(server.workspaces->len);
    bitset_set(bitset, ws_id);
    tagset_focus_tags(ws_id, bitset);
}

void tagset_toggle_add(struct tagset *tagset, BitSet *bitset)
{
    if (!tagset)
        return;

    BitSet *new_bitset = bitset_copy(bitset);
    bitset_xor(new_bitset, tagset->workspaces);

    tagset_set_tags(tagset, new_bitset);
    ipc_event_workspace();
}

void tagset_focus_tags(int ws_id, struct BitSet *bitset)
{
    struct workspace *ws = get_workspace(ws_id);
    struct monitor *ws_m = ws->selected_tagset ? ws->selected_tagset->m : NULL;
    struct monitor *m = ws_m ? ws_m : selected_monitor;

    struct tagset *tagset = create_tagset(m, ws_id, bitset);
    push_tagset(tagset);
}

void tagset_reload(struct tagset *tagset)
{
    if (!tagset)
        return;
    tagset_unload_workspaces(tagset);
    tagset_load_workspaces(tagset, tagset->workspaces);
}

bool container_intersects_with_monitor(struct container *con, struct monitor *m)
{
    if (!con)
        return false;
    if (!m)
        return false;

    struct wlr_box tmp_geom;
    return wlr_box_intersection(&tmp_geom, container_get_current_geom(con), &m->geom);
}

GPtrArray *server_update_floating_containers()
{
    list_clear(server.floating_containers, NULL);
    list_clear(server.floating_stack, NULL);
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct tagset *tagset = monitor_get_active_tagset(m);
        GPtrArray *floating_containers = tagset_get_floating_list_copy(tagset);
        wlr_list_cat(server.floating_containers, floating_containers);

        wlr_list_cat(server.floating_stack, tagset->visible_visual_set->floating_visual_stack);
    }
    return server.floating_containers;
}

GPtrArray *tagset_get_global_floating_copy(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    struct workspace *ws = tagset_get_workspace(tagset);

    GPtrArray *conditions = g_ptr_array_new();
    g_ptr_array_add(conditions, container_is_tiled_and_visible);
    g_ptr_array_add(conditions, container_is_floating);

    GPtrArray *visible_global_floating_list_copy = NULL;
    if (lt->options.arrange_by_focus) {
        // TODO: FIXME
        visible_global_floating_list_copy =
            list_create_filtered_sub_list_with_order(
                ws->focus_set->focus_stack_normal,
                conditions
                );
    } else {
        visible_global_floating_list_copy =
            list_create_filtered_sub_list_with_order(
                ws->con_set->tiled_containers,
                conditions
                );
    }

    g_ptr_array_free(conditions, FALSE);

    return visible_global_floating_list_copy;
}

GPtrArray *tagset_get_tiled_list_copy(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    GPtrArray *tiled_list = NULL;
    if (lt->options.arrange_by_focus) {
        tiled_list = list_create_filtered_sub_list(
                tagset->local_focus_set->focus_stack_normal,
                container_is_managed);
        return tiled_list;
    } else {
        tiled_list = list_create_filtered_sub_list(
                tagset->con_set->tiled_containers,
                container_is_tiled_and_managed);
        return tiled_list;
    }
}

GPtrArray *tagset_get_tiled_list(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus) {
        return tagset->local_focus_set->focus_stack_normal;
    } else {
        return tagset->con_set->tiled_containers;
    }
}

GPtrArray *tagset_get_floating_list_copy(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (!lt)
        return NULL;

    GPtrArray *floating_containers = g_ptr_array_new();
    if (lt->options.arrange_by_focus) {
        floating_containers =
            list_create_filtered_sub_list(
                    tagset->local_focus_set->focus_stack_normal,
                    container_is_floating);
    } else {
        floating_containers =
            list_create_filtered_sub_list(
                    tagset->con_set->tiled_containers,
                    container_is_floating);
    }
    return floating_containers;
}

GPtrArray *tagset_get_visible_list_copy(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    GPtrArray *hidden_list = NULL;
    if (lt->options.arrange_by_focus) {
        hidden_list = list_create_filtered_sub_list(
                tagset->local_focus_set->focus_stack_normal,
                container_is_visible);
    } else {
        hidden_list = list_create_filtered_sub_list(
                tagset->con_set->tiled_containers,
                container_is_visible);
    }
    return hidden_list;
}

GPtrArray *tagset_get_hidden_list_copy(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus) {
        GPtrArray *hidden_list = list_create_filtered_sub_list(
                tagset->local_focus_set->focus_stack_normal,
                container_is_hidden);
        return hidden_list;
    } else {
        GPtrArray *hidden_list = list_create_filtered_sub_list(
                tagset->con_set->tiled_containers,
                container_is_hidden);
        return hidden_list;
    }
}

void tagset_list_remove(GPtrArray *list, struct container *con)
{
    struct tagset *tagset = container_get_tagset(con);
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus) {
        struct workspace *ws = tagset_get_workspace(tagset);
        workspace_remove_container_from_focus_stack_locally(ws, con);
    } else {
        g_ptr_array_remove(list, con);
    }
}

void tagset_list_remove_index(GPtrArray *list, int i)
{
    if (list->len <= 0)
        return;
    struct container *con = g_ptr_array_index(list, 0);
    struct tagset *tagset = container_get_tagset(con);
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus) {
        struct workspace *ws = tagset_get_workspace(tagset);
        struct container *local_con = get_in_composed_list(tagset->local_focus_set->focus_stack_lists, i);
        remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, local_con);
        update_sub_focus_stack(tagset);
    } else {
        g_ptr_array_remove_index(list, i);
    }
}

void tagset_list_add(GPtrArray *list, struct container *con)
{
    struct tagset *tagset = container_get_tagset(con);
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus) {
        struct workspace *ws = tagset_get_workspace(tagset);
        list_set_append_container_to_focus_stack(ws, con);
        update_sub_focus_stack(tagset);
    } else {
        g_ptr_array_add(list, con);
    }
}

void tagset_list_insert(GPtrArray *list, int i, struct container *con)
{
    struct tagset *tagset = container_get_tagset(con);
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus) {
        struct workspace *ws = tagset_get_workspace(tagset);
        g_ptr_array_insert(ws->focus_set->focus_stack_normal, i, con);
        update_sub_focus_stack(tagset);
    } else {
        g_ptr_array_insert(list, i, con);
    }
}

struct container *tagset_list_steal_index(GPtrArray *list, int i)
{
    if (list->len <= 0) {
        return NULL;
    }
    struct container *_con = g_ptr_array_index(list, 0);
    struct tagset *tagset = container_get_tagset(_con);
    struct layout *lt = tagset_get_layout(tagset);
    struct workspace *ws = tagset_get_workspace(tagset);

    struct container *con = NULL;
    if (lt->options.arrange_by_focus) {
        con = get_in_composed_list(tagset->local_focus_set->focus_stack_lists, i);
        remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
        update_sub_focus_stack(tagset);
    } else {
        con = g_ptr_array_steal_index(list, i);
    }
    return con;
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
    if (tagset_contains_sticky_client(workspaces, c))
        return true;

    BitSet *bitset = bitset_create(server.workspaces->len);
    workspace_id_to_tag(bitset, c->ws_id);
    bitset_and(bitset, workspaces);
    bool contains = bitset_any(bitset);
    bitset_destroy(bitset);
    return contains;
}

bool container_viewable_on_monitor(struct monitor *m,
        struct container *con)
{
    if (container_is_hidden(con))
        return false;
    struct tagset *tagset = monitor_get_active_tagset(m);
    if (!tagset)
        return false;
    bool intersects_with_monitor =
        container_intersects_with_monitor(con, tagset->m);
    if (!intersects_with_monitor) {
        return false;
    }

    return container_potentially_viewable_on_monitor(m, con);
}

// TODO refactor this function
bool container_potentially_viewable_on_monitor(struct monitor *m,
        struct container *con)
{
    struct tagset *tagset = monitor_get_active_tagset(m);
    if (!tagset)
        return false;
    bool visible = tagset_exist_on(tagset, con);
    if (visible)
        return true;

    bool is_floating = container_is_floating(con);
    if (!is_floating)
        return false;

    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct tagset *tagset = monitor_get_active_tagset(m);
        bool contains_client = tagset_contains_client(tagset->workspaces, con->client);
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

bool tagset_is_visible(struct tagset *tagset)
{
    if (!tagset)
        return false;
    assert(tagset->m != NULL);

    return tagset->m->tagset == tagset;
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

    return tagset_contains_client(workspaces, c);
}

bool tagset_exist_on(struct tagset *tagset, struct container *con)
{
    if (!tagset)
        return false;
    if (!con)
        return false;
    return exist_on(tagset->m, tagset->workspaces, con);
}

bool tagset_visible_on(struct tagset *tagset, struct container *con)
{
    if (!tagset)
        return false;
    if (!con)
        return false;
    return visible_on(tagset->m, tagset->workspaces, con);
}

struct workspace *tagset_get_workspace(struct tagset *tagset)
{
    if (!tagset)
        return NULL;
    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    if (!ws)
        return NULL;
    return ws;
}
