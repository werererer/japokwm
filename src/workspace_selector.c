#include "workspace_selector.h"
#include "server.h"
#include "bitset/bitset.h"
#include "tile/tileUtils.h"

void init_workspace_selector(struct workspace_selector *ws_selector)
{
    ws_selector->ws_id = INVALID_WORKSPACE_ID;
    bitset_setup(&ws_selector->ids, server.workspaces.length);
    /* bitset_reserve */
}

static void clear_container_lists(struct container_lists *dest)
{
    if (!dest)
        return;

    wlr_list_clear(&dest->focus_stack_layer_background, NULL);
    wlr_list_clear(&dest->focus_stack_layer_bottom, NULL);
    wlr_list_clear(&dest->focus_stack_layer_top, NULL);
    wlr_list_clear(&dest->focus_stack_layer_overlay, NULL);
    wlr_list_clear(&dest->focus_stack_on_top, NULL);
    wlr_list_clear(&dest->focus_stack_normal, NULL);
    wlr_list_clear(&dest->focus_stack_hidden, NULL);
    wlr_list_clear(&dest->focus_stack_not_focusable, NULL);

    wlr_list_clear(&dest->tiled_containers, NULL);
    wlr_list_clear(&dest->floating_containers, NULL);
    wlr_list_clear(&dest->hidden_containers, NULL);
}

#define remove_list(dest, src, list) wlr_list_remove_all(&dest->list, cmp_ptr, &src->list)
static void remove_container_lists(struct container_lists *dest, struct container_lists *src)
{
    if (!src)
        return;
    if (!dest)
        return;

    remove_list(dest, src, focus_stack_layer_background);
    remove_list(dest, src, focus_stack_layer_bottom);
    remove_list(dest, src, focus_stack_layer_top);
    remove_list(dest, src, focus_stack_layer_overlay);
    remove_list(dest, src, focus_stack_on_top);
    remove_list(dest, src, focus_stack_normal);
    remove_list(dest, src, focus_stack_hidden);
    remove_list(dest, src, focus_stack_not_focusable);
    remove_list(dest, src, tiled_containers);
    remove_list(dest, src, floating_containers);
    remove_list(dest, src, hidden_containers);
}

static void write_certain_items(struct wlr_list *dest, struct wlr_list *src,
        struct workspace *ws)
{
    if (!ws) {
        wlr_list_cat(dest, src);
        return;
    }

    for (int i = 0; i < src->length; i++) {
        struct container *con = src->items[i];
        if (con->client->ws_selector.ws_id == ws->id) {
            wlr_list_push(dest, con);
        }
    }
}

#define append_list(dest, src, list, ws) write_certain_items(&dest->list, &src->list, ws)
static void write_container_lists(struct container_lists *dest, struct container_lists *src, struct workspace *ws)
{
    if (!src)
        return;
    if (!dest)
        return;

    append_list(dest, src, focus_stack_layer_background, ws);
    append_list(dest, src, focus_stack_layer_bottom, ws);
    append_list(dest, src, focus_stack_layer_top, ws);
    append_list(dest, src, focus_stack_layer_overlay, ws);
    append_list(dest, src, focus_stack_on_top, ws);
    append_list(dest, src, focus_stack_normal, ws);
    append_list(dest, src, focus_stack_hidden, ws);
    append_list(dest, src, focus_stack_not_focusable, ws);
    append_list(dest, src, tiled_containers, ws);
    append_list(dest, src, floating_containers, ws);
    append_list(dest, src, hidden_containers, ws);
}

static void write_monitor_to_workspace(struct workspace *ws, struct monitor *m)
{
    clear_container_lists(&ws->lists);
    write_container_lists(&ws->lists, &m->view.lists, ws);
}

static void write_monitor_to_workspaces(struct monitor *m)
{
    for (int i = 0; i < length_of_composed_list(&m->view.lists.container_lists); i++) {
        struct container *con = get_container(m, i);
        int ws_id = con->client->ws_selector.ws_id;
        struct workspace *ws = get_workspace(ws_id);
        write_monitor_to_workspace(ws, m);
    }
}

static void remove_workspace_from_monitor(struct monitor *m, struct workspace *ws)
{
    remove_container_lists(&m->view.lists, &ws->lists);
}

