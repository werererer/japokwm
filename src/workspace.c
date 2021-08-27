#include "workspace.h"

#include <assert.h>
#include <string.h>
#include <wayland-server.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_cursor.h>

#include "ipc-server.h"
#include "monitor.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "utils/parseConfigUtils.h"
#include "container.h"

static void update_workspaces_id(GPtrArray *workspaces)
{
    for (int id = 0; id < workspaces->len; id++) {
        struct workspace *ws = g_ptr_array_index(workspaces, id);
        ws->id = id;
    }
}

GPtrArray *create_workspaces(GPtrArray *tag_names)
{
    GPtrArray *workspaces = g_ptr_array_new();
    for (int i = 0; i < tag_names->len; i++) {
        const char *name = g_ptr_array_index(tag_names, i);
        struct workspace *ws = create_workspace(name, i, server.default_layout);
        g_ptr_array_add(workspaces, ws);
    }
    return workspaces;
}

struct workspace *create_workspace(const char *name, size_t id, struct layout *lt)
{
    struct workspace *ws = calloc(1, sizeof(struct workspace));
    ws->name = strdup(name);
    ws->id = id;

    ws->loaded_layouts = g_ptr_array_new();
    // fill layout stack with reasonable values
    push_layout(ws, lt);
    push_layout(ws, lt);

    ws->list_set = create_list_set();
    ws->subscribed_tagsets = g_ptr_array_new();
    return ws;
}

void copy_layout_from_selected_workspace(GPtrArray *workspaces)
{
    struct layout *src_lt = get_layout_in_monitor(selected_monitor);

    for (int i = 0; i < workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(workspaces, i);
        struct layout *dest_lt = ws->layout;
        struct layout *dest_prev_lt = ws->previous_layout;

        if (dest_lt == src_lt)
            continue;

        copy_layout(dest_lt, src_lt);
        copy_layout(dest_prev_lt, src_lt);
    }
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
            struct workspace *ws = g_ptr_array_steal_index(
                    server.workspaces,
                    server.workspaces->len);
            destroy_workspace(ws);
        }
    }

    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(server.workspaces, i);
        const char *name = g_ptr_array_index(tag_names, i);
        rename_workspace(ws, name);
    }
}

void destroy_workspace(struct workspace *ws)
{
    g_ptr_array_free(ws->subscribed_tagsets, TRUE);
    for (int i = 0; i < length_of_composed_list(ws->list_set->container_lists); i++) {
        struct container *con = get_in_composed_list(ws->list_set->container_lists, i);
        struct client *c = con->client;
        kill_client(c);
    }
    destroy_list_set(ws->list_set);
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

void destroy_workspaces(GPtrArray *workspaces)
{
    for (int i = 0; i < workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(workspaces, 0);
        destroy_workspace(ws);
    }
    g_ptr_array_free(workspaces, true);
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
        printf("prev_m\n");
        return true;
    }
    if (ws->tagset) {
        printf("tagset is visible\n");
        return tagset_is_visible(ws->tagset);
    }
    if (ws->selected_tagset) {
        return tagset_is_visible(ws->selected_tagset);
    }
    return false;
}

bool workspace_is_active(struct workspace *ws)
{
    struct monitor *m = workspace_get_monitor(ws);

    if (!m)
        return false;

    struct tagset *tagset = monitor_get_active_tagset(m);
    return bitset_test(tagset->loaded_workspaces, ws->id);
}

int get_workspace_container_count(struct workspace *ws)
{
    if (!ws)
        return -1;

    return length_of_composed_list(ws->list_set->visible_container_lists);
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
    return NULL;
}

struct workspace *get_workspace(int id)
{
    if (id < 0)
        return NULL;
    if (id >= server.workspaces->len)
        return NULL;

    return g_ptr_array_index(server.workspaces, id);
}

struct workspace *get_next_empty_workspace(GPtrArray *workspaces, size_t i)
{
    struct workspace *ws = NULL;
    for (int j = i; j < workspaces->len; j++) {
        struct workspace *ws = g_ptr_array_index(workspaces, j);
        if (is_workspace_empty(ws))
            break;
    }

    return ws;
}

struct workspace *get_prev_empty_workspace(GPtrArray *workspaces, size_t i)
{
    if (i >= workspaces->len)
        return NULL;

    struct workspace *ws = NULL;
    for (int j = i; j >= 0; j--) {
        struct workspace *ws = g_ptr_array_index(workspaces, j);
        if (is_workspace_empty(ws))
            break;
    }

