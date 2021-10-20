#include "tagset.h"

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
#include "list_sets/focus_stack_set.h"
#include "list_sets/container_stack_set.h"
#include "workspace.h"
#include "workspace.h"
#include "root.h"

static void tagset_assign_workspace(struct workspace *sel_ws, struct workspace *ws, bool active);
static void tagset_unset_workspace(struct workspace *sel_ws, struct workspace *ws);
static void tagset_set_workspace(struct workspace *sel_ws, struct workspace *ws);
static void workspace_damage(struct workspace *ws);
static void tagset_assign_workspaces(struct workspace *ws, BitSet *workspaces);

static bool workspace_is_damaged(struct workspace *ws);

static void tagset_workspace_connect(struct workspace *sel_ws, struct workspace *ws);

static void tagset_assign_workspace(struct workspace *sel_ws, struct workspace *ws, bool active)
{
    if (!sel_ws)
        return;
    int ws_id = ws->id;
    bitset_assign(sel_ws->workspaces, ws_id, active);
    workspace_damage(sel_ws);
}

static void tagset_unset_workspace(struct workspace *sel_ws, struct workspace *ws)
{
    tagset_assign_workspace(sel_ws, ws, false);
}

static void tagset_set_workspace(struct workspace *sel_ws, struct workspace *ws)
{
    tagset_assign_workspace(sel_ws, ws, true);
}

static void workspace_damage(struct workspace *ws)
{
    ws->damaged = true;
}

static bool workspace_is_damaged(struct workspace *ws)
{
    if (!ws)
        return false;
    return ws->damaged;
}

static void tagset_assign_workspaces(struct workspace *ws, BitSet *workspaces)
{
    bitset_assign_bitset(&ws->workspaces, workspaces);
}

