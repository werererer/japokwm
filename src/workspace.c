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

static void setup_lists(struct workspace *ws)
{
    wlr_list_init(&ws->loaded_layouts);

    wlr_list_init(&ws->container_lists);
    wlr_list_init(&ws->visible_container_lists);

    wlr_list_init(&ws->independent_containers);
    wlr_list_init(&ws->tiled_containers);
    wlr_list_init(&ws->hidden_containers);
    wlr_list_init(&ws->floating_containers);

    wlr_list_push(&ws->container_lists, &ws->tiled_containers);
    wlr_list_push(&ws->container_lists, &ws->floating_containers);
    wlr_list_push(&ws->container_lists, &ws->hidden_containers);

    wlr_list_push(&ws->visible_container_lists, &ws->tiled_containers);
    wlr_list_push(&ws->visible_container_lists, &ws->floating_containers);

    wlr_list_init(&ws->focus_stack_lists);
    wlr_list_init(&ws->focus_stack_visible_lists);
    wlr_list_init(&ws->focus_stack_lists_with_layer_shell);

    wlr_list_init(&ws->focus_stack_layer_background);
    wlr_list_init(&ws->focus_stack_layer_bottom);
    wlr_list_init(&ws->focus_stack_layer_top);
    wlr_list_init(&ws->focus_stack_layer_overlay);
    wlr_list_init(&ws->focus_stack_layer_bottom);
    wlr_list_init(&ws->focus_stack_on_top);
    wlr_list_init(&ws->focus_stack_normal);
    wlr_list_init(&ws->focus_stack_hidden);
    wlr_list_init(&ws->focus_stack_not_focusable);

    wlr_list_push(&ws->focus_stack_lists, &ws->focus_stack_layer_top);
    wlr_list_push(&ws->focus_stack_lists, &ws->focus_stack_on_top);
    wlr_list_push(&ws->focus_stack_lists, &ws->focus_stack_normal);
    wlr_list_push(&ws->focus_stack_lists, &ws->focus_stack_not_focusable);
    wlr_list_push(&ws->focus_stack_lists, &ws->focus_stack_hidden);

    wlr_list_push(&ws->focus_stack_lists_with_layer_shell, &ws->focus_stack_layer_overlay);
    wlr_list_push(&ws->focus_stack_lists_with_layer_shell, &ws->focus_stack_layer_top);
    wlr_list_push(&ws->focus_stack_lists_with_layer_shell, &ws->focus_stack_on_top);
    wlr_list_push(&ws->focus_stack_lists_with_layer_shell, &ws->focus_stack_normal);
    wlr_list_push(&ws->focus_stack_lists_with_layer_shell, &ws->focus_stack_not_focusable);
    wlr_list_push(&ws->focus_stack_lists_with_layer_shell, &ws->focus_stack_layer_bottom);
    wlr_list_push(&ws->focus_stack_lists_with_layer_shell, &ws->focus_stack_layer_background);

    wlr_list_push(&ws->focus_stack_visible_lists, &ws->focus_stack_on_top);
    wlr_list_push(&ws->focus_stack_visible_lists, &ws->focus_stack_normal);
    wlr_list_push(&ws->focus_stack_visible_lists, &ws->focus_stack_not_focusable);
}

struct workspace *create_workspace(const char *name, size_t id, struct layout *lt)
{
    struct workspace *ws = calloc(1, sizeof(struct workspace));
    ws->name = name;
    ws->id = id;

    setup_lists(ws);

    // fill layout stack with reasonable values
    push_layout(ws, lt);
    push_layout(ws, lt);
    return ws;
}

