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

void select_workspace(struct workspace_selector *ws_selector, int ws_id)
{
    ws_selector->ws_id = ws_id;
    bitset_reset_all(&ws_selector->ids);
    bitset_set(&ws_selector->ids, ws_id);
    struct monitor *m = selected_monitor;
    m->visible_lists = get_combined_visible_lists(&m->ws_selector);
    m->tiled_containers = get_combined_tiled_containers(&m->ws_selector);
    m->hidden_containers = get_combined_hidden_containers(&m->ws_selector);
    m->hidden_containers = get_combined_hidden_containers(&m->ws_selector);
}

void tag_workspace(struct workspace_selector *ws_selector, BitSet *bitset)
{
    bitset_xor(&ws_selector->ids, bitset);
    struct monitor *m = selected_monitor;
    m->visible_lists = get_combined_visible_lists(&m->ws_selector);
    m->tiled_containers = get_combined_tiled_containers(&m->ws_selector);
    m->hidden_containers = get_combined_hidden_containers(&m->ws_selector);
    m->focus_stack_lists = get_combined_focus_stack_lists(&m->ws_selector);
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
            struct workspace *ws = server.workspaces.items[i];
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
    struct wlr_list *ref_list =
        get_lists_in_workspace_func(get_workspace(selected_monitor->ws_selector.ws_id));

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
