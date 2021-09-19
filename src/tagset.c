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
#include "list_sets/visual_stack_set.h"
#include "list_sets/focus_stack_set.h"
#include "list_sets/container_stack_set.h"

static void tagset_assign_workspace(struct tagset *tagset, struct workspace *ws, bool load);
static void tagset_assign_workspaces(struct tagset *tagset, BitSet *workspaces);
static void tagset_set_tag(struct tagset *tagset, struct workspace *ws, bool load);
static void tagset_set_tags(struct tagset *tagset, BitSet *workspaces);
static void tagset_load_workspace(struct tagset *tagset, struct workspace *ws);
static void tagset_unload_workspace(struct tagset *tagset, struct workspace *ws);
static void tagset_remove_workspace(struct tagset *tagset, struct workspace *ws);

static void tagset_workspace_connect(struct tagset *tagset, struct workspace *ws);

static void tagset_subscribe_to_workspace(struct tagset *tagset, struct workspace *ws);

static void tagset_subscribe_to_workspace(struct tagset *tagset, struct workspace *ws)
{
    struct container_set *dest = tagset->list_set;
    struct workspace *sel_ws = get_workspace(tagset->selected_ws_id);
    struct container_set *src = sel_ws->list_set;

    container_set_append(ws, dest, src);
}

static void tagset_assign_workspace(struct tagset *tagset, struct workspace *ws, bool load)
{
    if (!tagset)
        return;

    bitset_assign(tagset->workspaces, ws->id, load);
}

static void tagset_assign_workspaces(struct tagset *tagset, BitSet *workspaces)
{
    tagset->workspaces = bitset_copy(workspaces);
}

static void tagset_set_tag(struct tagset *tagset, struct workspace *ws, bool load)
{
    if (!tagset)
        return;

    bitset_assign(tagset->workspaces, ws->id, load);
}

void tagset_set_tags(struct tagset *tagset, BitSet *bitset)
{
    tagset_workspaces_disconnect(tagset);
    tagset_assign_workspaces(tagset, bitset);
    tagset_workspaces_connect(tagset);
    struct workspace *ws = tagset_get_workspace(tagset);
    update_sub_focus_stack(ws);
    update_visual_visible_stack(ws);
}


// you should use tagset_write_to_workspace to unload workspaces first else
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
    tagset_remove_workspace(tagset, ws);
}

