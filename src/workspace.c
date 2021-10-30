#include "workspace.h"

#include <assert.h>
#include <string.h>
#include <wayland-server.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_cursor.h>

#include "client.h"
#include "ipc-server.h"
#include "list_sets/container_stack_set.h"
#include "monitor.h"
#include "server.h"
#include "utils/parseConfigUtils.h"
#include "tile/tileUtils.h"
#include "utils/parseConfigUtils.h"
#include "container.h"
#include "stringop.h"
#include "list_sets/list_set.h"
#include "list_sets/focus_stack_set.h"
#include "tagset.h"
#include "root.h"
#include "translationLayer.h"

static void update_workspaces_id(GPtrArray *workspaces)
{
    for (int id = 0; id < workspaces->len; id++) {
        struct workspace *ws = g_ptr_array_index(workspaces, id);
        ws->id = id;
    }
}

static void _destroy_workspace(void *ws)
{
    destroy_workspace(ws);
}

void load_workspaces(GList *workspaces, GPtrArray *tag_names)
{
    for (int i = 0; i < tag_names->len; i++) {
        struct workspace *ws = get_workspace(i);
        const char *name = g_ptr_array_index(tag_names, i);
        ws->current_layout = server.default_layout->name;
        ws->previous_layout = server.default_layout->name;
        workspace_remove_loaded_layouts(ws);
        workspace_rename(ws, name);
    }
}

void destroy_workspaces(GHashTable *workspaces)
{
    g_hash_table_unref(workspaces);
}

GHashTable *create_workspaces()
{
    // GHashTable *workspaces = g_ptr_array_new_with_free_func(_destroy_workspace);
    GHashTable *workspaces = g_hash_table_new(g_int_hash, g_int_equal);
    return workspaces;
}

struct workspace *create_workspace(const char *name, size_t id, struct layout *lt)
{
    struct workspace *ws = calloc(1, sizeof(struct workspace));
    ws->name = strdup(name);
    ws->id = id;

    ws->loaded_layouts = g_ptr_array_new();
    ws->current_layout = lt->name;
    ws->previous_layout = lt->name;

    // TODO: findout which value must be used
    ws->workspaces = bitset_create();
    ws->prev_workspaces = bitset_create();
    ws->workspaces->data = ws;

    bitset_set(ws->workspaces, ws->id);
    bitset_set(ws->prev_workspaces, ws->id);

    ws->con_set = create_container_set();
    ws->visible_con_set = create_container_set();

    ws->focus_set = focus_set_create();
    ws->visible_focus_set = focus_set_create();

    ws->visible_bar_edges = WLR_EDGE_BOTTOM
        | WLR_EDGE_TOP
        | WLR_EDGE_LEFT
        | WLR_EDGE_RIGHT;
    return ws;
}

void update_workspaces(GList *workspaces, GPtrArray *tag_names)
{
    for (GList *iterator = workspaces; iterator; iterator = iterator->next) {
        struct workspace *ws = iterator->data;
        if (ws->id > tag_names->len)
            continue;
        const char *new_name = g_ptr_array_index(tag_names, ws->id);
        workspace_rename(ws, new_name);
    }
}

void focus_most_recent_container()
{
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);

    struct container *con = monitor_get_focused_container(m);
    // TODO: this can be exported to an external function
    if (!con) {
        if (ws->visible_con_set->tiled_containers->len <= 0)
            return;
        struct container *con = g_ptr_array_index(ws->visible_con_set->tiled_containers, 0);
        if (!con)
            return;
    }

    focus_container(con);
}

struct container *get_container(struct workspace *ws, int i)
{
    return get_in_composed_list(ws->visible_focus_set->focus_stack_visible_lists, i);
}

struct container *get_container_in_stack(struct workspace *ws, int i)
{
    GPtrArray *tiled_containers = workspace_get_tiled_list_copy(ws);
    struct container *con = g_ptr_array_index(tiled_containers, i);
    return con;
}