    return ws;
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

void push_layout(struct workspace *ws, struct layout *lt)
{
    lt->ws_id = ws->id;
    ws->previous_layout = ws->layout;
    ws->layout = lt;
}

void load_default_layout(lua_State *L)
{
    load_layout(L, server.default_layout->name);
}

void load_layout(lua_State *L, const char *name)
{
    char *config_path = get_config_file("layouts");
    char *file = strdup("");
    join_path(&file, config_path);
    join_path(&file, name);
    join_path(&file, "init.lua");
    if (config_path)
        free(config_path);

    if (!file_exists(file))
        goto cleanup;

    if (luaL_loadfile(L, file)) {
        lua_pop(L, 1);
        goto cleanup;
    }
    lua_call_safe(L, 0, 0, 0);

cleanup:
    free(file);
}

void reset_loaded_layout(struct workspace *ws)
{
    int length = ws->loaded_layouts->len;
    for (int i = 0; i < length; i++) {
        struct layout *lt = g_ptr_array_index(ws->loaded_layouts, 0);
        destroy_layout(lt);
        g_ptr_array_remove_index(ws->loaded_layouts, 0);
    }
}

void remove_loaded_layouts(GPtrArray *workspaces)
{
    for (int i = 0; i < workspaces->len; i++) {
        struct workspace *ws = get_workspace(i);
        list_clear(ws->loaded_layouts, (void (*)(void *))destroy_layout);
    }
}

void focus_next_unoccupied_workspace(struct monitor *m, GPtrArray *workspaces, struct workspace *ws)
{
    struct workspace *w = find_next_unoccupied_workspace(workspaces, ws);

    if (!w)
        return;

    BitSet *bitset = bitset_create(server.workspaces->len);
    bitset_set(bitset, w->id);

    struct tagset *tagset = create_tagset(m, w->id, bitset);
    focus_tagset_no_ref(tagset);
}

void rename_workspace(struct workspace *ws, const char *name)
{
    if (!ws)
        return;
    free(ws->name);
    ws->name = strdup(name);
}

void workspace_add_container_to_containers(struct workspace *ws, struct container *con, int i)
{
    assert(con != NULL);

    DO_ACTION(ws,
        if (con->floating) {
            g_ptr_array_insert(list_set->floating_containers, i, con);
            continue;
        }
        if (con->hidden) {
            g_ptr_array_insert(list_set->hidden_containers, i, con);
            continue;
        }
        assert(list_set->tiled_containers->len >= i);
        if (list_set->tiled_containers->len <= 0) {
            g_ptr_array_add(list_set->tiled_containers, con);
        } else {
            g_ptr_array_insert(list_set->tiled_containers, i, con);
        }
    );
}

void list_set_add_container_to_focus_stack(struct list_set *list_set, struct container *con)
{
    if (con->client->type == LAYER_SHELL) {
        switch (con->client->surface.layer->current.layer) {
            case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
                g_ptr_array_insert(list_set->focus_stack_layer_background, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
                g_ptr_array_insert(list_set->focus_stack_layer_bottom, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
                g_ptr_array_insert(list_set->focus_stack_layer_top, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
                g_ptr_array_insert(list_set->focus_stack_layer_overlay, 0, con);
                break;
        }
        return;
    }
    if (con->on_top) {
        g_ptr_array_insert(list_set->focus_stack_on_top, 0, con);
        return;
    }
    if (!con->focusable) {
        g_ptr_array_insert(list_set->focus_stack_not_focusable, 0, con);
        return;
    }

    g_ptr_array_insert(list_set->focus_stack_normal, 0, con);
}

void workspace_add_container_to_focus_stack(struct workspace *ws, struct container *con)
{
    DO_ACTION(ws, 
            list_set_add_container_to_focus_stack(list_set, con);
            );
}

void add_container_to_stack(struct container *con)
{
    if (!con)
        return;

    if (con->client->type == LAYER_SHELL) {
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

    if (con->floating) {
        g_ptr_array_insert(server.floating_visual_stack, 0, con);
        return;
    }

    g_ptr_array_insert(server.tiled_visual_stack, 0, con);
}

void workspace_remove_container(struct workspace *ws, struct container *con)
{
    DO_ACTION(ws,
            remove_in_composed_list(list_set->container_lists, cmp_ptr, con);
            );
}

void workspace_remove_container_from_focus_stack(struct workspace *ws, struct container *con)
{
    DO_ACTION(ws,
            remove_in_composed_list(list_set->focus_stack_lists, cmp_ptr, con);
            );
}

void workspace_remove_independent_container(struct workspace *ws, struct container *con)
{
    DO_ACTION(ws,
            g_ptr_array_remove(list_set->independent_containers, con);
            );
}

// TODO refactor this function
void layout_set_set_layout(lua_State *L)
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

    load_layout(L, layout_name);
}
