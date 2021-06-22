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

struct tagset *create_tagset(struct monitor *m, int selected_ws_id, BitSet workspaces)
{
    struct tagset *tagset = calloc(1, sizeof(struct tagset));
    tagset->m = m;
    tagset->selected_ws_id = selected_ws_id;

    setup_list_set(&tagset->list_set);

    bitset_setup(&tagset->workspaces, server.workspaces.length);
    tagset_set_tags(tagset, workspaces);

    wlr_list_push(&server.tagsets, tagset);
    return tagset;
}

void destroy_tagset(struct tagset *tagset)
{
    if (!tagset)
        return;
    wlr_list_remove(&server.tagsets, cmp_ptr, tagset);
    free(tagset);
}

void focus_most_recent_container(struct tagset *tagset, enum focus_actions a)
{
    struct container *con = get_in_composed_list(&tagset->list_set.focus_stack_lists, 0);

    if (!con) {
        con = get_container(tagset, 0);
        if (!con) {
            ipc_event_window();
            return;
        }
    }

    focus_container(con, a);
}

void focus_tagset(struct tagset *tagset)
{
    if(!tagset)
        return;

    struct monitor *m = selected_monitor;
    m->tagset = tagset;

    ipc_event_workspace();

    // TODO add support for sticky containers again
    /* struct tagset *old_tagset = monitor_get_active_tagset(m); */
    /* struct container *con; */
    /* wl_list_for_each(con, &sticky_stack, stlink) { */
    /*     con->client->ws_id = ws->id; */
    /* } */

    arrange();
    focus_most_recent_container(m->tagset, FOCUS_NOOP);
    root_damage_whole(m->root);
}

static void tagset_save_to_workspace(struct tagset *tagset)
{
    if (!tagset)
        return;
    if (!tagset->loaded) {
        printf("cant save not loaded: %i\n", tagset->selected_ws_id);
        return;
    }

    for (int i = 0; i < tagset->workspaces.size; i++) {
        bool bit = bitset_test(&tagset->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        clear_list_set(&ws->list_set);

        // TODO convert to a function
        for (int i = 0; i < ws->list_set.all_lists.length; i++) {
            struct wlr_list *dest_list = ws->list_set.all_lists.items[i];
            struct wlr_list *src_list = tagset->list_set.all_lists.items[i];
            for (int j = 0; j < src_list->length; j++) {
                struct container *con = src_list->items[j];
                if (con->client->ws_id != ws->id)
                    continue;
                wlr_list_push(dest_list, con);
            }
        }

        wlr_list_remove(&ws->list_set.change_affected_list_sets, cmp_ptr, &tagset->list_set);
    }
}

void tagset_unset_tagset(struct tagset *tagset)
{
    if (!tagset)
        return;
    if (!tagset->loaded) {
        printf("cant save not loaded: %i\n", tagset->selected_ws_id);
        return;
    }

    clear_list_set(&tagset->list_set);
    tagset->loaded = false;
    printf("unloaded: %i\n", tagset->selected_ws_id);
}

struct layout *tagset_get_layout(struct tagset *tagset)
{
    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    return ws->layout;
}

void tagset_load_from_workspace(struct tagset *tagset)
{
    if (!tagset)
        return;
    if (tagset->loaded) {
        printf("cant load: %i\n", tagset->selected_ws_id);
        return;
    }

    for(size_t i = 0; i < tagset->workspaces.size; i++) {
        bool bit = bitset_test(&tagset->workspaces, i);

        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        subscribe_list_set(&tagset->list_set, &ws->list_set);
        ws->m = tagset->m;
    }
    printf("load tagset: %i\n", tagset->selected_ws_id);
    tagset->loaded = true;
}

void tagset_set_tags(struct tagset *tagset, BitSet bitset)
{
    struct monitor *m = selected_monitor;
    tagset_save_to_workspace(m->tagset);
    bitset_move(&tagset->workspaces, &bitset);
    tagset_load_from_workspace(tagset);

    ipc_event_workspace();
}

void push_tagset(struct tagset *tagset)
{
    assert(tagset != NULL);

    struct monitor *m = selected_monitor;

    if (m->tagset == tagset) {
        printf("tagset already active\n");
        return;
    }

    /* printf("previous_tagset: %p current tagset: %p\n", server.previous_tagset, tagset); */
    tagset_save_to_workspace(m->tagset);
    if (server.previous_tagset) {
        if (server.previous_tagset != tagset) {
            /* printf("unset previous tagset\n"); */
            tagset_unset_tagset(server.previous_tagset);
        }
    } else {
        tagset_save_to_workspace(m->tagset);
    }

    if (m->tagset != server.previous_tagset)
        server.previous_tagset = m->tagset;
    focus_tagset(tagset);
}

void tagset_set_workspace_id(int ws_id)
{
    BitSet bitset;
    bitset_setup(&bitset, server.workspaces.length);
    bitset_set(&bitset, ws_id);
    monitor_focus_tags(selected_monitor, ws_id, bitset);
}

void tagset_toggle_add(struct tagset *tagset, BitSet bitset)
{
    if (!tagset)
        return;

    bitset_xor(&bitset, &tagset->workspaces);

    monitor_focus_tags(selected_monitor, tagset->selected_ws_id, bitset);
}

void tagset_toggle_add_workspace_id(struct tagset *tagset, int ws_id)
{
    // TODO implement
}

struct tagset *get_tagset_from_workspace_id(struct wlr_list *workspaces, int ws_id)
{
    for (int i = 0; i < server.tagsets.length; i++) {
        struct tagset *tagset = server.tagsets.items[i];
        BitSet bitset;
        bitset_setup(&bitset, server.workspaces.length);
        bitset_set(&bitset, ws_id);
        bitset_and(&bitset, &tagset->workspaces);
        if (bitset_any(&bitset)) {
            return tagset;
        }
    }
    return NULL;
}

struct container *get_container(struct tagset *ts, int i)
{
    struct list_set *list_set = &ts->list_set;
    if (!list_set)
        return NULL;

    return get_in_composed_list(&list_set->container_lists, i);
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

struct wlr_list *tagset_get_visible_lists(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus)
        return &tagset->list_set.focus_stack_visible_lists;
    else
        return &tagset->list_set.visible_container_lists;
}

struct wlr_list *tagset_get_tiled_list(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus)
        return &tagset->list_set.focus_stack_normal;
    else
        return &tagset->list_set.tiled_containers;
}