void destroy_workspace(struct workspace *ws)
{
    for (int i = 0; i < ws->loaded_layouts->len; i++) {
        struct layout *lt = g_ptr_array_steal_index(ws->loaded_layouts, 0);
        destroy_layout(lt);
    }

    workspace_remove_loaded_layouts(ws);
    g_ptr_array_unref(ws->loaded_layouts);

    bitset_destroy(ws->workspaces);
    bitset_destroy(ws->prev_workspaces);
    focus_set_destroy(ws->focus_set);
    focus_set_destroy(ws->visible_focus_set);

    for (int i = 0; i < ws->con_set->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(ws->con_set->tiled_containers, i);
        if (con->ws_id != ws->id)
            continue;

        struct client *c = con->client;
        kill_client(c);
    }
    destroy_container_set(ws->con_set);
    destroy_container_set(ws->visible_con_set);
    free(ws->name);
    free(ws);
}

void update_workspace_ids(GPtrArray *workspaces)
{
    for (int i = 0; i < workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(workspaces, i);
        ws->id = i;
    }
}

bool is_workspace_occupied(struct workspace *ws)
{
    assert(ws);

    struct monitor *m = workspace_get_monitor(ws);
    bool is_occupied = (m != NULL) && workspace_is_visible(ws, m);
    return is_occupied;
}

bool workspace_is_visible(struct workspace *ws, struct monitor *m)
{
    assert(ws != NULL);

    struct workspace *sel_ws = monitor_get_active_workspace(m);
    if (ws == sel_ws) {
        return true;
    }
    if (bitset_test(sel_ws->workspaces, ws->id)) {
        return true;
    }
    if (ws->current_m && !is_workspace_empty(ws)) {
        return true;
    }
    return false;
}

bool is_workspace_extern(struct workspace *ws)
{
    struct monitor *ws_m = workspace_get_selected_monitor(ws);
    if (!ws_m) {
        return false;
    }

    struct monitor *sel_m = server_get_selected_monitor();
    struct workspace *sel_ws = monitor_get_active_workspace(sel_m);
    if (bitset_test(sel_ws->workspaces, ws->id) && sel_m != ws_m) {
        return true;
    }
    return false;
}

bool is_workspace_the_selected_one(struct workspace *ws)
{
    struct monitor *m = workspace_get_monitor(ws);
    struct workspace *sel_ws = monitor_get_active_workspace(m);
    return sel_ws->id == ws->id && !is_workspace_extern(ws);
}

bool workspace_is_active(struct workspace *ws)
{
    struct monitor *m = workspace_get_monitor(ws);
    if (!m)
        return false;
    struct workspace *sel_ws = monitor_get_active_workspace(m);

    return bitset_test(sel_ws->workspaces, ws->id);
}

int get_workspace_container_count(struct workspace *ws)
{
    if (!ws)
        return -1;

    int count = 0;
    for (int i = 0; i < ws->con_set->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(ws->con_set->tiled_containers, i);
        if (con->ws_id == ws->id) {
            count++;
        }
    }
    return count;
}

bool is_workspace_empty(struct workspace *ws)
{
    return get_workspace_container_count(ws) == 0;
}

struct workspace *find_next_unoccupied_workspace(GList *workspaces, struct workspace *ws)
{
    // we have infinity amount of workspaces so just get the next one
    // (infinity - 1 = infinity ;) )
    size_t start_id = ws ? ws->id : 0;
    for (size_t i = start_id ;; i++) {
        struct workspace *ws = get_workspace(i);
        if (!is_workspace_occupied(ws))
            return ws;
    }
}

struct workspace *get_next_empty_workspace(GList *workspaces, size_t ws_id)
{
    for (GList *iter = g_list_nth(workspaces, ws_id); iter; iter = iter->next) {
        struct workspace *ws = iter->data;
        if (is_workspace_empty(ws)) {
            return ws;
        }
    }

    for (GList *iter = g_list_nth(workspaces, ws_id); iter; iter = iter->prev) {
        struct workspace *ws = iter->data;
        if (is_workspace_empty(ws)) {
            return ws;
        }
    }

    return NULL;
}

struct workspace *get_nearest_empty_workspace(GList *workspaces, int ws_id)
{
    struct workspace *initial_workspace = get_workspace(ws_id);
    if (is_workspace_empty(initial_workspace)) {
        return initial_workspace;
    }

