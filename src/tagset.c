#include "tagset.h"

#include <assert.h>

#include "bitset/bitset.h"
#include "list_set.h"
#include "server.h"
#include "workspace.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "tile/tileUtils.h"
#include "ipc-server.h"
#include "monitor.h"
#include "cursor.h"

static void tagset_write_to_workspaces(struct tagset *tagset);
static void tagset_assign_workspace(struct tagset *tagset, struct workspace *ws, bool load);
static void tagset_assign_workspaces(struct tagset *tagset, BitSet *workspaces);
static void tagset_set_tag(struct tagset *tagset, struct workspace *ws, bool load);
static void tagset_set_tags(struct tagset *tagset, BitSet *workspaces);
static void tagset_load_workspaces(struct tagset *tagset, BitSet *workspaces);
static void tagset_load_workspace(struct tagset *tagset, struct workspace *ws);
static void tagset_unload_workspaces(struct tagset *tagset);
static void tagset_unload_workspace(struct tagset *tagset, struct workspace *ws);
static void tagset_clear_workspaces(struct tagset *tagset);
static void tagset_workspaces_disconnect(struct tagset *tagset);
static void tagset_workspaces_connect(struct tagset *tagset);

static void tagset_workspace_connect(struct tagset *tagset, struct workspace *ws);

static void tagset_subscribe_to_workspace(struct tagset *tagset, struct workspace *ws);

static void tagset_append_list_sets(struct tagset *tagset, struct workspace *ws)
{
    struct list_set *dest = tagset->list_set;
    struct workspace *sel_ws = get_workspace(tagset->selected_ws_id);
    struct list_set *src = sel_ws->list_set;
    debug_print("start tagset_append_list_sets\n");
    debug_print("sel workspace: %i\n", sel_ws->id);

    for (int i = 0; i < dest->all_lists->len; i++) {
        GPtrArray *dest_list = g_ptr_array_index(dest->all_lists, i);
        GPtrArray *src_list = g_ptr_array_index(src->all_lists, i);
        debug_print("src_list length: %i\n", src_list->len);
        for (int j = 0; j < src_list->len; j++) {
            struct container *src_con = g_ptr_array_index(src_list, j);
            if (src_con->client->ws_id != ws->id) {
                debug_print("con: %p not on ws: %i\n", src_con, ws->id);
                continue;
            }
            guint src_pos;
            if (!g_ptr_array_find(src_list, src_con, &src_pos)) {
                debug_print("not found1\n");
                continue;
            }

            bool added = false;
            debug_print("works0\n");
            if (src_pos == 0) {
                g_ptr_array_insert(dest_list, 0, src_con);
                added = true;
            } else {
                for (int k = 0; k < dest_list->len; k++) {
                    struct container *dest_con = g_ptr_array_index(dest_list, k);
                    guint dest_pos;
                    if (!g_ptr_array_find(src_list, dest_con, &dest_pos)) {
                        debug_print("not found1\n");
                        continue;
                    }

                    debug_print("src_pos: %i dest_pos: %i\n", src_pos, dest_pos);
                    if (src_pos < dest_pos) {
                        debug_print("don't end src_pos: %i dest_pos: %i\n", src_pos, dest_pos);
                        continue;
                    }

                    debug_print("works1\n");
                    guint final_pos;
                    bool final_found = g_ptr_array_find(dest_list, dest_con, &final_pos);
                    assert(final_found == true);
                    final_pos++;

                    debug_print("add container: %p to tagset: %p\n", src_con, tagset);
                    debug_print("pos: %i\n", final_pos);

                    g_ptr_array_insert(dest_list, final_pos, src_con);
                    added = true;
                    break;
                }
            }

            if (!added) {
                debug_print("add the boring way: %p\n", src_con);
                g_ptr_array_add(dest_list, src_con);
                added = true;
            }
        }
    }
    debug_print("end tagset append list_sets\n");
}

static void tagset_subscribe_to_workspace(struct tagset *tagset, struct workspace *ws)
{
    g_ptr_array_add(ws->subscribed_tagsets, tagset);
    tagset_append_list_sets(tagset, ws);
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
    /* tagset_write_to_workspaces(tagset); */
    tagset_workspaces_disconnect(tagset);
    tagset_assign_workspaces(tagset, bitset);
    tagset_workspaces_connect(tagset);
}


