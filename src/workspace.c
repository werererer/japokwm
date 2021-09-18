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
#include "stringop.h"

static void update_workspaces_id(GPtrArray *workspaces)
{
    for (int id = 0; id < workspaces->len; id++) {
        struct workspace *ws = g_ptr_array_index(workspaces, id);
        ws->id = id;
    }
}

static struct focus_set *focus_set_create()
{
    struct focus_set *focus_set = calloc(1, sizeof(struct focus_set));
    focus_set->focus_stack_lists = g_ptr_array_new();
    focus_set->focus_stack_visible_lists = g_ptr_array_new();
    focus_set->focus_stack_lists_with_layer_shell = g_ptr_array_new();

    focus_set->focus_stack_layer_background = g_ptr_array_new();
    focus_set->focus_stack_layer_bottom = g_ptr_array_new();
    focus_set->focus_stack_layer_top = g_ptr_array_new();
    focus_set->focus_stack_layer_overlay = g_ptr_array_new();
    focus_set->focus_stack_on_top = g_ptr_array_new();
    focus_set->focus_stack_normal = g_ptr_array_new();
    focus_set->focus_stack_hidden = g_ptr_array_new();
    focus_set->focus_stack_not_focusable = g_ptr_array_new();

    g_ptr_array_add(focus_set->focus_stack_lists, focus_set->focus_stack_layer_top);
    g_ptr_array_add(focus_set->focus_stack_lists, focus_set->focus_stack_on_top);
    g_ptr_array_add(focus_set->focus_stack_lists, focus_set->focus_stack_normal);
    g_ptr_array_add(focus_set->focus_stack_lists, focus_set->focus_stack_not_focusable);
    g_ptr_array_add(focus_set->focus_stack_lists, focus_set->focus_stack_hidden);

    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_layer_overlay);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_layer_top);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_on_top);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_normal);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_not_focusable);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_layer_bottom);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_layer_background);

    g_ptr_array_add(focus_set->focus_stack_visible_lists, focus_set->focus_stack_on_top);
    g_ptr_array_add(focus_set->focus_stack_visible_lists, focus_set->focus_stack_normal);
    g_ptr_array_add(focus_set->focus_stack_visible_lists, focus_set->focus_stack_not_focusable);

    return focus_set;
}

static void focus_set_destroy(struct focus_set *focus_set)
{
    g_ptr_array_free(focus_set->focus_stack_layer_background, FALSE);
    g_ptr_array_free(focus_set->focus_stack_layer_bottom, FALSE);
    g_ptr_array_free(focus_set->focus_stack_layer_top, FALSE);
    g_ptr_array_free(focus_set->focus_stack_layer_overlay, FALSE);
    g_ptr_array_free(focus_set->focus_stack_lists, FALSE);
    g_ptr_array_free(focus_set->focus_stack_visible_lists, FALSE);
    g_ptr_array_free(focus_set->focus_stack_lists_with_layer_shell, FALSE);

    g_ptr_array_free(focus_set->focus_stack_on_top, FALSE);
    g_ptr_array_free(focus_set->focus_stack_normal, FALSE);
    g_ptr_array_free(focus_set->focus_stack_hidden, FALSE);
    g_ptr_array_free(focus_set->focus_stack_not_focusable, FALSE);
}

static struct visual_set *visual_set_create()
{
    struct visual_set *visual_set = calloc(1, sizeof(struct visual_set));
    visual_set->all_stack_lists = g_ptr_array_new();
    visual_set->stack_lists = g_ptr_array_new();
    visual_set->visual_stack_lists = g_ptr_array_new();

    visual_set->tiled_visual_stack = g_ptr_array_new();
    visual_set->floating_visual_stack = g_ptr_array_new();

    g_ptr_array_add(visual_set->all_stack_lists, server.layer_visual_stack_overlay);
    g_ptr_array_add(visual_set->all_stack_lists, server.layer_visual_stack_top);
    g_ptr_array_add(visual_set->all_stack_lists, server.floating_stack);
    g_ptr_array_add(visual_set->all_stack_lists, visual_set->tiled_visual_stack);
    g_ptr_array_add(visual_set->all_stack_lists, server.layer_visual_stack_bottom);
    g_ptr_array_add(visual_set->all_stack_lists, server.layer_visual_stack_background);

    g_ptr_array_add(visual_set->stack_lists, server.layer_visual_stack_overlay);
    g_ptr_array_add(visual_set->stack_lists, server.layer_visual_stack_top);
    g_ptr_array_add(visual_set->stack_lists, visual_set->floating_visual_stack);
    g_ptr_array_add(visual_set->stack_lists, visual_set->tiled_visual_stack);
    g_ptr_array_add(visual_set->stack_lists, server.layer_visual_stack_bottom);
    g_ptr_array_add(visual_set->stack_lists, server.layer_visual_stack_background);