void tagset_set_tags(struct workspace *sel_ws, BitSet *workspaces)
{
    // we need to copy to not accidentially destroy the same thing we are
    // working with which can happen through assign_bitset
    BitSet *workspaces_copy = bitset_copy(workspaces);

    bitset_assign_bitset(&sel_ws->prev_workspaces, sel_ws->workspaces);

    tagset_workspaces_disconnect(sel_ws);
    tagset_assign_workspaces(sel_ws, workspaces_copy);
    tagset_workspaces_connect(sel_ws);
    workspace_damage(sel_ws);
    tagset_load_workspaces(sel_ws, sel_ws->workspaces);
    update_reduced_focus_stack(sel_ws);
    bitset_assign_bitset(&sel_ws->workspaces, sel_ws->workspaces);
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

        struct workspace *sel_ws = monitor_get_active_workspace(m);

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
static void tagset_workspace_disconnect(struct workspace *sel_ws, struct workspace *ws)
{
    if (!sel_ws)
        return;
    // ws->current_m = NULL;

    struct monitor *ws_m = workspace_get_selected_monitor(ws);
    if (ws_m && ws_m != server_get_selected_monitor()) {
        ws->current_m = workspace_get_selected_monitor(ws);
        // move the workspace back
        tagset_set_workspace(sel_ws, ws);
    }
}

void tagset_workspaces_reconnect(struct workspace *ws)
{
    tagset_load_workspaces();
    tagset_workspaces_connect(ws);
}

void tagset_workspaces_disconnect(struct workspace *sel_ws)
{
    if (!sel_ws)
        return;

    tagset_workspace_disconnect(sel_ws, sel_ws);

    for (size_t i = 0; i < sel_ws->workspaces->size; i++) {
        bool bit = bitset_test(sel_ws->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        tagset_workspace_disconnect(sel_ws, ws);
    }
}

static void tagset_workspace_connect(struct workspace *sel_ws, struct workspace *ws)
{
    ws->current_m = workspace_get_monitor(sel_ws);

    if (sel_ws->id == ws->id) {
        // ws->m = workspace_get_monitor(sel_ws);
    }
}

void tagset_workspaces_connect(struct workspace *sel_ws)
{
    for (size_t i = 0; i < sel_ws->workspaces->size; i++) {
        bool bit = bitset_test(sel_ws->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        tagset_workspace_connect(sel_ws, ws);
    }
}

void monitor_focus_tags(struct monitor *m, struct workspace *ws, BitSet *workspaces)
{
    assert(workspaces != NULL);

    workspace_set_selected_monitor(ws, m);
    tagset_assign_workspaces(ws, workspaces);

    push_tagset(ws);
}

void workspace_write_to_focus_stacks(struct workspace *ws)
{
    if (!ws)
        return;

    focus_set_write_to_parent(ws->focus_set, ws->visible_focus_set);
}

bool is_reduced_focus_stack(struct workspace *ws, struct container *con)
{
    struct monitor *m = workspace_get_monitor(ws);
    bool viewable = container_viewable_on_monitor(m, con);
    bool visible = visible_on(m, ws->workspaces, con);
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
    struct workspace *ws = workspace_ptr;
    struct monitor *m = workspace_get_monitor(ws);
    bool viewable = container_viewable_on_monitor(m, con);
    bool visible = exist_on(m, ws->workspaces, con);
    if (viewable || visible) {
        return true;
    }
    return false;
}

void update_reduced_focus_stack(struct workspace *ws)
{
    lists_clear(ws->visible_focus_set->focus_stack_lists);
    lists_append_list_under_condition(
            ws->visible_focus_set->focus_stack_lists,
            ws->focus_set->focus_stack_lists,
            _is_reduced_focus_stack,
            ws);
}

bool is_local_focus_stack(struct workspace *ws, struct container *con)
{
    struct monitor *m = workspace_get_monitor(ws);
    if (!container_is_managed(con)) {
        return false;
    }
    if (!exist_on(m, ws->workspaces, con)) {
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
    struct workspace *ws = workspace_ptr;
    bool is_local = is_local_focus_stack(ws, con);
    return is_local;
}

bool is_visual_visible_stack(struct workspace *ws, struct container *con)
{
    struct monitor *m = workspace_get_monitor(ws);
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
    struct workspace *ws = workspace_ptr;
    bool is_visible = is_visual_visible_stack(ws, con);
    return is_visible;
}

void tagset_move_sticky_containers(struct workspace *ws)
{
    GPtrArray *list = list_create_filtered_sub_list(ws->con_set->tiled_containers, container_exists);
    for (int i = 0; i < list->len; i++) {
        struct container *con = g_ptr_array_index(list, i);
        container_move_sticky_containers(con, ws->id);
    }
    g_ptr_array_unref(list);
}

static void restore_floating_containers(struct workspace *ws)
{
    GPtrArray *floating_list = tagset_get_floating_list_copy(ws);
    if (!floating_list)
        return;
    for (int i = 0; i < floating_list->len; i++) {
        struct container *con = g_ptr_array_index(floating_list, i);
        struct wlr_box *con_geom = container_get_current_geom(con);
        container_set_current_geom(con, con_geom);
    }
}

void focus_tagset(struct workspace *ws)
{
    if(!ws)
        return;

    struct monitor *prev_m = server_get_selected_monitor();
    struct workspace *prev_ws = monitor_get_active_workspace(prev_m);

    struct monitor *ws_m = workspace_get_selected_monitor(ws);
    struct monitor *m = ws_m ? ws_m : server_get_selected_monitor();
    monitor_set_selected_workspace(m, ws);
    server_set_selected_monitor(m);

    if (prev_m == m && prev_ws != ws) {
        tagset_workspaces_disconnect(prev_ws);
    }
    tagset_workspaces_connect(ws);
    workspace_damage(ws);
    tagset_load_workspaces();
    restore_floating_containers(ws);
    bitset_assign_bitset(&ws->workspaces, ws->workspaces);
    update_reduced_focus_stack(ws);
    ipc_event_workspace();

    tagset_move_sticky_containers(ws);
    arrange();
    focus_most_recent_container();
    root_damage_whole(m->root);

    struct seat *seat = input_manager_get_default_seat();
    cursor_rebase(seat->cursor);
}

void workspace_write_to_workspaces(struct workspace *ws)
{
    if (!ws)
        return;

    container_set_write_to_parent(ws->con_set, ws->visible_con_set);
}

static void _set_previous_tagset(struct workspace *sel_ws)
{
    if (!sel_ws)
        return;

    bitset_assign_bitset(&server.previous_bitset, sel_ws->workspaces);
    server.previous_workspace = sel_ws->id;
}

void push_tagset(struct workspace *sel_ws)
{
    // struct monitor *ws_m = workspace_get_selected_monitor(sel_ws);
    // struct monitor *m = ws_m ? ws_m : server_get_selected_monitor();
    struct workspace *ws = server_get_selected_workspace();
    // if (ws != sel_ws) {
    _set_previous_tagset(ws);
    // }

    focus_tagset(sel_ws);
}

static void handle_too_few_workspaces(uint32_t ws_id)
{
    // no number has more than 11 digits when int is 32 bit long
    char name[12];
    // TODO explain why +1
    sprintf(name, "%d:%d", server.workspaces->len, server.workspaces->len+1);
    while (ws_id >= server.workspaces->len)
    {
        struct workspace *new_ws = create_workspace(name, ws_id, server.default_layout);
        g_ptr_array_add(server.workspaces, new_ws);
        struct workspace *ws0 = get_workspace(0);
        wlr_list_cat(new_ws->con_set->tiled_containers, ws0->con_set->tiled_containers);

        wlr_list_cat(new_ws->focus_set->focus_stack_layer_background, ws0->focus_set->focus_stack_layer_background);
        wlr_list_cat(new_ws->focus_set->focus_stack_layer_bottom, ws0->focus_set->focus_stack_layer_bottom);
        wlr_list_cat(new_ws->focus_set->focus_stack_layer_top, ws0->focus_set->focus_stack_layer_top);
        wlr_list_cat(new_ws->focus_set->focus_stack_layer_overlay, ws0->focus_set->focus_stack_layer_overlay);
        wlr_list_cat(new_ws->focus_set->focus_stack_on_top, ws0->focus_set->focus_stack_on_top);
        wlr_list_cat(new_ws->focus_set->focus_stack_normal, ws0->focus_set->focus_stack_normal);
        wlr_list_cat(new_ws->focus_set->focus_stack_not_focusable, ws0->focus_set->focus_stack_not_focusable);
    }
}

void tagset_focus_workspace(struct monitor *m, struct workspace *ws)
{
    BitSet *workspaces = bitset_copy(ws->workspaces);
    tagset_focus_tags(ws, workspaces);
    bitset_destroy(workspaces);
}

void tagset_toggle_add(struct monitor *m, BitSet *bitset)
{
    if (!m)
        return;

    BitSet *new_bitset = bitset_copy(bitset);
    bitset_xor(new_bitset, monitor_get_workspaces(m));

    struct workspace *ws = monitor_get_active_workspace(m);
    tagset_set_tags(ws, new_bitset);

    bitset_destroy(new_bitset);
}

void tagset_focus_tags(struct workspace *ws, struct BitSet *bitset)
{
    struct monitor *ws_m = workspace_get_selected_monitor(ws);
    ws_m = ws_m ? ws_m : server_get_selected_monitor();
    monitor_focus_tags(ws_m, ws, bitset);
}

void tagset_reload(struct workspace *sel_ws)
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
    return wlr_box_intersection(&tmp_geom, container_get_current_geom(con), &m->geom);
}

GPtrArray *tagset_get_global_floating_copy(struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);

    GPtrArray *conditions = g_ptr_array_new();
    g_ptr_array_add(conditions, container_is_tiled_and_visible);
    g_ptr_array_add(conditions, container_is_floating_and_visible);

    GPtrArray *visible_global_floating_list_copy = NULL;
    if (lt->options->arrange_by_focus) {
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

    g_ptr_array_unref(conditions);

    return visible_global_floating_list_copy;
}

GPtrArray *workspace_get_tiled_list_copy(struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);

    GPtrArray *tiled_list = NULL;
    if (lt->options->arrange_by_focus) {
        tiled_list = g_ptr_array_new();
        list_append_list_under_condition(
                tiled_list,
                ws->focus_set->focus_stack_normal,
                _is_local_focus_stack,
                ws);
        return tiled_list;
    } else {
        tiled_list = list_create_filtered_sub_list(
                ws->visible_con_set->tiled_containers,
                container_is_tiled_and_managed);
        return tiled_list;
    }
}

GPtrArray *workspace_get_tiled_list(struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);
    if (lt->options->arrange_by_focus) {
        return ws->visible_focus_set->focus_stack_normal;
    } else {
        return ws->visible_con_set->tiled_containers;
    }
}

GPtrArray *tagset_get_floating_list_copy(struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);

    if (!lt)
        return NULL;

    GPtrArray *floating_containers = g_ptr_array_new();
    if (lt->options->arrange_by_focus) {
        floating_containers =
            list_create_filtered_sub_list(
                    ws->visible_focus_set->focus_stack_normal,
                    container_is_floating);
    } else {
        floating_containers =
            list_create_filtered_sub_list(
                    ws->visible_con_set->tiled_containers,
                    container_is_floating);
    }
    return floating_containers;
}

