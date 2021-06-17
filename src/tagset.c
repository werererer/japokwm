#include "tagset.h"
#include "bitset/bitset.h"
#include "list_set.h"
#include "server.h"
#include "workspace.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "tile/tileUtils.h"
#include "ipc-server.h"

struct tagset *create_tagset(struct monitor *m, struct layout *lt, int selected_ws_id, BitSet workspaces)
{
    struct tagset *tagset = calloc(1, sizeof(struct tagset));
    tagset->m = m;
    tagset->selected_ws_id = selected_ws_id;

    wlr_list_init(&tagset->loaded_layouts);
    setup_list_set(&tagset->list_set);

    bitset_setup(&tagset->workspaces, server.workspaces.length);
    tagset_set_tags(tagset, workspaces);

    // fill layout stack with reasonable values
    push_layout(tagset, lt);
    push_layout(tagset, lt);
    wlr_list_push(&server.tagsets, tagset);
    return tagset;
}

void destroy_tagset(struct tagset *ts)
{
    wlr_list_remove(&server.tagsets, cmp_ptr, ts);
    free(ts);
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

    for (int i = 0; i < tagset->workspaces.size; i++) {
        bool bit = bitset_test(&tagset->workspaces, i);
        if (bit) {
            struct workspace *ws = get_workspace(i);
            ws->m = selected_monitor;
        }
    }

    ipc_event_workspace();

    /* struct tagset *old_tagset = monitor_get_active_tagset(m); */
    /* struct container *con; */
    /* wl_list_for_each(con, &sticky_stack, stlink) { */
    /*     con->client->ws_id = ws->id; */
    /* } */

    if (m->tagset) {
        destroy_tagset(m->tagset);
    }
    m->tagset = tagset;

    arrange();
    focus_most_recent_container(m->tagset, FOCUS_NOOP);
    root_damage_whole(m->root);
}

void tagset_set_tags(struct tagset *tagset, BitSet bitset)
{
    if (!tagset)
        return;

    printf("tagset set tags\n");
    clear_list_set(&tagset->list_set);

    for (int i = 0; i < tagset->workspaces.size; i++) {
        bool bit = bitset_test(&tagset->workspaces, i);

        printf("bit1: %i\n", bit);
        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        wlr_list_remove(&ws->list_set.change_affected_list_sets, cmp_ptr, &tagset->list_set);
    }

    bitset_move(&tagset->workspaces, &bitset);
    for(size_t i = 0; i < tagset->workspaces.size; i++) {
        bool bit = bitset_test(&tagset->workspaces, i);

        printf("bit2: %i\n", bit);
        if (!bit)
            continue;

        struct workspace *ws = get_workspace(i);
        append_list_set(&tagset->list_set, &ws->list_set);
        ws->m = tagset->m;
    }
    ipc_event_workspace();
    printf("tagset set tags end\n");
}

void push_tagset(struct tagset *tagset)
{
    printf("push_tagset\n");
    if (!tagset)
        return;

    struct monitor *m = selected_monitor;

    if (m->tagset == tagset)
        return;

    if (m->tagset != server.previous_tagset)
        server.previous_tagset = m->tagset;

    focus_tagset(tagset);
}

void tagset_set_workspace_id(struct tagset *tagset, int ws_id)
{
    BitSet bitset;
    bitset_setup(&bitset, server.workspaces.length);
    tagset_set_tags(tagset, bitset);
}

void tagset_toggle_add(struct tagset *tagset, BitSet bitset)
{
    if (!tagset)
        return;

    bitset_xor(&bitset, &tagset->workspaces);

    tagset_set_tags(tagset, bitset);
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

struct wlr_list *tagset_get_visible_lists(struct tagset *ts)
{
    struct layout *lt = ts->layout;

    if (lt->options.arrange_by_focus)
        return &ts->list_set.focus_stack_visible_lists;
    else
        return &ts->list_set.visible_container_lists;
}

struct wlr_list *tagset_get_tiled_list(struct tagset *ts)
{
    struct layout *lt = ts->layout;

    if (lt->options.arrange_by_focus)
        return &ts->list_set.focus_stack_normal;
    else
        return &ts->list_set.tiled_containers;
}

struct wlr_list *tagset_get_floating_list(struct tagset *ts)
{
    struct layout *lt = ts->layout;

    if (lt->options.arrange_by_focus)
        return &ts->list_set.focus_stack_normal;
    else
        return &ts->list_set.floating_containers;
}

struct wlr_list *tagset_get_hidden_list(struct tagset *ts)
{
    struct layout *lt = ts->layout;

    if (lt->options.arrange_by_focus)
        return &ts->list_set.focus_stack_hidden;
    else
        return &ts->list_set.hidden_containers;
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
        printf("monitor unequal\n");
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

void push_layout(struct tagset *ts, struct layout *lt)
{
    lt->ws_id = ts->selected_ws_id;
    ts->previous_layout = ts->layout;
    ts->layout = lt;
}

void load_default_layout(lua_State *L)
{
    load_layout(L, server.default_layout->name);
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



void reset_loaded_layout(struct tagset *ts)
{
    int length = ts->loaded_layouts.length;
    for (int i = 0; i < length; i++) {
        struct layout *lt = ts->loaded_layouts.items[0];
        destroy_layout(lt);
        wlr_list_del(&ts->loaded_layouts, 0);
    }
}

void remove_loaded_layouts(struct wlr_list *workspaces)
{
    for (int i = 0; i < workspaces->length; i++) {
        struct tagset *ts = get_tagset_from_workspace_id(workspaces, i);
        wlr_list_clear(&ts->loaded_layouts, (void (*)(void *))destroy_layout);
    }
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