static void tagset_remove_workspace(struct tagset *tagset, struct workspace *ws)
{
    struct container_set *dest = tagset->list_set;
    struct container_set *src = ws->list_set;

    for (int i = 0; i < length_of_composed_list(src->container_lists); i++) {
        struct container *con = get_in_composed_list(src->container_lists, i);

        if (con->client->ws_id != ws->id)
            continue;

        for (int j = 0; j < dest->container_lists->len; j++) {
            GPtrArray *dest_list = g_ptr_array_index(dest->container_lists, j);
            g_ptr_array_remove(dest_list, con);
        }
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

    clear_list_set(tagset->list_set);

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
    struct tagset *tagset = calloc(1, sizeof(struct tagset));
    tagset->m = m;

    tagset->selected_ws_id = selected_ws_id;

    tagset->list_set = create_list_set();

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
    destroy_list_set(tagset->list_set);
    free(tagset);
}

void tagset_move_sticky_containers(struct tagset *old_tagset, struct tagset *tagset)
{
    if (!old_tagset)
        return;

    int len = length_of_composed_list(old_tagset->list_set->container_lists);
    int pos = len-1;
    for (int i = len-1; i >= 0; i--) {
        struct container *con = get_in_composed_list(old_tagset->list_set->container_lists, pos);
        container_move_sticky_containers(con);
        pos--;
    }
}

static void restore_floating_containers(struct tagset *tagset)
{
    GPtrArray *floating_list = tagset_get_floating_list(tagset);
    if (!floating_list)
        return;
    for (int i = 0; i < floating_list->len; i++) {
        struct container *con = g_ptr_array_index(floating_list, i);
        struct wlr_box *con_geom = container_get_geom(con);
        resize(con, *con_geom);
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
    tagset_move_sticky_containers(old_tagset, tagset);
    tagset_workspaces_disconnect(old_tagset);
    tagset_workspaces_connect(tagset);
    if (prev_m == m && old_tagset != tagset) {
        destroy_tagset(old_tagset);
    }
    m->tagset = tagset;
    restore_floating_containers(tagset);
    struct workspace *ws = tagset_get_workspace(tagset);
    ws->prev_workspaces = bitset_copy(tagset->workspaces);
    update_sub_focus_stack(ws);
    update_visual_visible_stack(ws);
    ipc_event_workspace();

    arrange();
    focus_most_recent_container(ws);
    root_damage_whole(m->root);

    struct seat *seat = input_manager_get_default_seat();
    cursor_rebase(seat->cursor);
}

static void tagset_append_to_workspaces(struct tagset *tagset)
{
    for (int i = 0; i < tagset->list_set->container_lists->len; i++) {
        GPtrArray *tagset_list = g_ptr_array_index(tagset->list_set->container_lists, i);
        for (int j = 0; j < tagset_list->len; j++) {
            struct container *con = g_ptr_array_index(tagset_list, j);
            struct workspace *ws = get_workspace(con->client->ws_id);
            GPtrArray *ws_list = g_ptr_array_index(ws->list_set->container_lists, i);

            g_ptr_array_add(ws_list, con);
        }
    }
}

static void tagset_clear_sel_workspace(struct tagset *tagset)
{
    for (int i = 0; i < tagset->loaded_workspaces->size; i++) {
        bool bit = bitset_test(tagset->loaded_workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        clear_list_set(ws->list_set);
    }
}

static void tagset_append_to_sel_workspace(struct tagset *tagset)
{
    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    for (int i = 0; i < tagset->list_set->container_lists->len; i++) {
        GPtrArray *tagset_list = g_ptr_array_index(tagset->list_set->container_lists, i);
        for (int j = 0; j < tagset_list->len; j++) {
            struct container *con = g_ptr_array_index(tagset_list, j);
            GPtrArray *ws_list = g_ptr_array_index(ws->list_set->container_lists, i);

            g_ptr_array_add(ws_list, con);
        }
    }
}

void tagset_write_to_workspaces(struct tagset *tagset)
{
    if (!tagset)
        return;
    printf("write to wroskapce\n");

    struct workspace *ws = tagset_get_workspace(tagset);
    GArray *positions = container_array2D_get_positions_array(tagset->list_set->container_lists);

    GArray *prev_positions = g_array_copy(positions);
    g_array_sort(prev_positions, cmp_int);

    GPtrArray *tiled_containers = g_ptr_array_new();
    wlr_list_cat(tiled_containers, ws->list_set->tiled_containers);

    for (int i = 0; i < prev_positions->len; i++) {
        int prev_position = g_array_index(prev_positions, int, i);
        int position = g_array_index(positions, int, i);
        struct container *prev_con = g_ptr_array_index(tiled_containers, position);
        g_ptr_array_index(ws->list_set->tiled_containers, prev_position) = prev_con;
    }

    g_ptr_array_free(tiled_containers, false);
    g_array_free(prev_positions, false);
    g_array_free(positions, false);
}


struct layout *tagset_get_layout(struct tagset *tagset)
{
    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    if (!ws)
        return NULL;
    return ws->layout;
}

static void _set_previous_tagset(struct tagset *tagset)
{
    if (!tagset)
        return;
    server.previous_bitset = bitset_copy(tagset->workspaces);
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
    struct workspace *ws = tagset_get_workspace(tagset);
    ws->prev_workspaces = bitset_copy(tagset->workspaces);
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
    return wlr_box_intersection(&tmp_geom, container_get_geom(con), &m->geom);
}

GPtrArray *server_update_floating_containers()
{
    list_clear(server.floating_containers, NULL);
    list_clear(server.floating_stack, NULL);
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct tagset *tagset = monitor_get_active_tagset(m);
        wlr_list_cat(server.floating_containers, tagset->list_set->floating_containers);

        struct workspace *ws = tagset_get_workspace(tagset);
        wlr_list_cat(server.floating_stack, ws->visible_visual_set->floating_visual_stack);
    }
    return server.floating_containers;
}

GPtrArray *tagset_get_global_floating_lists(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);
    struct workspace *ws = tagset_get_workspace(tagset);
    if (lt->options.arrange_by_focus) {
        // TODO this doesn't seem right
        return ws->visible_focus_set->focus_stack_visible_lists;
    } else {
        return tagset->list_set->global_floating_container_lists;
    }
}

GPtrArray *tagset_get_visible_lists(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus) {
        struct workspace *ws = tagset_get_workspace(tagset);
        return ws->local_focus_set->focus_stack_visible_lists;
    } else {
        return tagset->list_set->visible_container_lists;
    }
}