// you should use tagset_write_to_workspace to unload workspaces first else
static void tagset_load_workspaces(struct tagset *tagset, BitSet *workspaces)
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
    if (ws->selected_tagset != tagset && ws->selected_tagset) {
        // move the workspace back
        ws->tagset = ws->selected_tagset;
        tagset_load_workspace(ws->tagset, ws);
    }
}

static void tagset_workspaces_disconnect(struct tagset *tagset)
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

static void tagset_workspaces_connect(struct tagset *tagset)
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
    g_ptr_array_remove(ws->subscribed_tagsets, tagset);
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
    list_set_remove_list_set(tagset->list_set, ws->list_set);
    tagset_unsubscribe_from_workspace(tagset, ws);
}

static void tagset_unload_workspaces(struct tagset *tagset)
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
    struct tagset *tagset = g_rc_box_new0(struct tagset);
    tagset->m = m;

    tagset->selected_ws_id = selected_ws_id;

    tagset->list_set = create_list_set();

    tagset->workspaces = bitset_create(server.workspaces->len);
    tagset->loaded_workspaces = bitset_create(server.workspaces->len);
    tagset_set_tags(tagset, workspaces);

    g_ptr_array_add(server.tagsets, tagset);
    return tagset;
}

static void _destroy_tagset(void *tagset_ptr)
{
    if (!tagset_ptr)
        return;

    struct tagset *tagset = (struct tagset *)tagset_ptr;

    struct workspace *selected_workspace = get_workspace(tagset->selected_ws_id);
    if (selected_workspace->selected_tagset == tagset) {
        selected_workspace->selected_tagset = NULL;
    }

    tagset_clean_destroyed_tagset(tagset);

    g_ptr_array_remove(server.tagsets, tagset);
    bitset_destroy(tagset->workspaces);
    bitset_destroy(tagset->loaded_workspaces);
    destroy_list_set(tagset->list_set);
}

void tagset_acquire(struct tagset *tagset)
{
    if (!tagset)
        return;
    g_rc_box_acquire(tagset);
}

void tagset_release(struct tagset *tagset)
{
    if (!tagset)
        return;
    g_rc_box_release_full(tagset, _destroy_tagset);
}

void focus_most_recent_container(struct tagset *tagset)
{
    struct container *con = get_in_composed_list(tagset->list_set->focus_stack_lists, 0);

    if (!con) {
        con = get_container(tagset, 0);
        if (!con) {
            ipc_event_window();
            return;
        }
    }

    focus_container(con);
}

static void tagset_move_sticky_containers(struct tagset *old_tagset, struct tagset *tagset)
{
    if (!old_tagset)
        return;

    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    int len = length_of_composed_list(old_tagset->list_set->container_lists);
    int pos = len-1;
    for (int i = len-1; i >= 0; i--) {
        struct container *con = get_in_composed_list(old_tagset->list_set->container_lists, pos);
        if (con->client->sticky) {
            debug_print("move container: %p\n", con);
            move_container_to_workspace(con, ws);
        }
        pos--;
    }
}

void focus_tagset_no_ref(struct tagset *tagset)
{
    if(!tagset)
        return;

    struct monitor *m = tagset->m;

    focus_monitor(m);
    selected_monitor = m;

    struct tagset *old_tagset = m->tagset;

    /* tagset_write_to_workspaces(m->tagset); */
    tagset_move_sticky_containers(old_tagset, tagset);
    tagset_workspaces_disconnect(old_tagset);
    tagset_workspaces_connect(tagset);
    tagset_release(m->tagset);
    m->tagset = tagset;
    ipc_event_workspace();

    arrange();
    focus_most_recent_container(tagset);
    root_damage_whole(m->root);

    struct seat *seat = input_manager_get_default_seat();
    cursor_rebase(seat->cursor);
}

void focus_tagset(struct tagset *tagset)
{
    if(!tagset)
        return;

    tagset_acquire(tagset);
    focus_tagset_no_ref(tagset);
}