    g_ptr_array_add(visual_set->visual_stack_lists, visual_set->floating_visual_stack);
    g_ptr_array_add(visual_set->visual_stack_lists, visual_set->tiled_visual_stack);
    return visual_set;
}

static void visual_set_destroy(struct visual_set *visual_set)
{
    // TODO fix me
    /* visual_set->visual_stack_lists = g_ptr_array_new(); */
    /* visual_set->normal_visual_stack_lists = g_ptr_array_new(); */
    /* visual_set->layer_visual_stack_lists = g_ptr_array_new(); */

    /* visual_set->tiled_visual_stack = g_ptr_array_new(); */
    /* visual_set->floating_visual_stack = g_ptr_array_new(); */
    /* visual_set->layer_visual_stack_background = g_ptr_array_new(); */
    /* visual_set->layer_visual_stack_bottom = g_ptr_array_new(); */
    /* visual_set->layer_visual_stack_top = g_ptr_array_new(); */
    /* visual_set->layer_visual_stack_overlay = g_ptr_array_new(); */
    free(visual_set);
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

    ws->prev_workspaces = bitset_create(server.workspaces->len);
    ws->independent_containers = g_ptr_array_new();

    ws->focus_set = focus_set_create();
    ws->visible_focus_set = focus_set_create();
    ws->local_focus_set = focus_set_create();
    ws->visual_set = visual_set_create();
    ws->visible_visual_set = visual_set_create();

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
        workspace_rename(ws, name);
    }
}

void focus_most_recent_container(struct workspace *ws)
{
    struct container *con = get_in_composed_list(ws->visible_focus_set->focus_stack_lists, 0);

    // FIXME
    if (!con) {
        con = get_container(ws, 0);
        if (!con) {
            workspace_update_names(&server, server.workspaces);
            ipc_event_window();
            return;
        }
    }

    focus_container(con);
}

struct container *get_container(struct workspace *ws, int i)
{
    return get_in_composed_list(ws->visible_focus_set->focus_stack_visible_lists, i);
}

