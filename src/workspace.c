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

static void handle_too_few_workspaces(uint32_t ws_id);

static void handle_too_few_workspaces(uint32_t ws_id)
{
    // no number has more than 11 digits when int is 32 bit long
    char name[12];
    // TODO explain why +1
    sprintf(name, "%d:%d", server.workspaces->len, server.workspaces->len+1);
    while (ws_id >= server.workspaces->len)
    {
        struct workspace *new_ws = create_workspace(name, server.workspaces->len, server.default_layout);
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

void load_workspaces(GPtrArray *workspaces, GPtrArray *tag_names)
{
    while (server.workspaces->len > tag_names->len) {
        g_ptr_array_steal_index(server.workspaces, server.workspaces->len-1);
    }
    for (int i = 0; i < tag_names->len; i++) {
        struct workspace *ws = get_workspace(i);
        const char *name = g_ptr_array_index(tag_names, i);
        ws->current_layout = server.default_layout->symbol;
        ws->previous_layout = server.default_layout->symbol;
        workspace_remove_loaded_layouts(ws);
        workspace_rename(ws, name);
    }
}

void destroy_workspaces(GPtrArray *workspaces)
{
    g_ptr_array_unref(workspaces);
}

GPtrArray *create_workspaces()
{
    GPtrArray *workspaces = g_ptr_array_new_with_free_func(_destroy_workspace);
    return workspaces;
}

struct workspace *create_workspace(const char *name, size_t id, struct layout *lt)
{
    struct workspace *ws = calloc(1, sizeof(struct workspace));
    ws->name = strdup(name);
    ws->id = id;

    ws->loaded_layouts = g_ptr_array_new();
    ws->current_layout = lt->symbol;
    ws->previous_layout = lt->symbol;

    // TODO: findout which value must be used
    ws->workspaces = bitset_create();
    ws->prev_workspaces = bitset_create();
    bitset_set(ws->workspaces, ws->id);
    bitset_set(ws->prev_workspaces, ws->id);

    ws->focus_set = focus_set_create();
    ws->con_set = create_container_set();

    ws->consider_layer_shell = true;
    return ws;
}

void update_workspaces(GPtrArray *workspaces, GPtrArray *tag_names)
{
    if (tag_names->len > server.workspaces->len) {
        for (int i = server.workspaces->len-1; i < tag_names->len; i++) {
            const char *name = g_ptr_array_index(tag_names, 0);

            struct workspace *ws = create_workspace(name, i, server.default_layout);
            g_ptr_array_add(server.workspaces, ws);
        }
    } else {
        int tile_containers_length = server.workspaces->len;
        for (int i = tag_names->len; i < tile_containers_length; i++) {
            g_ptr_array_steal_index(server.workspaces, server.workspaces->len);
        }
    }

    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(server.workspaces, i);
        const char *name = g_ptr_array_index(tag_names, i);
        workspace_rename(ws, name);
    }
}

void focus_most_recent_container()
{
    struct monitor *m = server_get_selected_monitor();
    struct tagset *tagset = monitor_get_active_tagset(m);

    struct container *con = get_focused_container(m);
    // TODO: this can be exported to an external function
    if (!con) {
        if (tagset->con_set->tiled_containers->len <= 0)
            return;
        struct container *con = g_ptr_array_index(tagset->con_set->tiled_containers, 0);
        if (!con)
            return;
    }

    focus_container(con);
}

struct container *get_container(struct workspace *ws, int i)
{
    struct tagset *tagset = workspace_get_active_tagset(ws);
    return get_in_composed_list(tagset->visible_focus_set->focus_stack_visible_lists, i);
}

struct container *get_container_in_stack(struct workspace *ws, int i)
{
    struct tagset *tagset = workspace_get_active_tagset(ws);
    GPtrArray *visible_list = tagset_get_visible_list_copy(tagset);
    struct container *con = g_ptr_array_index(visible_list, i);
    return con;
}

void destroy_workspace(struct workspace *ws)
{
    for (int i = 0; i < ws->loaded_layouts->len; i++) {
        struct layout *lt = g_ptr_array_steal_index(ws->loaded_layouts, 0);
        destroy_layout(lt);
    }
    g_ptr_array_unref(ws->loaded_layouts);

    workspace_remove_loaded_layouts(ws);

    bitset_destroy(ws->workspaces);
    bitset_destroy(ws->prev_workspaces);
    focus_set_destroy(ws->focus_set);

    for (int i = 0; i < ws->con_set->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(ws->con_set->tiled_containers, i);
        struct client *c = con->client;

        if (c->ws_id != ws->id)
            continue;

        kill_client(c);
    }
    destroy_container_set(ws->con_set);
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
    bool is_occupied = (m != NULL) && workspace_is_visible(ws);
    return is_occupied;
}

bool workspace_is_visible(struct workspace *ws)
{
    assert(ws != NULL);

    if (ws->prev_m && !is_workspace_empty(ws)) {
        return true;
    }
    if (ws->tagset) {
        return tagset_is_visible(ws->tagset);
    }
    if (ws->selected_tagset) {
        return tagset_is_visible(ws->selected_tagset);
    }
    return false;
}

bool is_workspace_extern(struct workspace *ws)
{
    if (!ws->selected_tagset)
        return false;
    if (!ws->tagset)
        return false;
    bool is_extern = ws->tagset->m != ws->selected_tagset->m;
    return is_extern;
}

bool is_workspace_the_selected_one(struct workspace *ws)
{
    if (!ws->selected_tagset)
        return false;
    return ws->selected_tagset->selected_ws_id == ws->id
        && tagset_is_visible(ws->selected_tagset)
        && !is_workspace_extern(ws);
}

bool workspace_is_active(struct workspace *ws)
{
    struct monitor *m = workspace_get_monitor(ws);

    if (!m)
        return false;

    struct tagset *tagset = monitor_get_active_tagset(m);
    return bitset_test(tagset->workspaces, ws->id);
}

int get_workspace_container_count(struct workspace *ws)
{
    if (!ws)
        return -1;

    int count = 0;
    for (int i = 0; i < ws->con_set->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(ws->con_set->tiled_containers, i);
        if (con->client->ws_id == ws->id) {
            count++;
        }
    }
    return count;
}

bool is_workspace_empty(struct workspace *ws)
{
    return get_workspace_container_count(ws) == 0;
}

struct workspace *find_next_unoccupied_workspace(GPtrArray *workspaces, struct workspace *ws)
{
    for (size_t i = ws ? ws->id : 0; i < workspaces->len; i++) {
        struct workspace *w = g_ptr_array_index(workspaces, i);
        if (!w)
            break;
        if (!is_workspace_occupied(w))
            return w;
    }
    struct workspace *w = get_workspace(server_get_workspace_count());
    return w;
}

struct workspace *get_workspace(int id)
{
    if (id < 0)
        return NULL;
    if (id >= server.workspaces->len) {
        handle_too_few_workspaces(id);
    }

    return g_ptr_array_index(server.workspaces, id);
}

struct workspace *get_next_empty_workspace(GPtrArray *workspaces, size_t ws_id)
{
    for (int i = ws_id; i < workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(workspaces, i);
        if (is_workspace_empty(ws)) {
            return ws;
        }
    }

    for (int i = ws_id-1; i >= 0; i--) {
        struct workspace *ws = g_ptr_array_index(workspaces, i);
        if (is_workspace_empty(ws)) {
            return ws;
        }
    }

    return NULL;
}

struct workspace *get_nearest_empty_workspace(GPtrArray *workspaces, int ws_id)
{
    struct workspace *initial_workspace = get_workspace(ws_id);
    if (is_workspace_empty(initial_workspace)) {
        return initial_workspace;
    }

    struct workspace *ws = NULL;
    for (int i = 0, up_counter = ws_id+1, down_counter = ws_id-1;
            i < workspaces->len;
            i++,up_counter++,down_counter--) {

        bool is_up_counter_valid = up_counter < workspaces->len;
        bool is_down_counter_valid = down_counter >= 0;
        if (is_down_counter_valid) {
            ws = g_ptr_array_index(workspaces, down_counter);
            if (is_workspace_empty(ws))
                break;
        }
        if (is_up_counter_valid) {
            ws = g_ptr_array_index(workspaces, up_counter);
            if (is_workspace_empty(ws))
                break;
        }
        if (!is_up_counter_valid && !is_down_counter_valid) {
            break;
        }
    }

    return ws;
}


struct workspace *get_prev_empty_workspace(GPtrArray *workspaces, size_t ws_id)
{
    for (int i = ws_id; i >= 0; i--) {
        struct workspace *ws = g_ptr_array_index(workspaces, i);
        if (is_workspace_empty(ws)) {
            return ws;
        }
    }

    for (int i = ws_id+1; i < workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(workspaces, i);
        if (is_workspace_empty(ws)) {
            return ws;
        }
    }

    return NULL;
}

struct tagset *workspace_get_selected_tagset(struct workspace *ws)
{
    struct tagset *tagset = ws->selected_tagset;
    return tagset;
}

struct tagset *workspace_get_tagset(struct workspace *ws)
{
    return ws->tagset;
}

struct tagset *workspace_get_active_tagset(struct workspace *ws)
{
    if (!ws)
        return NULL;
    struct tagset *tagset = ws->tagset;
    if (!tagset) {
        tagset = ws->selected_tagset;
        if (!tagset)
            return NULL;
    }
    return tagset;
}

struct layout *workspace_get_layout(struct workspace *ws)
{
    if (!ws)
        return NULL;
    if (ws->loaded_layouts->len <= 0)
        return NULL;
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
    if (ws->consider_layer_shell) {
        struct root *root = workspace_get_root(ws); geom = root->geom;
    } else {
        struct monitor *m = workspace_get_monitor(ws);
        geom = m->geom;
    }
    return geom;
}

struct monitor *workspace_get_selected_monitor(struct workspace *ws)
{
    assert(ws != NULL);
    if (!ws->selected_tagset)
        return NULL;
    return ws->selected_tagset->m;
}

struct monitor *workspace_get_monitor(struct workspace *ws)
{
    assert(ws != NULL);

    if (ws->tagset) {
        return ws->tagset->m;
    }
    if (ws->selected_tagset) {
        return ws->selected_tagset->m;
    }
    if (ws->prev_m && !is_workspace_empty(ws)) {
        return ws->prev_m;
    }
    return NULL;
}

void push_layout(struct workspace *ws, const char *layout_name)
{
    ws->previous_layout = ws->current_layout;
    ws->current_layout = layout_name;
}

void set_default_layout(struct workspace *ws)
{
    push_layout(ws, server.default_layout->symbol);
}

static void load_layout_file(lua_State *L, struct layout *lt)
{
    struct workspace *ws = get_workspace(lt->ws_id);
    struct monitor *m = workspace_get_monitor(ws);
    init_local_config_variables(L, m);
    const char *name = lt->symbol;

    char *config_path = get_config_file("layouts");
    char *file = strdup("");
    join_path(&file, config_path);
    join_path(&file, name);
    join_path(&file, "init.lua");
    if (config_path)
        free(config_path);

    if (!file_exists(file))
        goto cleanup;

    if (load_file(L, file) != EXIT_SUCCESS) {
        lua_pop(L, 1);
        goto cleanup;
    }

cleanup:
    free(file);
}

void load_layout(struct monitor *m)
{
    struct workspace *ws = monitor_get_active_workspace(m);
    const char *name = ws->current_layout;
    assert(name != NULL);

    guint i;
    bool found = g_ptr_array_find_with_equal_func(ws->loaded_layouts, name, cmp_layout_to_string, &i);
    if (found) {
        struct layout *lt = g_ptr_array_steal_index(ws->loaded_layouts, i);
        g_ptr_array_insert(ws->loaded_layouts, 0, lt);
        push_layout(ws, lt->symbol);
    } else {
        struct layout *lt = create_layout(L);

        lt->ws_id = ws->id;
        copy_layout_safe(lt, server.default_layout);

        lt->symbol = name;

        g_ptr_array_insert(ws->loaded_layouts, 0, lt);
        push_layout(ws, lt->symbol);

        load_layout_file(L, lt);
    }
    int layout_index = server.layout_set.lua_layout_index;
    server.layout_set.lua_layout_index = layout_index;
}

static void destroy_layout0(void *lt)
{
    destroy_layout(lt);
}

void workspace_remove_loaded_layouts(struct workspace *ws)
{
    list_clear(ws->loaded_layouts, destroy_layout0);
}

void workspaces_remove_loaded_layouts(GPtrArray *workspaces)
{
    for (int i = 0; i < workspaces->len; i++) {
        struct workspace *ws = get_workspace(i);
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

void workspace_update_names(struct server *server, GPtrArray *workspaces)
{
    struct layout *lt = server->default_layout;
    if (!lt->options->automatic_workspace_naming)
        return;
    for (int i = 0; i < workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(workspaces, i);
        const char *default_name;
        if (i < lt->options->tag_names->len) {
            default_name = g_ptr_array_index(lt->options->tag_names, i);
        } else {
            default_name = ws->name;
        }
        struct container *con = workspace_get_focused_container(ws);

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
    ipc_event_workspace();
}

struct container *workspace_get_focused_container(struct workspace *ws)
{
    for (int i = 0; i < length_of_composed_list(ws->focus_set->focus_stack_lists); i++) {
        struct container *con = get_in_composed_list(ws->focus_set->focus_stack_lists, i);
        if (con->client->ws_id == ws->id) {
            return con;
        }
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

    DO_ACTION_GLOBALLY(server.workspaces,
        list_set_add_container_to_containers(con_set, con, i);
    );
}

void workspace_remove_container_from_containers_locally(struct workspace *ws, struct container *con)
{
    struct tagset *tagset = workspace_get_active_tagset(ws);
    struct layout *lt = tagset_get_layout(tagset);
    if (lt->options->arrange_by_focus) {
        remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
        remove_in_composed_list(tagset->visible_focus_set->focus_stack_lists, cmp_ptr, con);
    } else {
        DO_ACTION_LOCALLY(ws,
            g_ptr_array_remove(con_set->tiled_containers, con);
        );
    }
}

void workspace_add_container_to_containers_locally(struct workspace *ws, int i, struct container *con)
{
    struct tagset *tagset = workspace_get_active_tagset(ws);
    struct layout *lt = tagset_get_layout(tagset);
    if (lt->options->arrange_by_focus) {
        list_set_insert_container_to_focus_stack(ws->focus_set, 0, con);
        update_reduced_focus_stack(tagset);
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
    for (int i = 0; i < server.workspaces->len; i++) {
        ws = g_ptr_array_index(server.workspaces, i);
        list_set_insert_container_to_focus_stack(ws->focus_set, pos, con);
    }
    for (int i = 0; i < server.tagsets->len; i++) {
        struct tagset *tagset = g_ptr_array_index(server.tagsets, i);
        update_reduced_focus_stack(tagset);
    }
}

void workspace_remove_container_from_focus_stack_locally(struct workspace *ws, struct container *con)
{
    remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
    struct tagset *tagset = workspace_get_tagset(ws);
    update_reduced_focus_stack(tagset);
}

void workspace_add_container_to_focus_stack_locally(struct workspace *ws, struct container *con)
{
    list_set_insert_container_to_focus_stack(ws->focus_set, 0, con);
    struct tagset *tagset = workspace_get_tagset(ws);
    update_reduced_focus_stack(tagset);
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
    DO_ACTION_GLOBALLY(server.workspaces,
            g_ptr_array_remove(con_set->tiled_containers, con);
            );
}

void workspace_remove_container_from_focus_stack(struct workspace *ws, struct container *con)
{
    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(server.workspaces, i);
        remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
    }

    for (int i = 0; i < server.tagsets->len; i++) {
        struct tagset *tagset = g_ptr_array_index(server.tagsets, i);
        remove_in_composed_list(tagset->visible_focus_set->focus_stack_lists, cmp_ptr, con);
    }
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

void workspace_repush(struct workspace *ws, struct container *con, int new_pos)
{
    struct tagset *tagset = workspace_get_active_tagset(ws);

    GPtrArray *tiled_list = tagset_get_tiled_list_copy(tagset);
    g_ptr_array_remove(tiled_list, con);
    g_ptr_array_insert(tiled_list, new_pos, con);

    GPtrArray *actual_tiled_list = tagset_get_tiled_list(tagset);
    sub_list_write_to_parent_list1D(actual_tiled_list, tiled_list);
    debug_print("new_pos: %i\n", new_pos);
    debug_print("tiled list len: %i\n", tiled_list);
    debug_print("actual tiled list len: %i\n", actual_tiled_list);

    g_ptr_array_unref(tiled_list);

    tagset_write_to_workspaces(tagset);
}

void workspace_repush_on_focus_stack(struct workspace *ws, struct container *con, int new_pos)
{
    struct tagset *tagset = workspace_get_active_tagset(ws);

    remove_in_composed_list(tagset->visible_focus_set->focus_stack_lists, cmp_ptr, con);
    list_set_insert_container_to_focus_stack(tagset->visible_focus_set, new_pos, con);
    focus_set_write_to_parent(ws->focus_set, tagset->visible_focus_set);
}

bool workspace_sticky_contains_client(struct workspace *ws, struct client *client)
{
    return bitset_test(client->sticky_workspaces, ws->id);
}

// TODO refactor this function
void layout_set_set_layout(struct workspace *ws)
{
    if (server.layout_set.layout_sets_ref <= 0) {
        return;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    if (!lua_is_index_defined(L, server.layout_set.key)) {
        lua_pop(L, 1);
        return;
    }
    lua_get_layout_set_element(L, server.layout_set.key);

    lua_rawgeti(L, -1, server.layout_set.lua_layout_index);
    const char *layout_name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    lua_pop(L, 1);

    lua_pop(L, 1);

    push_layout(ws, layout_name);
}
