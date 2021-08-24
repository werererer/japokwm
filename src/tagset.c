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

static void tagset_assign_workspace(struct tagset *tagset, struct workspace *ws, bool load);
static void tagset_load_workspaces(struct tagset *tagset, BitSet *workspaces);
static void tagset_load_workspace(struct tagset *tagset, struct workspace *ws);
static void tagset_unload_workspaces(struct tagset *tagset);
static void tagset_unload_workspace(struct tagset *tagset, struct workspace *ws);
static void tagset_clear_workspaces(struct tagset *tagset);
static void move_old_workspaces_back(BitSet *old_workspaces, BitSet *workspaces);
static void force_on_current_monitor(struct tagset *tagset, BitSet *bitset);
static void unfocus_action(struct tagset *old_tagset, struct monitor *new_monitor, BitSet *new_bits);

static void tagset_subscribe_to_workspace(struct tagset *tagset, struct workspace *ws);

static void tagset_subscribe_to_workspace(struct tagset *tagset, struct workspace *ws)
{
    g_ptr_array_add(ws->subscribed_tagsets, tagset);
    append_list_set(tagset->list_set, ws->list_set);
}

static void tagset_assign_workspace(struct tagset *tagset, struct workspace *ws, bool load)
{
    if (load) {
        tagset_load_workspace(tagset, ws);
    } else {
        tagset_load_workspace(tagset, ws);
    }
}