static void tagset_clear_workspaces(struct tagset *tagset)
{
    for (int i = 0; i < tagset->loaded_workspaces->size; i++) {
        bool bit = bitset_test(tagset->loaded_workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        clear_list_set(ws->list_set);
    }
}

static void tagset_append_to_workspaces(struct tagset *tagset)
{
    for (int i = 0; i < tagset->list_set->all_lists->len; i++) {
        GPtrArray *tagset_list = g_ptr_array_index(tagset->list_set->all_lists, i);
        for (int j = 0; j < tagset_list->len; j++) {
            struct container *con = g_ptr_array_index(tagset_list, j);
            struct workspace *ws = get_workspace(con->client->ws_id);
            GPtrArray *ws_list = g_ptr_array_index(ws->list_set->all_lists, i);

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
    for (int i = 0; i < tagset->list_set->all_lists->len; i++) {
        GPtrArray *tagset_list = g_ptr_array_index(tagset->list_set->all_lists, i);
        for (int j = 0; j < tagset_list->len; j++) {
            struct container *con = g_ptr_array_index(tagset_list, j);
            GPtrArray *ws_list = g_ptr_array_index(ws->list_set->all_lists, i);

            g_ptr_array_add(ws_list, con);
        }
    }
}

static void tagset_write_to_workspaces(struct tagset *tagset)
{
    if (!tagset)
        return;

    /* tagset_clear_sel_workspace(tagset); */
    /* tagset_append_to_sel_workspace(tagset); */
}

struct layout *tagset_get_layout(struct tagset *tagset)
{
    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    return ws->layout;
}

static void _set_previous_tagset(struct tagset *tagset)
{
    tagset_acquire(tagset);
    tagset_release(server.previous_tagset);
    server.previous_tagset = tagset;
}

void push_tagset_no_ref(struct tagset *tagset)
{
    struct monitor *m = selected_monitor;

    if (m->tagset != tagset) {
        _set_previous_tagset(m->tagset);
    }

    focus_tagset_no_ref(tagset);
}

void push_tagset(struct tagset *tagset)
{
    if (!tagset)
        return;
    tagset_acquire(tagset);

    push_tagset_no_ref(tagset);
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
    struct monitor *ws_m = workspace_get_monitor(ws);
    struct monitor *m = ws_m ? ws_m : selected_monitor;

    struct tagset *tagset = workspace_get_selected_tagset(ws);
    if (tagset) {
        tagset_set_tags(tagset, bitset);
        push_tagset(tagset);
    } else {
        tagset = create_tagset(m, ws_id, bitset);
        push_tagset_no_ref(tagset);
    }
}

struct container *get_container(struct tagset *tagset, int i)
{
    struct list_set *list_set = tagset->list_set;
    if (!list_set)
        return NULL;

    return get_in_composed_list(list_set->container_lists, i);
}

static bool container_intersects_with_monitor(struct container *con, struct monitor *m)
{
    if (!con)
        return false;
    if (!m)
        return false;

    struct wlr_box tmp_geom;
    return wlr_box_intersection(&tmp_geom, &con->geom, &m->geom);
}

GPtrArray *tagset_get_visible_lists(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus)
        return tagset->list_set->focus_stack_visible_lists;
    else
        return tagset->list_set->visible_container_lists;
}

GPtrArray *tagset_get_tiled_list(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus)
        return tagset->list_set->focus_stack_normal;
    else
        return tagset->list_set->tiled_containers;
}

GPtrArray *tagset_get_floating_list(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus)
        return tagset->list_set->focus_stack_normal;
    else
        return tagset->list_set->floating_containers;
}

GPtrArray *tagset_get_hidden_list(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus)
        return tagset->list_set->focus_stack_hidden;
    else
        return tagset->list_set->hidden_containers;
}

void workspace_id_to_tag(BitSet *dest, int ws_id)
{
    bitset_set(dest, ws_id);
}

bool tagset_contains_client(struct tagset *tagset, struct client *c)
{
    BitSet *bitset = bitset_create(server.workspaces->len);
    workspace_id_to_tag(bitset, c->ws_id);
    bitset_and(bitset, tagset->workspaces);
    bool contains = bitset_any(bitset);
    bitset_destroy(bitset);
    return contains;
}

bool visible_on(struct tagset *tagset, struct container *con)
{
    if (!con)
        return false;
    if (con->hidden)
        return false;

    return exist_on(tagset, con);
}

bool tagset_is_visible(struct tagset *tagset)
{
    if (!tagset)
        return false;
    assert(tagset->m != NULL);

    return tagset->m->tagset == tagset;
}

bool exist_on(struct tagset *tagset, struct container *con)
{
    if (!con || !tagset)
        return false;
    struct monitor *m = container_get_monitor(con);
    if (m != tagset->m) {
        if (con->floating)
            return container_intersects_with_monitor(con, tagset->m)
                && tagset_contains_client(m->tagset, con->client);
        else
            return false;
    }

    struct client *c = con->client;

    if (!c)
        return false;

    if (c->type == LAYER_SHELL)
        return true;
    if (c->sticky)
        return true;

    return tagset_contains_client(tagset, c);
}