    int workspace_count = server_get_workspace_count();
    struct workspace *ws = NULL;
    for (int i = 0, up_counter = ws_id+1, down_counter = ws_id-1;
            i < workspace_count;
            i++,up_counter++,down_counter--) {

        bool is_up_counter_valid = up_counter < workspace_count;
        bool is_down_counter_valid = down_counter >= 0;
        if (is_down_counter_valid) {
            ws = g_list_nth(workspaces, down_counter)->data;
            if (is_workspace_empty(ws))
                break;
        }
        if (is_up_counter_valid) {
            ws = g_list_nth(workspaces, up_counter)->data;
            if (is_workspace_empty(ws))
                break;
        }
        if (!is_up_counter_valid && !is_down_counter_valid) {
            break;
        }
    }

    return ws;
}


struct workspace *get_prev_empty_workspace(GList *workspaces, size_t ws_id)
{
    for (GList *iter = g_list_nth(workspaces, ws_id); iter; iter = iter->next) {
        struct workspace *ws = iter->data;
        if (is_workspace_empty(ws)) {
            return ws;
        }
    }

    for (GList *iter = g_list_nth(workspaces, ws_id+1); iter; iter = iter->prev) {
        struct workspace *ws = iter->data;
        if (is_workspace_empty(ws)) {
            return ws;
        }
    }

    return NULL;
}

struct layout *workspace_get_layout(struct workspace *ws)
{
    if (!ws)
        return NULL;
    if (ws->loaded_layouts->len == 0) {
        load_layout(ws);
        assert(ws->loaded_layouts->len > 0);
    }
    return g_ptr_array_index(ws->loaded_layouts, 0);
}

struct root *workspace_get_root(struct workspace *ws)
{
    struct monitor *m = workspace_get_monitor(ws);
    struct root *root = monitor_get_active_root(m);
    return root;
}

struct wlr_box workspace_get_active_geom(struct workspace *ws)
{
    struct wlr_box geom;
    struct root *root = workspace_get_root(ws);
    geom = root->geom;
    return geom;
}

struct monitor *workspace_get_selected_monitor(struct workspace *ws)
{
    assert(ws != NULL);
    return ws->m;
}

struct monitor *workspace_get_monitor(struct workspace *ws)
{
    assert(ws != NULL);

    if (ws->current_m) {
        return ws->current_m;
    }
    if (ws->m) {
        return ws->m;
    }
    return NULL;
}

void workspace_set_selected_monitor(struct workspace *ws, struct monitor *m)
{
    if (ws->m)
        return;
    ws->m = m;
}

void workspace_set_current_monitor(struct workspace *ws, struct monitor *m)
{
    ws->current_m = m;
}

BitSet *workspace_get_tags(struct workspace *ws)
{
    return ws->workspaces;
}

BitSet *workspace_get_prev_tags(struct workspace *ws)
{
    return ws->prev_workspaces;
}

void push_layout(struct workspace *ws, const char *layout_name)
{
    ws->previous_layout = ws->current_layout;
    ws->current_layout = layout_name;
}

void set_default_layout(struct workspace *ws)
{
    push_layout(ws, server.default_layout->name);
}

static void load_layout_file(lua_State *L, struct layout *lt)
{
    struct workspace *ws = get_workspace(lt->ws_id);
    init_local_config_variables(L, ws);
    const char *name = lt->name;

    char *config_path = get_config_layout_path();
    if (!config_path) {
        printf("couldn't find layout: %s loading default layout instead \n", name);
        return;
    }

    char *file = strdup("");
    join_path(&file, config_path);
    join_path(&file, name);
    join_path(&file, "init.lua");
    if (config_path)
        free(config_path);

    if (!file_exists(file))
        goto cleanup;

    if (load_file(L, file) != EXIT_SUCCESS) {
        goto cleanup;
    }

cleanup:
    free(file);
}