static void write_workspace_to_monitor(struct monitor *m, struct workspace *ws)
{
    write_container_lists(&m->view.lists, &ws->lists, NULL);
}

static void write_workspaces_to_monitor(struct monitor *m)
{
    clear_container_lists(&m->view.lists);
    struct wlr_list workspaces = get_workspaces(&m->view.ws_selector.ids);
    for (int i = 0; i < workspaces.length; i++) {
        struct workspace *ws = workspaces.items[i];
        write_workspace_to_monitor(m, ws);
    }
}

void workspace_selector_from_workspace_id(struct workspace_selector *ws_selector,
        int ws_id)
{
    struct workspace *ws = get_workspace(ws_id);

    if (!ws)
        return;

    ws_selector->ws_id = ws->id;
    bitset_reset_all(&ws_selector->ids);
    bitset_set(&ws_selector->ids, ws->id);
}

void select_workspace(struct workspace_selector *ws_selector, int ws_id)
{
    ws_selector->ws_id = ws_id;
}

void tag_workspace(struct workspace_selector *ws_selector, BitSet *bitset)
{
    struct monitor *m = selected_monitor;

    write_monitor_to_workspaces(m);

    bitset_xor(&ws_selector->ids, bitset);

    write_workspaces_to_monitor(m);

    arrange();
}

struct wlr_list get_workspaces(BitSet *bitset)
{
    struct wlr_list res_list;
    wlr_list_init(&res_list);

    for (int i = 0; i < server.workspaces.length; i++) {
        BitSet result;
        bitset_setup(&result, server.workspaces.length);
        bitset_set(&result, i);
        bitset_and(&result, bitset);
        if (bitset_any(&result)) {
            struct workspace *ws = get_workspace(i);
            wlr_list_push(&res_list, ws);
        }
    }

    return res_list;
}

static struct wlr_list combine_list(struct wlr_list *workspaces,
        struct wlr_list *(get_list_in_workspace_func)(struct workspace *))
{
    struct wlr_list res_lists;
    wlr_list_init(&res_lists);
    for (int i = 0; i < workspaces->length; i++) {
        struct workspace *ws = workspaces->items[i];
        struct wlr_list *lists = get_list_in_workspace_func(ws);
        wlr_list_cat(&res_lists, lists);
    }
    return res_lists;
}

static struct wlr_list combine_lists(struct wlr_list *workspaces, 
        struct wlr_list *(get_lists_in_workspace_func)(struct workspace *))
{
    struct wlr_list res_lists;
    wlr_list_init(&res_lists);

    struct monitor *m = selected_monitor;

    struct wlr_list *ref_list = get_lists_in_workspace_func(get_workspace(m->view.ws_selector.ws_id));

    for (int i = 0; i < ref_list->length; i++) {
        for (int j = 0; j < workspaces->length; j++) {
            struct workspace *ws = workspaces->items[j];
            struct wlr_list *lists = get_lists_in_workspace_func(ws);

            wlr_list_push(&res_lists, lists->items[i]);
        }
    }

    return res_lists;
}

struct wlr_list get_combined_focus_stack_lists(struct workspace_selector *ws_selector)
{
    struct wlr_list workspaces = get_workspaces(&ws_selector->ids);
    struct wlr_list res_lists = combine_lists(&workspaces, get_focus_stack_lists);
    return res_lists;
}

struct wlr_list get_combined_visible_lists(struct workspace_selector *ws_selector)
{
    struct wlr_list workspaces = get_workspaces(&ws_selector->ids);
    struct wlr_list res_lists = combine_lists(&workspaces, get_visible_lists);
    return res_lists;
}

struct wlr_list get_combined_tiled_containers(struct workspace_selector *ws_selector)
{
    struct wlr_list workspaces = get_workspaces(&ws_selector->ids);
    struct wlr_list res_list = combine_list(&workspaces, get_tiled_list);
    return res_list;
}

struct wlr_list get_combined_hidden_containers(struct workspace_selector *ws_selector)
{
    struct wlr_list workspaces = get_workspaces(&ws_selector->ids);
    struct wlr_list res_list = combine_list(&workspaces, get_hidden_list);
    return res_list;
}