GPtrArray *tagset_get_tiled_list(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus) {
        struct workspace *ws = tagset_get_workspace(tagset);
        return ws->local_focus_set->focus_stack_normal;
    } else {
        return tagset->list_set->tiled_containers;
    }
}

GPtrArray *tagset_get_floating_list(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (!lt)
        return NULL;

    if (lt->options.arrange_by_focus) {
        struct workspace *ws = tagset_get_workspace(tagset);
        return ws->local_focus_set->focus_stack_normal;
    } else {
        return tagset->list_set->floating_containers;
    }
}

GPtrArray *tagset_get_hidden_list(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus) {
        struct workspace *ws = tagset_get_workspace(tagset);
        return ws->local_focus_set->focus_stack_hidden;
    } else {
        return tagset->list_set->hidden_containers;
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
        struct container *local_con = get_in_composed_list(ws->local_focus_set->focus_stack_lists, i);
        remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, local_con);
        update_sub_focus_stack(ws);
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
        update_sub_focus_stack(ws);
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
        list_set_add_container_to_focus_stack(ws, con);
        update_sub_focus_stack(ws);
    } else {
        g_ptr_array_insert(list, i, con);
    }
}

struct container *tagset_list_steal_index(GPtrArray *list, int i)
{
    if (list->len <= 0) {
        debug_print("list is empty\n");
        return NULL;
    }
    struct container *_con = g_ptr_array_index(list, 0);
    struct tagset *tagset = container_get_tagset(_con);
    struct layout *lt = tagset_get_layout(tagset);
    struct workspace *ws = tagset_get_workspace(tagset);

    struct container *con = NULL;
    if (lt->options.arrange_by_focus) {
        con = get_in_composed_list(ws->local_focus_set->focus_stack_lists, i);
        remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
        update_sub_focus_stack(ws);
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
    bool visible = tagset_visible_on(tagset, con);
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

bool visible_on(struct monitor *m, BitSet *workspaces, int i, struct container *con)
{
    if (!con)
        return false;
    if (con->hidden)
        return false;

    return exist_on(m, workspaces, i, con);
}

bool tagset_is_visible(struct tagset *tagset)
{
    if (!tagset)
        return false;
    assert(tagset->m != NULL);

    return tagset->m->tagset == tagset;
}

bool exist_on(struct monitor *m, BitSet *workspaces, int i, struct container *con)
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
    return exist_on(tagset->m, tagset->workspaces, tagset->selected_ws_id, con);
}

bool tagset_visible_on(struct tagset *tagset, struct container *con)
{
    if (!tagset)
        return false;
    if (!con)
        return false;
    return visible_on(tagset->m, tagset->workspaces, tagset->selected_ws_id, con);
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