struct wlr_list *tagset_get_floating_list(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus)
        return &tagset->list_set.focus_stack_normal;
    else
        return &tagset->list_set.floating_containers;
}

struct wlr_list *tagset_get_hidden_list(struct tagset *tagset)
{
    struct layout *lt = tagset_get_layout(tagset);

    if (lt->options.arrange_by_focus)
        return &tagset->list_set.focus_stack_hidden;
    else
        return &tagset->list_set.hidden_containers;
}

BitSet workspace_id_to_tag(int ws_id)
{
    BitSet bitset;
    bitset_setup(&bitset, server.workspaces.length);
    bitset_set(&bitset, ws_id);
    return bitset;
}

bool tagset_contains_client(struct tagset *ts, struct client *c)
{
    BitSet bitset = workspace_id_to_tag(c->ws_id);
    bitset_and(&bitset, &ts->workspaces);
    return bitset_any(&bitset);
}

bool tagset_has_clients(struct tagset *tagset)
{
    if (!tagset)
        return false;

    for (int i = 0; i < server.normal_clients.length; i++) {
        struct client *c = server.normal_clients.items[i];

        if (tagset_contains_client(tagset, c))
            return true;
    }

    return false;
}

bool workspace_has_clients(struct tagset *tagset)
{
    if (!tagset)
        return false;

    for (int i = 0; i < server.normal_clients.length; i++) {
        struct client *c = server.normal_clients.items[i];

        if (tagset_contains_client(tagset, c))
            return true;
    }

    return false;
}

bool hidden_on(struct container *con, struct tagset *ts)
{
    return !visible_on(con, ts) && exist_on(con, ts);
}

bool visible_on(struct container *con, struct tagset *ts)
{
    if (!con)
        return false;
    if (con->hidden)
        return false;

    return exist_on(con, ts);
}

bool exist_on(struct container *con, struct tagset *ts)
{
    if (!con || !ts)
        return false;
    if (con->m != ts->m) {
        if (con->floating)
            return container_intersects_with_monitor(con, ts->m)
                && tagset_contains_client(con->m->tagset, con->client);
        else
            return false;
    }

    struct client *c = con->client;

    if (!c)
        return false;

    if (c->type == LAYER_SHELL) {
        return true;
    }
    if (c->sticky) {
        return true;
    }

    return tagset_contains_client(ts, c);
}

int tagset_get_container_count(struct tagset *ts)
{
    if (!ts)
        return -1;

    int i = 0;
    for (int i = 0; i < ts->list_set.tiled_containers.length; i++) {
        struct container *con = get_container(ts, i);

        if (visible_on(con, ts))
            i++;
    }
    return i;
}