// returns position of the layout if it was found and -1 if not
static int workspace_load_layout(struct workspace *ws, const char *layout_name)
{
    guint i;
    bool found = g_ptr_array_find_with_equal_func(ws->loaded_layouts, layout_name, cmp_layout_to_string, &i);
    if (found) {
        return i;
    }

    struct layout *lt = create_layout(L);

    lt->ws_id = ws->id;
    copy_layout_safe(lt, server.default_layout);

    lt->name = strdup(layout_name);

    g_ptr_array_insert(ws->loaded_layouts, 0, lt);

    load_layout_file(L, lt);

    return -1;
}

void load_layout(struct workspace *ws)
{
    const char *name = ws->current_layout;
    assert(name != NULL);

    int i = workspace_load_layout(ws, name);
    push_layout(ws, name);
    if (i >= 0) {
        struct layout *lt = g_ptr_array_steal_index(ws->loaded_layouts, i);
        g_ptr_array_insert(ws->loaded_layouts, 0, lt);
        push_layout(ws, lt->name);
    } else {
        struct layout *lt = workspace_get_layout(ws);
        for (int i = 0; i < lt->linked_layouts->len; i++) {
            char *linked_layout_name = g_ptr_array_index(lt->linked_layouts, i);
            workspace_load_layout(ws, linked_layout_name);
        }
    }
}

static void destroy_layout0(void *lt)
{
    destroy_layout(lt);
}

void workspace_remove_loaded_layouts(struct workspace *ws)
{
    list_clear(ws->loaded_layouts, destroy_layout0);
}

void workspaces_remove_loaded_layouts(GList *workspaces)
{
    for (GList *iter = workspaces; iter; iter = iter->next) {
        struct workspace *ws = iter->data;
        workspace_remove_loaded_layouts(ws);
    }
}

void workspace_rename(struct workspace *ws, const char *name)
{
    if (!ws)
        return;
    free(ws->name);
    ws->name = strdup(name);
}

static struct container *workspace_get_local_focused_container(struct workspace *ws)
{
    if (!ws)
        return NULL;

    for (int i = 0; i < length_of_composed_list(ws->visible_focus_set->focus_stack_lists); i++) {
        struct container *con = get_in_composed_list(ws->visible_focus_set->focus_stack_lists, i);
        if (con->ws_id != ws->id)
            continue;
        return con;
    }
    return NULL;
}

void workspace_update_name(struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);
    if (!lt)
        return;
    int ws_id = ws->id;

    const char *default_name;
    if (ws_id < lt->options->tag_names->len) {
        default_name = g_ptr_array_index(lt->options->tag_names, ws_id);
    } else {
        default_name = ws->name;
    }
    struct container *con = workspace_get_local_focused_container(ws);

    const char *name = default_name;

    //TODO refactor
    char *num_name = strdup("");

    const char *app_id = container_get_app_id(con);
    if (con
            && app_id != NULL
            && g_strcmp0(app_id, "") != 0
       ) {

        name = app_id;

        char ws_number[12];
        char ws_name_number[12];
        sprintf(ws_number, "%lu:", ws->id);
        sprintf(ws_name_number, "%lu:", ws->id+1);
        append_string(&num_name, ws_number);
        append_string(&num_name, ws_name_number);
    }

    append_string(&num_name, name);

    char final_name[12];
    strncpy(final_name, num_name, 12);
    workspace_rename(ws, final_name);
    free(num_name);
}

void workspace_update_names(struct server *server, GList *workspaces)
{
    for (GList *iterator = workspaces; iterator; iterator = iterator->next) {
        struct workspace *ws = iterator->data;
        workspace_update_name(ws);
    }
}

struct container *workspace_get_focused_container(struct workspace *ws)
{
    if (!ws)
        return NULL;

    for (int i = 0; i < length_of_composed_list(ws->visible_focus_set->focus_stack_lists); i++) {
        struct container *con = get_in_composed_list(ws->visible_focus_set->focus_stack_lists, i);
        return con;
    }
    return NULL;
}

void list_set_add_container_to_containers(struct container_set *con_set, struct container *con, int i)
{
    list_insert(con_set->tiled_containers, i, con);
}

void workspace_add_container_to_containers(struct workspace *ws, int i, struct container *con)
{
    assert(con != NULL);

    DO_ACTION_GLOBALLY(server_get_workspaces(),
        list_set_add_container_to_containers(con_set, con, i);
    );
}