void destroy_workspace(struct workspace *ws)
{
    g_ptr_array_free(ws->independent_containers, false);

    bitset_destroy(ws->prev_workspaces);
    focus_set_destroy(ws->focus_set);
    focus_set_destroy(ws->visible_focus_set);
    focus_set_destroy(ws->local_focus_set);
    visual_set_destroy(ws->visual_set);
    visual_set_destroy(ws->visible_visual_set);
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

void update_sub_focus_stack(struct workspace *ws)
{
    update_reduced_focus_stack(ws);
    update_local_focus_stack(ws);
}

void update_reduced_focus_stack(struct workspace *ws)
{
    for (int i = 0; i < ws->focus_set->focus_stack_lists->len; i++) {
        GPtrArray *src_list = g_ptr_array_index(ws->focus_set->focus_stack_lists, i);
        GPtrArray *dest_list = g_ptr_array_index(ws->visible_focus_set->focus_stack_lists, i);
        list_clear(dest_list, NULL);
        for (int j = 0; j < src_list->len; j++) {
            struct container *con = g_ptr_array_index(src_list, j);
            struct monitor *m = workspace_get_monitor(ws);
            bool viewable = container_viewable_on_monitor(m, con);
            bool visible = visible_on(m, ws->prev_workspaces, ws->id, con);
            if (viewable || visible) {
                g_ptr_array_add(dest_list, con);
            }
        }
    }
}

void update_local_focus_stack(struct workspace *ws)
{
    for (int i = 0; i < ws->focus_set->focus_stack_lists->len; i++) {
        GPtrArray *src_list = g_ptr_array_index(ws->focus_set->focus_stack_lists, i);
        GPtrArray *dest_list = g_ptr_array_index(ws->local_focus_set->focus_stack_lists, i);
        list_clear(dest_list, NULL);
        for (int j = 0; j < src_list->len; j++) {
            struct container *con = g_ptr_array_index(src_list, j);
            struct monitor *m = workspace_get_monitor(ws);
            if (exist_on(m, ws->prev_workspaces, ws->id, con)) {
                g_ptr_array_add(dest_list, con);
            }
        }
    }
}

void update_visual_visible_stack(struct workspace *ws)
{
    for (int i = 0; i < ws->visual_set->visual_stack_lists->len; i++) {
        GPtrArray *src_list = g_ptr_array_index(ws->visual_set->visual_stack_lists, i);
        GPtrArray *dest_list = g_ptr_array_index(ws->visible_visual_set->visual_stack_lists, i);
        list_clear(dest_list, NULL);
        for (int i = 0; i < src_list->len; i++) {
            struct container *con = g_ptr_array_index(src_list, i);
            struct monitor *m = workspace_get_monitor(ws);
            if (container_potentially_viewable_on_monitor(m, con)) {
                g_ptr_array_add(dest_list, con);
            }
        }
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
    return bitset_test(tagset->loaded_workspaces, ws->id);
}

int get_workspace_container_count(struct workspace *ws)
{
    if (!ws)
        return -1;

    int count = 0;
    for (int i = 0; i < length_of_composed_list(ws->list_set->visible_container_lists); i++) {
        struct container *con = get_in_composed_list(ws->list_set->visible_container_lists, i);
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

struct tagset *workspace_get_active_tagset(struct workspace *ws)
{
    struct tagset *tagset = ws->tagset;
    if (!tagset) {
        tagset = ws->selected_tagset;
        if (!tagset)
            return NULL;
    }
    return tagset;
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
    load_layout(L, server.default_layout->symbol);
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
    push_tagset(tagset);
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
    if (!lt->options.automatic_workspace_naming)
        return;
    for (int i = 0; i < workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(workspaces, i);
        const char *default_name;
        if (i < lt->options.tag_names->len) {
            default_name = g_ptr_array_index(lt->options.tag_names, i);
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
    struct container *con = get_in_composed_list(ws->visible_focus_set->focus_stack_visible_lists, 0);
    return con;
}

void list_set_add_container_to_containers(struct container_set *list_set, struct container *con, int i)
{
    if (container_is_floating(con)) {
        g_ptr_array_insert(list_set->floating_containers, i, con);
        return;
    }
    if (con->hidden) {
        g_ptr_array_insert(list_set->hidden_containers, i, con);
        return;
    }
    assert(list_set->tiled_containers->len >= i);
    if (list_set->tiled_containers->len <= 0) {
        g_ptr_array_add(list_set->tiled_containers, con);
    } else {
        g_ptr_array_insert(list_set->tiled_containers, i, con);
    }
}

void workspace_add_container_to_containers(struct workspace *ws, struct container *con, int i)
{
    assert(con != NULL);

    DO_ACTION_GLOBALLY(server.workspaces,
        list_set_add_container_to_containers(list_set, con, i);
    );
}

void workspace_remove_container_from_containers_locally(struct workspace *ws, struct container *con)
{
    struct tagset *tagset = workspace_get_active_tagset(ws);
    struct layout *lt = tagset_get_layout(tagset);
    if (lt->options.arrange_by_focus) {
        remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
        remove_in_composed_list(ws->visible_focus_set->focus_stack_lists, cmp_ptr, con);
        remove_in_composed_list(ws->local_focus_set->focus_stack_lists, cmp_ptr, con);
    } else {
        DO_ACTION_LOCALLY(ws,
            remove_in_composed_list(list_set->container_lists, cmp_ptr, con);
        );
    }
}

void workspace_add_container_to_containers_locally(struct workspace *ws, struct container *con, int i)
{
    struct tagset *tagset = workspace_get_active_tagset(ws);
    struct layout *lt = tagset_get_layout(tagset);
    if (lt->options.arrange_by_focus) {
        list_set_add_container_to_focus_stack(ws, con);
        update_sub_focus_stack(ws);
    } else {
        DO_ACTION_LOCALLY(ws,
            list_set_add_container_to_containers(list_set, con, i);
        );
    }
}

void list_set_append_container_to_focus_stack(struct workspace *ws, struct container *con)
{
    if (con->client->type == LAYER_SHELL) {
        switch (con->client->surface.layer->current.layer) {
            case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
                g_ptr_array_add(ws->focus_set->focus_stack_layer_background, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
                g_ptr_array_add(ws->focus_set->focus_stack_layer_bottom, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
                g_ptr_array_add(ws->focus_set->focus_stack_layer_top, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
                g_ptr_array_add(ws->focus_set->focus_stack_layer_overlay, con);
                break;
        }
        return;
    }
    if (con->hidden) {
        g_ptr_array_add(ws->focus_set->focus_stack_hidden, con);
        return;
    }
    if (con->on_top) {
        g_ptr_array_add(ws->focus_set->focus_stack_on_top, con);
        return;
    }
    if (!con->focusable) {
        g_ptr_array_add(ws->focus_set->focus_stack_not_focusable, con);
        return;
    }

    g_ptr_array_add(ws->focus_set->focus_stack_normal, con);
}

void list_set_add_container_to_focus_stack(struct workspace *ws, struct container *con)
{
    if (con->client->type == LAYER_SHELL) {
        switch (con->client->surface.layer->current.layer) {
            case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
                g_ptr_array_insert(ws->focus_set->focus_stack_layer_background, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
                g_ptr_array_insert(ws->focus_set->focus_stack_layer_bottom, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
                g_ptr_array_insert(ws->focus_set->focus_stack_layer_top, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
                g_ptr_array_insert(ws->focus_set->focus_stack_layer_overlay, 0, con);
                break;
        }
        return;
    }
    if (con->hidden) {
        g_ptr_array_insert(ws->focus_set->focus_stack_hidden, 0, con);
        return;
    }
    if (con->on_top) {
        g_ptr_array_insert(ws->focus_set->focus_stack_on_top, 0, con);
        return;
    }
    if (!con->focusable) {
        g_ptr_array_insert(ws->focus_set->focus_stack_not_focusable, 0, con);
        return;
    }

    g_ptr_array_insert(ws->focus_set->focus_stack_normal, 0, con);
}

void workspace_add_container_to_focus_stack(struct workspace *ws, struct container *con)
{
    debug_print("add container to focus_stack\n");
    // TODO: refactor me
    for (int i = 0; i < server.workspaces->len; i++) {
        ws = g_ptr_array_index(server.workspaces, i);
        list_set_add_container_to_focus_stack(ws, con);
        update_sub_focus_stack(ws);
    }
}

void workspace_remove_container_from_focus_stack_locally(struct workspace *ws, struct container *con)
{
    remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
    update_sub_focus_stack(ws);
}

void workspace_add_container_to_focus_stack_locally(struct workspace *ws, struct container *con)
{
    list_set_add_container_to_focus_stack(ws, con);
    update_sub_focus_stack(ws);
}

void workspace_remove_container_from_floating_stack_locally(struct workspace *ws, struct container *con)
{
    DO_ACTION_LOCALLY(ws, 
            g_ptr_array_remove(list_set->floating_containers, con);
            );
}

void workspace_add_container_to_floating_stack_locally(struct workspace *ws, struct container *con, int i)
{
    DO_ACTION_LOCALLY(ws, 
            g_ptr_array_insert(list_set->floating_containers, i, con);
            );
}

void workspace_remove_container_from_visual_stack_layer(struct workspace *ws, struct container *con)
{
    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = get_workspace(i);
        remove_in_composed_list(server.layer_visual_stack_lists, cmp_ptr, con);
        update_visual_visible_stack(ws);
    }
}

void workspace_remove_container_from_visual_stack_normal(struct workspace *ws, struct container *con)
{
    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = get_workspace(i);
        remove_in_composed_list(ws->visual_set->visual_stack_lists, cmp_ptr, con);
        update_visual_visible_stack(ws);
    }
}

void workspace_add_container_to_visual_stack_layer(struct workspace *ws, struct container *con)
{
    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = get_workspace(i);
        add_container_to_stack(ws, con);
    }
}

void workspace_add_container_to_visual_stack_normal(struct workspace *ws, struct container *con)
{
    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = get_workspace(i);
        add_container_to_stack(ws, con);
    }
}

void add_container_to_stack(struct workspace *ws, struct container *con)
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
        update_visual_visible_stack(ws);
        return;
    }

    if (container_is_floating(con)) {
        g_ptr_array_insert(ws->visual_set->floating_visual_stack, 0, con);
        update_visual_visible_stack(ws);
        return;
    }

    g_ptr_array_insert(ws->visual_set->tiled_visual_stack, 0, con);
    update_visual_visible_stack(ws);
}

void workspace_remove_container(struct workspace *ws, struct container *con)
{
    DO_ACTION_GLOBALLY(server.workspaces,
            remove_in_composed_list(list_set->container_lists, cmp_ptr, con);
            );
}

void workspace_remove_container_from_focus_stack(struct workspace *ws, struct container *con)
{
    for (int i = 0; i < server.workspaces->len; i++) {
        ws = g_ptr_array_index(server.workspaces, i);
        remove_in_composed_list(ws->focus_set->focus_stack_lists, cmp_ptr, con);
        remove_in_composed_list(ws->visible_focus_set->focus_stack_lists, cmp_ptr, con);
        remove_in_composed_list(ws->local_focus_set->focus_stack_lists, cmp_ptr, con);
    }
}

void workspace_remove_independent_container(struct workspace *ws, struct container *con)
{
    g_ptr_array_remove(ws->independent_containers, con);
}

static int get_in_container_stack(struct container *con)
{
    if (!con)
        return INVALID_POSITION;

    struct monitor *m = selected_monitor;
    struct workspace *ws = monitor_get_active_workspace(m);
    int position = find_in_composed_list(ws->list_set->visible_container_lists, cmp_ptr, con);
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

    GPtrArray *tiled_list = tagset_get_tiled_list(tagset);

    g_ptr_array_remove(tiled_list, con);
    g_ptr_array_insert(tiled_list, new_pos, con);
    tagset_write_to_workspaces(tagset);
}

bool workspace_sticky_contains_client(struct workspace *ws, struct client *client)
{
    return bitset_test(client->sticky_workspaces, ws->id);
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