static void tagset_load_workspaces(struct tagset *tagset, BitSet *workspaces)
{
    assert(tagset != NULL);
    assert(tagset->loaded == false);

    bitset_move(tagset->workspaces, workspaces);

    for (size_t i = 0; i < tagset->workspaces->size; i++) {
        bool bit = bitset_test(tagset->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        tagset_subscribe_to_workspace(tagset, ws);
        ws->tagset = tagset;
    }
    tagset->loaded = true;
}

static void tagset_load_workspace(struct tagset *tagset, struct workspace *ws)
{
    assert(tagset != NULL);
    assert(ws != NULL);

    bool bit = bitset_test(tagset->workspaces, ws->id);

    // if is already loaded in tagset
    if (bit)
        return;

    bitset_set(tagset->workspaces, ws->id);
    tagset_subscribe_to_workspace(tagset, ws);

    append_list_set(tagset->list_set, ws->list_set);
    list_set_remove_list_set(tagset->list_set, ws->list_set);
    tagset->loaded = true;
}

static void tagset_unsubscribe_from_workspace(struct tagset *tagset, struct workspace *ws)
{
    g_ptr_array_remove(ws->subscribed_tagsets, tagset);
}

static void tagset_unload_workspace(struct tagset *tagset, struct workspace *ws)
{
    assert(tagset != NULL);
    assert(ws != NULL);

    bool bit = bitset_test(tagset->workspaces, ws->id);

    if (!bit)
        return;

    bitset_reset(tagset->workspaces, ws->id);

    list_set_remove_list_set(tagset->list_set, ws->list_set);

    tagset_unsubscribe_from_workspace(tagset, ws);

    if (!bitset_any(tagset->workspaces))
        tagset->loaded = false;
}

static void tagset_unload_workspaces(struct tagset *tagset)
{
    assert(tagset != NULL);
    assert(tagset->loaded == true);

    clear_list_set(tagset->list_set);

    for (int i = 0; i < tagset->workspaces->size; i++) {
        bool bit = bitset_test(tagset->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        tagset_unsubscribe_from_workspace(tagset, ws);
        if (ws->tagset == tagset) {
            ws->tagset = NULL;
        }
    }
    tagset->loaded = false;
}

static void move_old_workspaces_back(BitSet *old_workspaces, BitSet *workspaces)
{
    BitSet *diff = bitset_copy(old_workspaces);
    bitset_xor(diff, workspaces);
    for (int i = 0; i < diff->size; i++) {
        bool is_changed = bitset_test(diff, i);
        bool new_bit = bitset_test(workspaces, i);

        if (!is_changed)
            continue;
        if (new_bit)
            continue;

        struct workspace *ws = get_workspace(i);

        if (!ws->tagset)
            continue;

        if (is_workspace_occupied(ws) && ws->m->tagset != ws->tagset) {
            ws->tagset = ws->m->tagset;
            tagset_load_workspace(ws->tagset, ws);
        }
    }
}

static void force_on_current_monitor(struct tagset *tagset, BitSet *bitset)
{
    BitSet *changed_bits = bitset_copy(tagset->workspaces);
    bitset_xor(changed_bits, bitset);
    // force new tags to be on current monitor
    for (int i = 0; i < bitset->size; i++) {
        bool bit_changed = bitset_test(changed_bits, i);

        if (!bit_changed)
            continue;

        struct workspace *ws = get_workspace(i);

        if (!ws->m)
            continue;

        struct tagset *old_tagset = ws->tagset;

        if (!ws->tagset)
            continue;

        if (is_workspace_occupied(ws) && tagset != old_tagset) {
            bool new_bit = !bitset_test(bitset, i);
            struct workspace *ws = get_workspace(i);
            tagset_assign_workspace(old_tagset, ws, new_bit);
        }
    }
}

static void reset_old_workspaces(struct tagset *old_tagset, struct monitor *new_monitor)
{
    if (!old_tagset)
        return;

    for (int i = 0; i < old_tagset->workspaces->size; i++) {
        bool bit = bitset_test(old_tagset->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        bool is_empty = is_workspace_empty(ws);
        if (is_empty && ws->m == selected_monitor && ws->m == new_monitor) {
            ws->m = NULL;
        }
    }
}

static void unfocus_action(struct tagset *old_tagset, struct monitor *new_monitor, BitSet *new_bits)
{
    if (!old_tagset)
        return;

    struct seat *seat = input_manager_get_default_seat();
    cursor_constrain(seat->cursor, NULL);

    move_old_workspaces_back(old_tagset->workspaces, new_bits);
    reset_old_workspaces(old_tagset, new_monitor);
}

static void focus_action(struct tagset *tagset)
{
    /* NOOP */
}

struct tagset *create_tagset(struct monitor *m, int selected_ws_id, BitSet *workspaces)
{
    struct tagset *tagset = calloc(1, sizeof(struct tagset));
    tagset->m = m;

    tagset->selected_ws_id = selected_ws_id;
    struct workspace *selected_workspace = get_workspace(tagset->selected_ws_id);
    selected_workspace->tagset = tagset;

    tagset->list_set = create_list_set();

    tagset->workspaces = bitset_create(server.workspaces->len);
    tagset_load_workspaces(tagset, workspaces);

    g_ptr_array_add(server.tagsets, tagset);
    return tagset;
}

void destroy_tagset(struct tagset *tagset)
{
    if (!tagset)
        return;
    tagset_unload_workspaces(tagset);
    g_ptr_array_remove(server.tagsets, tagset);
    bitset_destroy(tagset->workspaces);
    destroy_list_set(tagset->list_set);
    free(tagset);
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

void focus_tagset(struct tagset *tagset)
{
    if(!tagset)
        return;

    struct monitor *m = tagset->m;
    focus_monitor(m);

    struct tagset *old_tagset = m->tagset;

    if (old_tagset) {
        for (int i = 0; i < length_of_composed_list(old_tagset->list_set->container_lists); i++) {
            struct container *con = get_in_composed_list(old_tagset->list_set->container_lists, i);
            struct workspace *ws = get_workspace(tagset->selected_ws_id);
            if (con->client->sticky) {
                move_container_to_workspace(con, ws);
            }
        }

        unfocus_action(old_tagset, tagset->m, tagset->workspaces);
    }

    m->tagset = tagset;
    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    if (!ws->m) {
        ws->m = tagset->m;
        printf("tagset.c: tagset->m: %p\n", tagset->m);
    }

    ipc_event_workspace();

    arrange();
    focus_action(tagset);
    focus_most_recent_container(m->tagset);
    root_damage_whole(m->root);

    struct seat *seat = input_manager_get_default_seat();
    cursor_rebase(seat->cursor);
    focus_under_cursor(seat->cursor, 0);
    cursor_update_image(seat->cursor);
}

static void tagset_save_to_workspace(struct tagset *tagset, struct workspace *ws)
{
    for (int i = 0; i < ws->list_set->all_lists->len; i++) {
        GPtrArray *dest_list = g_ptr_array_index(ws->list_set->all_lists, i);
        GPtrArray *src_list = g_ptr_array_index(tagset->list_set->all_lists, i);
        for (int j = 0; j < src_list->len; j++) {
            struct container *con = g_ptr_array_index(src_list, j);
            if (con->client->ws_id != ws->id)
                continue;
            g_ptr_array_add(dest_list, con);
        }
    }
}

static void tagset_clear_workspaces(struct tagset *tagset)
{
    for (int i = 0; i < tagset->workspaces->size; i++) {
        bool bit = bitset_test(tagset->workspaces, i);

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

static void tagset_write_to_workspaces(struct tagset *tagset)
{
    if (!tagset)
        return;
    if (!tagset->loaded) {
        return;
    }

    tagset_clear_workspaces(tagset);
    tagset_append_to_workspaces(tagset);
}

struct layout *tagset_get_layout(struct tagset *tagset)
{
    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    return ws->layout;
}

void tagset_set_tags(struct tagset *tagset, BitSet *bitset)
{
    force_on_current_monitor(tagset, bitset);
    unfocus_action(tagset, tagset->m, bitset);

    tagset_write_to_workspaces(tagset);
    tagset_unload_workspaces(tagset);
    tagset_load_workspaces(tagset, bitset);

    ipc_event_workspace();
}

static void tagset_push_queue(struct tagset *prev_tagset, struct tagset *tagset)
{
    if (prev_tagset->selected_ws_id == tagset->selected_ws_id)
        return;

    if (server.previous_tagset != tagset) {
        destroy_tagset(server.previous_tagset);
    }

    server.previous_tagset = prev_tagset;
}

void push_tagset(struct tagset *tagset)
{
    if (!tagset)
        return;

    struct monitor *m = tagset->m;

    tagset_write_to_workspaces(m->tagset);

    tagset_push_queue(m->tagset, tagset);

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
    struct monitor *m = ws->m ? ws->m : selected_monitor;

    struct tagset *tagset = get_tagset_from_active_workspace_id(ws_id);
    if (tagset) {
        tagset_set_tags(tagset, bitset);
    } else {
        tagset = create_tagset(m, ws_id, bitset);
    }

    push_tagset(tagset);
}

struct tagset *get_tagset_from_active_workspace_id(int ws_id)
{
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct tagset *tagset = m->tagset;

        if (!tagset)
            continue;

        if (tagset->selected_ws_id == ws_id) {
            return tagset;
        }
    }
    return NULL;
}

struct tagset *get_tagset_from_workspace_id(int ws_id)
{
    // TODO fix memory leak in this function
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct tagset *tagset = m->tagset;

        if (!tagset)
            continue;

        BitSet *bitset = bitset_create(server.workspaces->len);
        bitset_set(bitset, ws_id);
        bitset_and(bitset, tagset->workspaces);
        if (bitset_any(bitset)) {
            bitset_destroy(bitset);
            return tagset;
        }
        bitset_destroy(bitset);
    }

    return NULL;
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