void update_workspaces(struct wlr_list *workspaces, struct wlr_list *tag_names)
{
    if (tag_names->length > server.workspaces.length) {
        for (int i = server.workspaces.length-1; i < tag_names->length; i++) {
            const char *name = tag_names->items[0];

            struct workspace *ws = create_workspace(name, i, server.default_layout);
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
    for (int i = 0; i < length_of_composed_list(&ws->container_lists); i++) {
        struct container *con = get_in_composed_list(&ws->container_lists, i);
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

void create_workspaces(struct wlr_list *workspaces, struct wlr_list *tag_names,
        struct layout *default_layout)
{
    wlr_list_init(workspaces);
    for (int i = 0; i < tag_names->length; i++) {
        struct workspace *ws = create_workspace(tag_names->items[i], i, default_layout);
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

static bool container_intersects_with_monitor(struct container *con, struct monitor *m)
{
    if (!con)
        return false;
    if (!m)
        return false;

    struct wlr_box tmp_geom;
    return wlr_box_intersection(&tmp_geom, &con->geom, &m->geom);
}

bool exist_on(struct container *con, struct workspace *ws)
{
    if (!con || !ws)
        return false;
    if (con->m != ws->m) {
        if (con->floating)
            return container_intersects_with_monitor(con, ws->m)
                && workspace_contains_client(get_workspace(con->m->ws_id), con->client);
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

    return workspace_contains_client(ws, c);
}

bool workspace_contains_client(struct workspace *ws, struct client *c)
{
    if (!ws)
        return false;
    if (!c)
        return false;

    return ws->id == c->ws_id;
}

bool workspace_has_clients(struct workspace *ws)
{
    if (!ws)
        return false;

    for (int i = 0; i < server.normal_clients.length; i++) {
        struct client *c = server.normal_clients.items[i];

        if (workspace_contains_client(ws, c))
            return true;
    }

    return false;
}

bool hidden_on(struct container *con, struct workspace *ws)
{
    return !visible_on(con, ws) && exist_on(con, ws);
}

bool visible_on(struct container *con, struct workspace *ws)
{
    if (!con)
        return false;
    if (con->hidden)
        return false;

    return exist_on(con, ws);
}

int get_workspace_container_count(struct workspace *ws)
{
    if (!ws)
        return -1;

    int i = 0;
    for (int i = 0; i < ws->tiled_containers.length; i++) {
        struct container *con = get_container(ws, i);

        if (visible_on(con, ws))
            i++;
    }
    return i;
}

struct container *get_container(struct workspace *ws, int i)
{
    if (!ws)
        return NULL;

    return get_in_composed_list(&ws->container_lists, i);
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

struct wlr_list *get_visible_lists(struct workspace *ws)
{
    struct layout *lt = ws->layout;

    if (lt->options.arrange_by_focus)
        return &ws->focus_stack_visible_lists;
    else
        return &ws->visible_container_lists;
}

struct wlr_list *get_tiled_list(struct workspace *ws)
{
    struct layout *lt = ws->layout;

    if (lt->options.arrange_by_focus)
        return &ws->focus_stack_normal;
    else
        return &ws->tiled_containers;
}

struct wlr_list *get_floating_list(struct workspace *ws)
{
    struct layout *lt = ws->layout;

    if (lt->options.arrange_by_focus)
        return &ws->focus_stack_normal;
    else
        return &ws->floating_containers;
}

struct wlr_list *get_hidden_list(struct workspace *ws)
{
    struct layout *lt = ws->layout;

    if (lt->options.arrange_by_focus)
        return &ws->focus_stack_hidden;
    else
        return &ws->hidden_containers;
}

void workspace_assign_monitor(struct workspace *ws, struct monitor *m)
{
    ws->m = m;
}

void set_selected_layout(struct workspace *ws, struct layout *layout)
{
    if (!ws)
        return;

    if (strcmp(ws->name, "") == 0) {
        wlr_log(WLR_ERROR, "ERROR: tag not initialized");
        return;
    }
    push_layout(ws, layout);
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
    struct workspace *selected_workspace = monitor_get_active_workspace(con->m);
    focus_most_recent_container(selected_workspace, FOCUS_NOOP);

    ipc_event_workspace();
}

void add_container_to_containers(struct container *con, struct workspace *ws, int i)
{
    if (!con)
        return;
    if (!ws)
        return;

    if (con->floating) {
        wlr_list_insert(&ws->floating_containers, i, con);
        return;
    }
    if (con->hidden) {
        wlr_list_insert(&ws->hidden_containers, i, con);
        return;
    }
    wlr_list_insert(&ws->tiled_containers, i, con);
}

void add_container_to_focus_stack(struct container *con, struct workspace *ws)
{
    if (con->client->type == LAYER_SHELL) {
        switch (con->client->surface.layer->current.layer) {
            case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
                wlr_list_insert(&ws->focus_stack_layer_background, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
                wlr_list_insert(&ws->focus_stack_layer_bottom, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
                wlr_list_insert(&ws->focus_stack_layer_top, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
                wlr_list_insert(&ws->focus_stack_layer_overlay, 0, con);
                break;
        }
        return;
    }
    if (con->on_top) {
        wlr_list_insert(&ws->focus_stack_on_top, 0, con);
        return;
    }
    if (!con->focusable) {
        wlr_list_insert(&ws->focus_stack_not_focusable, 0, con);
        return;
    }

    wlr_list_insert(&ws->focus_stack_normal, 0, con);
}

void add_container_to_stack(struct container *con)
{
    if (!con)
        return;

    if (con->client->type == LAYER_SHELL) {
        switch (con->client->surface.layer->current.layer) {
            case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
                wlr_list_insert(&server.layer_visual_stack_background, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
                wlr_list_insert(&server.layer_visual_stack_bottom, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
                wlr_list_insert(&server.layer_visual_stack_top, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
                wlr_list_insert(&server.layer_visual_stack_overlay, 0, con);
                break;
        }
        return;
    }

    if (con->floating) {
        wlr_list_insert(&server.floating_visual_stack, 0, con);
        return;
    }

    wlr_list_insert(&server.tiled_visual_stack, 0, con);
}

void focus_most_recent_container(struct workspace *ws, enum focus_actions a)
{
    struct container *con = get_in_composed_list(&ws->focus_stack_lists, 0);

    if (!con) {
        con = get_container(ws, 0);
        if (!con) {
            ipc_event_window();
            return;
        }
    }

    focus_container(con, a);
}

void focus_next_unoccupied_workspace(struct monitor *m, struct wlr_list *workspaces, struct workspace *ws)
{
    struct workspace *w = find_next_unoccupied_workspace(workspaces, ws);

    if (!w)
        return;

    focus_workspace(m, w);
}

void rename_workspace(struct workspace *ws, const char *name)
{
    if (!ws)
        return;
    ws->name = name;
}

void focus_workspace(struct monitor *m, struct workspace *ws)
{
    if (!m || !ws)
        return;
    assert(m->damage != NULL);

    // focus the workspace in the monitor it appears in if such a monitor exist
    // and is not the selected one
    if (is_workspace_occupied(ws) && ws->m != selected_monitor) {
        struct workspace *wss = monitor_get_active_workspace(m);
        for (int i = 0; i < wss->floating_containers.length; i++) {
            struct container *con = wss->floating_containers.items[i];
            move_container_to_workspace(con, ws);
        }

        center_mouse_in_monitor(ws->m);
        selected_monitor = ws->m;
        focus_workspace(ws->m, ws);
        return;
    }

    struct container *con;
    wl_list_for_each(con, &sticky_stack, stlink) {
        con->client->ws_id = ws->id;
    }

    ipc_event_workspace();

    struct workspace *old_ws = monitor_get_active_workspace(m);
    // unset old workspace
    if (old_ws && !workspace_has_clients(old_ws)) {
        struct workspace *old_ws = monitor_get_active_workspace(m);
        old_ws->m = NULL;
    }

    m->ws_id = ws->id;
    ws->m = m;

    arrange();
    focus_most_recent_container(ws, FOCUS_NOOP);
    root_damage_whole(m->root);
}

void copy_layout_from_selected_workspace(struct wlr_list *workspaces)
{
    struct layout *src_lt = get_layout_in_monitor(selected_monitor);

    for (int i = 0; i < workspaces->length; i++) {
        struct workspace *ws = workspaces->items[i];
        struct layout *dest_lt = ws->layout;
        struct layout *dest_prev_lt = ws->previous_layout;

        if (dest_lt == src_lt)
            continue;

        copy_layout(dest_lt, src_lt);
        copy_layout(dest_prev_lt, src_lt);
    }
}

void load_default_layout(lua_State *L, struct workspace *ws)
{
    load_layout(L, server.default_layout->name);
}

void set_container_workspace(struct container *con, struct workspace *ws)
{
    if (!con)
        return;
    if (!ws)
        return;
    if (con->m->ws_id == ws->id)
        return;

    struct workspace *sel_ws = monitor_get_active_workspace(con->m);

    if (!ws->m) {
        ws->m = con->m;
    } else {
        set_container_monitor(con, ws->m);
    }
    con->client->ws_id = ws->id;

    remove_in_composed_list(&sel_ws->container_lists, cmp_ptr, con);
    add_container_to_containers(con, ws, 0);

    remove_in_composed_list(&sel_ws->focus_stack_lists, cmp_ptr, con);
    add_container_to_focus_stack(con, ws);

    if (con->floating)
        con->client->bw = ws->layout->options.float_border_px;
    else
        con->client->bw = ws->layout->options.tile_border_px;
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

void load_layout(lua_State *L, const char *name)
{
    char *config_path = get_config_file("layouts");
    char file[NUM_CHARS] = "";
    strcpy(file, "");
    join_path(file, config_path);
    join_path(file, name);
    join_path(file, "init.lua");
    if (config_path)
        free(config_path);

    if (!file_exists(file))
        return;

    if (luaL_loadfile(L, file)) {
        lua_pop(L, 1);
        return;
    }
    lua_call_safe(L, 0, 0, 0);
}

void reset_loaded_layout(struct workspace *ws)
{
    int length = ws->loaded_layouts.length;
    for (int i = 0; i < length; i++) {
        struct layout *lt = ws->loaded_layouts.items[0];
        destroy_layout(lt);
        wlr_list_del(&ws->loaded_layouts, 0);
    }
}

void remove_loaded_layouts(struct wlr_list *workspaces)
{
    for (int i = 0; i < workspaces->length; i++) {
        struct workspace *ws = workspaces->items[i];
        wlr_list_clear(&ws->loaded_layouts, (void (*)(void *))destroy_layout);
    }
}

void push_workspace(struct monitor *m, struct workspace *ws)
{
    if (!m)
        return;
    if (!ws)
        return;

    if (m->ws_id == ws->id)
        return;

    if (m->ws_id != server.previous_workspace_id)
        server.previous_workspace_id = m->ws_id;

    focus_workspace(m, ws);
}

void push_layout(struct workspace *ws, struct layout *lt)
{
    lt->ws_id = ws->id;
    ws->previous_layout = ws->layout;
    ws->layout = lt;
}
