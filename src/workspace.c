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

static void update_workspaces_id(struct wlr_list *workspaces)
{
    int id = 0;
    for (int i = 0; i < workspaces->length; i++) {
        struct workspace *ws = workspaces->items[i];
        ws->id = id;
        id++;
    }
}

struct workspace *create_workspace(const char *name, size_t id)
{
    struct workspace *ws = calloc(1, sizeof(struct workspace));
    ws->name = name;
    ws->id = id;

    setup_list_set(&ws->list_set);
    return ws;
}

void update_workspaces(struct wlr_list *workspaces, struct wlr_list *tag_names)
{
    if (tag_names->length > server.workspaces.length) {
        for (int i = server.workspaces.length-1; i < tag_names->length; i++) {
            const char *name = tag_names->items[0];

            struct workspace *ws = create_workspace(name, i);
            wlr_list_push(&server.workspaces, ws);
        }
    } else {
        int tile_containers_length = server.workspaces.length;
        for (int i = tag_names->length; i < tile_containers_length; i++) {
            struct workspace *ws = wlr_list_pop(&server.workspaces);
            destroy_workspace(ws);
        }
    }

    for (int i = 0; i < server.workspaces.length; i++) {
        struct workspace *ws = server.workspaces.items[i];
        rename_workspace(ws, tag_names->items[i]);
    }
}

void destroy_workspace(struct workspace *ws)
{
    for (int i = 0; i < length_of_composed_list(&ws->list_set.container_lists); i++) {
        struct container *con = get_in_composed_list(&ws->list_set.container_lists, i);
        struct client *c = con->client;
        kill_client(c);
    }
    free(ws);
}

void update_workspace_ids(struct wlr_list *workspaces)
{
    for (int i = 0; i < workspaces->length; i++) {
        struct workspace *ws = workspaces->items[i];
        ws->id = i;
    }
}

void create_workspaces(struct wlr_list *workspaces, struct wlr_list *tag_names)
{
    wlr_list_init(workspaces);
    for (int i = 0; i < tag_names->length; i++) {
        struct workspace *ws = create_workspace(tag_names->items[i], i);
        wlr_list_push(workspaces, ws);
    }
}

void destroy_workspaces(struct wlr_list *workspaces)
{
    for (int i = 0; i < workspaces->length; i++)
        destroy_workspace(wlr_list_pop(workspaces));
    wlr_list_finish(workspaces);
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

    return ws->list_set.tiled_containers.length;
}

bool is_workspace_empty(struct workspace *ws)
{
    return get_workspace_container_count(ws) == 0;
}

struct workspace *find_next_unoccupied_workspace(struct wlr_list *workspaces, struct workspace *ws)
{
    for (size_t i = ws ? ws->id : 0; i < workspaces->length; i++) {
        struct workspace *w = workspaces->items[i];
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
    if (id >= server.workspaces.length)
        return NULL;

    return server.workspaces.items[id];
}

struct workspace *get_next_empty_workspace(struct wlr_list *workspaces, size_t i)
{
    struct workspace *ws = NULL;
    for (int j = i; j < workspaces->length; j++) {
        struct workspace *ws = workspaces->items[j];
        if (is_workspace_empty(ws))
            break;
    }

    return ws;
}

struct workspace *get_prev_empty_workspace(struct wlr_list *workspaces, size_t i)
{
    if (i >= workspaces->length)
        return NULL;

    struct workspace *ws = NULL;
    for (int j = i; j >= 0; j--) {
        struct workspace *ws = workspaces->items[j];
        if (is_workspace_empty(ws))
            break;
    }

    return ws;
}

void workspace_assign_monitor(struct workspace *ws, struct monitor *m)
{
    ws->m = m;
}

void move_container_to_workspace(struct container *con, struct workspace *ws)
{
    if (!ws)
        return;
    if (!con)
        return;
    if (con->client->type == LAYER_SHELL)
        return;

    set_container_workspace(con, ws);
    con->client->moved_workspace = true;
    container_damage_whole(con);

    arrange();
    struct tagset *selected_tagset = monitor_get_active_tagset(con->m);
    focus_most_recent_container(selected_tagset, FOCUS_NOOP);

    ipc_event_workspace();
}

void focus_next_unoccupied_workspace(struct monitor *m, struct wlr_list *workspaces, struct workspace *ws)
{
    struct workspace *w = find_next_unoccupied_workspace(workspaces, ws);

    if (!w)
        return;

    BitSet bitset;
    bitset_setup(&bitset, server.workspaces.length);
    bitset_set(&bitset, w->id);

    struct tagset *tagset = create_tagset(m, server.default_layout, w->id, bitset);
    focus_tagset(tagset);
}

void rename_workspace(struct workspace *ws, const char *name)
{
    if (!ws)
        return;
    ws->name = name;
}

void set_container_workspace(struct container *con, struct workspace *ws)
{
    if (!con)
        return;
    if (!ws)
        return;
    if (con->m->tagset->selected_ws_id == ws->id)
        return;

    struct workspace *sel_ws = monitor_get_active_workspace(con->m);

    if (ws->m) {
        set_container_monitor(con, ws->m);
    } else {
        ws->m = con->m;
    }
    printf("set container workspace: %zu\n", ws->id);
    con->client->ws_id = ws->id;

    remove_in_composed_list(&sel_ws->list_set.container_lists, cmp_ptr, con);
    add_container_to_containers(&ws->list_set, con, 0);

    remove_in_composed_list(&sel_ws->list_set.focus_stack_lists, cmp_ptr, con);
    add_container_to_focus_stack(&ws->list_set, con);

    struct tagset *ts = get_tagset_from_workspace_id(&server.workspaces, ws->id);
    if (con->floating)
        con->client->bw = ts->layout->options.float_border_px;
    else
        con->client->bw = ts->layout->options.tile_border_px;
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