GPtrArray *tagset_get_visible_list_copy(struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);

    GPtrArray *hidden_list = NULL;
    if (lt->options->arrange_by_focus) {
        hidden_list = list_create_filtered_sub_list(
                ws->visible_focus_set->focus_stack_normal,
                container_is_visible);
    } else {
        hidden_list = list_create_filtered_sub_list(
                ws->visible_con_set->tiled_containers,
                container_is_visible);
    }
    return hidden_list;
}

GPtrArray *tagset_get_hidden_list_copy(struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);

    if (lt->options->arrange_by_focus) {
        GPtrArray *hidden_list = list_create_filtered_sub_list(
                ws->visible_focus_set->focus_stack_normal,
                container_is_hidden);
        return hidden_list;
    } else {
        GPtrArray *hidden_list = list_create_filtered_sub_list(
                ws->visible_con_set->tiled_containers,
                container_is_hidden);
        return hidden_list;
    }
}

GPtrArray *workspace_get_stack_copy(struct workspace *ws)
{
    GPtrArray *tiled_copy = workspace_get_tiled_list_copy(ws);
    GPtrArray *floating_copy = list_create_filtered_sub_list(server.container_stack, container_is_floating);
    wlr_list_cat(floating_copy, tiled_copy);
    g_ptr_array_unref(tiled_copy);
    return floating_copy;
}

GPtrArray *workspace_get_complete_stack_copy(struct workspace *ws)
{
    GPtrArray *array = g_ptr_array_new();
    GPtrArray *stack_copy = workspace_get_stack_copy(ws);

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
    struct workspace *ws = monitor_get_active_workspace(m);
    if (!ws)
        return false;
    bool visible = tagset_exist_on(m, con);
    if (visible)
        return true;

    bool is_floating = container_is_floating(con);
    if (!is_floating)
        return false;

    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct workspace *sel_ws = monitor_get_active_workspace(m);
        bool contains_client = tagset_contains_client(sel_ws->workspaces, con->client);
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
    struct workspace *ws = monitor_get_active_workspace(m);
    return exist_on(m, ws->workspaces, con);
}

bool tagset_visible_on(struct monitor *m, struct container *con)
{
    if (!m)
        return false;
    if (!con)
        return false;
    struct workspace *ws = monitor_get_active_workspace(m);
    return visible_on(m, ws->workspaces, con);
}
