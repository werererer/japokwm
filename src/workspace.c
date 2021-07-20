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
    int id = 0;
    for (int i = 0; i < workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(workspaces, i);
        ws->id = id;
        id++;
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
    ws->name = name;
    ws->id = id;

    ws->loaded_layouts = g_ptr_array_new();
    printf("lt->lua_resize_data_ref: %i\n", lt->lua_resize_data_ref);
    // fill layout stack with reasonable values
    push_layout(ws, lt);
    push_layout(ws, lt);

    ws->list_set = create_list_set();
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
    for (int i = 0; i < length_of_composed_list(ws->list_set->container_lists); i++) {
        struct container *con = get_in_composed_list(ws->list_set->container_lists, i);
        struct client *c = con->client;
        kill_client(c);
    }
    destroy_list_set(ws->list_set);
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

    return ws->m ? true : false;
}

int get_workspace_container_count(struct workspace *ws)
{
    if (!ws)
        return -1;

    return ws->list_set->tiled_containers->len;
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

void workspace_assign_monitor(struct workspace *ws, struct monitor *m)
{
    ws->m = m;
}

void push_layout(struct workspace *ws, struct layout *lt)
{
    printf("push_layout: lt->lua_resize_function_ref: %i\n", lt->lua_resize_function_ref);
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
        wlr_list_clear(ws->loaded_layouts, (void (*)(void *))destroy_layout);
    }
}

void focus_next_unoccupied_workspace(struct monitor *m, GPtrArray *workspaces, struct workspace *ws)
{
    struct workspace *w = find_next_unoccupied_workspace(workspaces, ws);

    if (!w)
        return;

    BitSet bitset;
    bitset_setup(&bitset, server.workspaces->len);
    bitset_set(&bitset, w->id);

    struct tagset *tagset = create_tagset(m, w->id, bitset);
    focus_tagset(tagset);
}

void rename_workspace(struct workspace *ws, const char *name)
{
    if (!ws)
        return;
    ws->name = name;
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