void workspace_remove_container_from_containers_locally(struct workspace *ws, struct container *con)
{
    struct layout *lt = workspace_get_layout(ws);
    if (lt->options->arrange_by_focus) {
        remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
        remove_in_composed_list(ws->visible_focus_set->focus_stack_lists, cmp_ptr, con);
    } else {
        DO_ACTION_LOCALLY(ws,
            g_ptr_array_remove(con_set->tiled_containers, con);
        );
    }
}

void workspace_add_container_to_containers_locally(struct workspace *ws, int i, struct container *con)
{
    struct layout *lt = workspace_get_layout(ws);
    if (lt->options->arrange_by_focus) {
        list_set_insert_container_to_focus_stack(ws->focus_set, 0, con);
        update_reduced_focus_stack(ws);
    } else {
        DO_ACTION_LOCALLY(ws,
            list_set_add_container_to_containers(con_set, con, i);
        );
    }
}

void list_set_insert_container_to_focus_stack(struct focus_set *focus_set, int position, struct container *con)
{
    if (con->client->type == LAYER_SHELL) {
        switch (con->client->surface.layer->current.layer) {
            case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
                list_insert(focus_set->focus_stack_layer_background, position, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
                list_insert(focus_set->focus_stack_layer_bottom, position, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
                list_insert(focus_set->focus_stack_layer_top, position, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
                list_insert(focus_set->focus_stack_layer_overlay, position, con);
                break;
        }
        return;
    }
    if (con->on_top) {
        list_insert(focus_set->focus_stack_on_top, position, con);
        return;
    }
    if (!con->focusable) {
        list_insert(focus_set->focus_stack_not_focusable, position, con);
        return;
    }

    list_insert(focus_set->focus_stack_normal, position, con);
}

void workspace_add_container_to_focus_stack(struct workspace *ws, int pos, struct container *con)
{
    // TODO: refactor me
    struct container *prev_sel = workspace_get_focused_container(ws);
    for (GList *iter = server_get_workspaces(); iter; iter = iter->next) {
        struct workspace *ws = iter->data;
        list_set_insert_container_to_focus_stack(ws->focus_set, pos, con);
        update_reduced_focus_stack(ws);
    }
    struct container *sel = workspace_get_focused_container(ws);
    if (prev_sel != sel) {
        struct event_handler *ev = server.event_handler;
        call_on_unfocus_function(ev, prev_sel);
        call_on_focus_function(ev, sel);
    }
}

void workspace_remove_container_from_focus_stack_locally(struct workspace *ws, struct container *con)
{
    remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
    update_reduced_focus_stack(ws);
}

void workspace_add_container_to_focus_stack_locally(struct workspace *ws, struct container *con)
{
    list_set_insert_container_to_focus_stack(ws->focus_set, 0, con);
    update_reduced_focus_stack(ws);
}

void workspace_remove_container_from_floating_stack_locally(struct workspace *ws, struct container *con)
{
    DO_ACTION_LOCALLY(ws, 
            g_ptr_array_remove(con_set->tiled_containers, con);
            );
}

void workspace_add_container_to_floating_stack_locally(struct workspace *ws, int i, struct container *con)
{
    DO_ACTION_LOCALLY(ws,
            g_ptr_array_insert(con_set->tiled_containers, i, con);
            );
}

void workspace_remove_container_from_visual_stack_layer(struct workspace *ws, struct container *con)
{
    remove_in_composed_list(server.layer_visual_stack_lists, cmp_ptr, con);
}

void workspace_add_container_to_visual_stack_layer(struct workspace *ws, struct container *con)
{
    add_container_to_layer_stack(ws, con);
}

void add_container_to_layer_stack(struct workspace *ws, struct container *con)
{
    assert(con->client->type == LAYER_SHELL);

    switch (con->client->surface.layer->current.layer) {
        case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
            g_ptr_array_insert(server.layer_visual_stack_background, 0, con);
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
            g_ptr_array_insert(server.layer_visual_stack_bottom, 0, con);
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
            g_ptr_array_insert(server.layer_visual_stack_top, 0, con);
            break;
        case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
            g_ptr_array_insert(server.layer_visual_stack_overlay, 0, con);
            break;
    }
    return;
}

void remove_container_from_stack(struct workspace *ws, struct container *con)
{
    g_ptr_array_remove(server.container_stack, con);
}

void add_container_to_stack(struct workspace *ws, struct container *con)
{
    if (!con)
        return;
    assert(con->client->type != LAYER_SHELL);

    g_ptr_array_insert(server.container_stack, 0, con);
}

void workspace_remove_container(struct workspace *ws, struct container *con)
{
    DO_ACTION_GLOBALLY(server_get_workspaces(),
            g_ptr_array_remove(con_set->tiled_containers, con);
            );
}

void workspace_remove_container_from_focus_stack(struct workspace *ws, struct container *con)
{
    struct container *prev_sel = workspace_get_focused_container(ws);
    for (GList *iter = server_get_workspaces(); iter; iter = iter->next) {
        struct workspace *ws = iter->data;
        remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
        remove_in_composed_list(ws->visible_focus_set->focus_stack_lists, cmp_ptr, con);
    }
    struct container *sel = workspace_get_focused_container(ws);
    if (prev_sel != sel) {
        struct event_handler *ev = server.event_handler;
        call_on_unfocus_function(ev, prev_sel);
        call_on_focus_function(ev, sel);
    }
}

void workspace_set_tags(struct workspace *ws, BitSet *tags)
{
    bitset_assign_bitset(&ws->workspaces, tags);
}

void workspace_set_prev_tags(struct workspace *ws, struct BitSet *tags)
{
    bitset_assign_bitset(&ws->prev_workspaces, tags);
}

static int get_in_container_stack(struct container *con)
{
    if (!con)
        return INVALID_POSITION;

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    guint position = 0;
    g_ptr_array_find(ws->con_set->tiled_containers, con, &position);
    return position;
}

GArray *container_array2D_get_positions_array(GPtrArray2D *containers)
{
    GArray *positions = g_array_new(false, false, sizeof(int));
    for (int i = 0; i < length_of_composed_list(containers); i++) {
        struct container *con = get_in_composed_list(containers, i);
        int position = get_in_container_stack(con);
        g_array_append_val(positions, position);
    }
    return positions;
}


GArray *container_array_get_positions_array(GPtrArray *containers)
{
    GArray *positions = g_array_new(false, false, sizeof(int));
    for (int i = 0; i < containers->len; i++) {
        struct container *con = g_ptr_array_index(containers, i);
        int position = get_in_container_stack(con);
        g_array_append_val(positions, position);
    }
    return positions;
}

void workspace_repush(GPtrArray *array, int i, int abs_index)
{
    if (array->len <= 0) {
        return;
    }

    if (i >= array->len) {
        i = array->len-1;
    }
    if (i < 0) {
        return;
    }
    if (abs_index >= array->len) {
        abs_index = array->len-1;
    }
    if (i < 0) {
        return;
    }

    struct container *con = g_ptr_array_steal_index(array, i);

    g_ptr_array_insert(array, abs_index, con);
}

void workspace_repush_workspace(struct workspace *ws, struct container *con, int new_pos)
{
    GPtrArray *tiled_list = workspace_get_tiled_list_copy(ws);
    g_ptr_array_remove(tiled_list, con);
    g_ptr_array_insert(tiled_list, new_pos, con);

    GPtrArray *actual_tiled_list = workspace_get_tiled_list(ws);
    sub_list_write_to_parent_list1D(actual_tiled_list, tiled_list);

    g_ptr_array_unref(tiled_list);

    workspace_write_to_workspaces(ws);
}

void workspace_repush_on_focus_stack(struct workspace *ws, struct container *con, int new_pos)
{
    remove_in_composed_list(ws->visible_focus_set->focus_stack_lists, cmp_ptr, con);
    list_set_insert_container_to_focus_stack(ws->visible_focus_set, new_pos, con);
    focus_set_write_to_parent(ws->focus_set, ws->visible_focus_set);
}

bool workspace_sticky_contains_client(struct workspace *ws, struct client *client)
{
    return bitset_test(client->sticky_workspaces, ws->id);
}
