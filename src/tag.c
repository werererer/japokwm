#include "tag.h"

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
#include "layer_shell.h"

static void update_tags_id(GPtrArray *tags)
{
    for (int id = 0; id < tags->len; id++) {
        struct tag *tag = g_ptr_array_index(tags, id);
        tag->id = id;
    }
}

static void _destroy_tag(void *ws)
{
    destroy_tag(ws);
}

void load_tags(GList *tags, GPtrArray *tag_names)
{
    for (int i = 0; i < tag_names->len; i++) {
        struct tag *tag = get_tag(i);
        const char *name = g_ptr_array_index(tag_names, i);
        tag->current_layout = server.default_layout->name;
        tag->previous_layout = server.default_layout->name;
        tag_remove_loaded_layouts(tag);
        tag_rename(tag, name);
    }
}

void destroy_tags(GHashTable *tags)
{
    g_hash_table_unref(tags);
}

GHashTable *create_tags()
{
    // GHashTable *tags = g_ptr_array_new_with_free_func(_destroy_tag);
    GHashTable *tags = g_hash_table_new(g_int_hash, g_int_equal);
    return tags;
}

struct tag *create_tag(const char *name, size_t id, struct layout *lt)
{
    struct tag *tag = calloc(1, sizeof(struct tag));
    tag->name = strdup(name);
    tag->id = id;

    tag->loaded_layouts = g_ptr_array_new();
    tag->current_layout = lt->name;
    tag->previous_layout = lt->name;

    tag->tags = bitset_create();
    tag->prev_tags = bitset_create();
    tag->tags->data = tag;

    bitset_set(tag->tags, tag->id);
    bitset_set(tag->prev_tags, tag->id);

    tag->con_set = create_container_set();
    tag->visible_con_set = create_container_set();

    tag->focus_set = focus_set_create();
    tag->visible_focus_set = focus_set_create();

    tag->visible_bar_edges = WLR_EDGE_BOTTOM
        | WLR_EDGE_TOP
        | WLR_EDGE_LEFT
        | WLR_EDGE_RIGHT;
    return tag;
}

void update_tags(GList *tags, GPtrArray *tag_names)
{
    for (GList *iterator = tags; iterator; iterator = iterator->next) {
        struct tag *tag = iterator->data;
        if (tag->id > tag_names->len)
            continue;
        const char *new_name = g_ptr_array_index(tag_names, tag->id);
        tag_rename(tag, new_name);
    }
}

bool tag_has_no_visible_containers(struct tag *tag)
{
    return tag->visible_con_set->tiled_containers->len <= 0;
}

void tag_focus_first_container(struct tag *tag)
{
    if (tag_has_no_visible_containers(tag))
        return;

    struct container *con = g_ptr_array_index(tag->visible_con_set->tiled_containers, 0);
    if (!con)
        return;

    tag_focus_container(tag, con);
}

void tag_this_focus_most_recent_container()
{
    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    tag_focus_most_recent_container(tag);
}

void tag_focus_most_recent_container(struct tag *tag)
{
    if (!tag)
        return;
    struct container *con = tag_get_focused_container(tag);
    if (!con) {
        tag_focus_first_container(tag);
        return;
    }

    tag_focus_container(tag, con);
}

struct container *get_container(struct tag *tag, int i)
{
    return get_in_composed_list(tag->visible_focus_set->focus_stack_visible_lists, i);
}

struct container *get_container_in_stack(struct tag *tag, int i)
{
    GPtrArray *tiled_containers = tag_get_tiled_list_copy(tag);
    struct container *con = g_ptr_array_index(tiled_containers, i);
    return con;
}

void destroy_tag(struct tag *tag)
{
    for (int i = 0; i < tag->loaded_layouts->len; i++) {
        struct layout *lt = g_ptr_array_steal_index(tag->loaded_layouts, 0);
        destroy_layout(lt);
    }

    tag_remove_loaded_layouts(tag);
    g_ptr_array_unref(tag->loaded_layouts);

    bitset_destroy(tag->tags);
    bitset_destroy(tag->prev_tags);
    focus_set_destroy(tag->focus_set);
    focus_set_destroy(tag->visible_focus_set);

    for (int i = 0; i < tag->con_set->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(tag->con_set->tiled_containers, i);
        if (con->tag_id != tag->id)
            continue;

        struct client *c = con->client;
        kill_client(c);
    }
    destroy_container_set(tag->con_set);
    destroy_container_set(tag->visible_con_set);
    free(tag->name);
    free(tag);
}

void update_tag_ids(GPtrArray *tags)
{
    for (int i = 0; i < tags->len; i++) {
        struct tag *tag = g_ptr_array_index(tags, i);
        tag->id = i;
    }
}

bool is_tag_occupied(struct tag *tag)
{
    assert(tag);

    struct monitor *m = tag_get_monitor(tag);
    bool is_occupied = (m != NULL) && tag_is_visible(tag, m);
    return is_occupied;
}

bool tag_is_visible(struct tag *tag, struct monitor *m)
{
    assert(tag != NULL);

    struct tag *sel_tag = monitor_get_active_tag(m);
    if (!sel_tag) {
        return false;
    }
    if (tag == sel_tag) {
        return true;
    }
    if (bitset_test(sel_tag->tags, tag->id)) {
        return true;
    }
    if (tag->current_m && !is_tag_empty(tag)) {
        return true;
    }
    return false;
}

bool is_tag_extern(struct tag *tag)
{
    struct monitor *tag_m = tag_get_selected_monitor(tag);
    if (!tag_m) {
        return false;
    }

    struct monitor *sel_m = server_get_selected_monitor();
    struct tag *sel_tag = monitor_get_active_tag(sel_m);
    if (bitset_test(sel_tag->tags, tag->id) && sel_m != tag_m) {
        return true;
    }
    return false;
}

bool is_tag_the_selected_one(struct tag *tag)
{
    struct monitor *m = tag_get_monitor(tag);
    struct tag *sel_tag = monitor_get_active_tag(m);
    return sel_tag->id == tag->id && !is_tag_extern(tag);
}

bool tag_is_active(struct tag *tag)
{
    struct monitor *m = tag_get_monitor(tag);
    if (!m)
        return false;
    struct tag *sel_tag = monitor_get_active_tag(m);

    return bitset_test(sel_tag->tags, tag->id);
}

int get_tag_container_count(struct tag *tag)
{
    if (!tag)
        return -1;

    int count = 0;
    for (int i = 0; i < tag->con_set->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(tag->con_set->tiled_containers, i);
        if (con->tag_id == tag->id) {
            count++;
        }
    }
    return count;
}

bool is_tag_empty(struct tag *tag)
{
    return get_tag_container_count(tag) == 0;
}

struct tag *find_next_unoccupied_tag(GList *tags, struct tag *tag)
{
    // we have infinity amount of tags so just get the next one
    // (infinity - 1 = infinity ;) )
    size_t start_id = tag ? tag->id : 0;
    for (size_t i = start_id ;; i++) {
        struct tag *tag = get_tag(i);
        if (!is_tag_occupied(tag))
            return tag;
    }
}

struct tag *get_next_empty_tag(GList *tags, size_t tag_id)
{
    for (size_t i = tag_id; i < 8; i++) {
        struct tag *tag = get_tag(i);
        if (is_tag_empty(tag)) {
            return tag;
        }
    }
    for (size_t i = tag_id-1; i >= 0; i--) {
        struct tag *tag = get_tag(i);
        if (is_tag_empty(tag)) {
            return tag;
        }
    }
    return NULL;
}

struct tag *get_nearest_empty_tag(GList *tags, int tag_id)
{
    struct tag *initial_tag = get_tag(tag_id);
    if (is_tag_empty(initial_tag)) {
        return initial_tag;
    }

    int tag_count = server_get_tag_count();
    struct tag *tag = NULL;
    for (int i = 0, up_counter = tag_id+1, down_counter = tag_id-1;
            i < tag_count;
            i++,up_counter++,down_counter--) {

        bool is_up_counter_valid = up_counter < tag_count;
        bool is_down_counter_valid = down_counter >= 0;
        if (is_down_counter_valid) {
            tag = get_tag(down_counter);
            if (is_tag_empty(tag))
                break;
        }
        if (is_up_counter_valid) {
            tag = get_tag(up_counter);
            if (is_tag_empty(tag))
                break;
        }
        if (!is_up_counter_valid && !is_down_counter_valid) {
            break;
        }
    }

    return tag;
}


struct tag *get_prev_empty_tag(GList *tags, size_t tag_id)
{
    for (size_t i = tag_id-1; i >= 0; i--) {
        struct tag *tag = get_tag(i);
        if (is_tag_empty(tag)) {
            return tag;
        }
    }
    for (size_t i = tag_id; i < 8; i++) {
        struct tag *tag = get_tag(i);
        if (is_tag_empty(tag)) {
            return tag;
        }
    }
    return NULL;
}

struct layout *tag_get_layout(struct tag *tag)
{
    if (!tag)
        return NULL;
    if (tag->loaded_layouts->len == 0) {
        focus_layout(tag, tag->current_layout);
        assert(tag->loaded_layouts->len > 0);
    }
    return g_ptr_array_index(tag->loaded_layouts, 0);
}

struct layout *tag_get_previous_layout(struct tag *tag)
{
    if (!tag)
        return NULL;
    while (tag->loaded_layouts->len < 1) {
        tag_load_layout(tag, tag->previous_layout);
        assert(tag->loaded_layouts->len > 0);
    }
    return g_ptr_array_index(tag->loaded_layouts, 1);
}

struct root *tag_get_root(struct tag *tag)
{
    struct monitor *m = tag_get_monitor(tag);
    struct root *root = monitor_get_active_root(m);
    return root;
}

struct wlr_box tag_get_active_geom(struct tag *tag)
{
    struct wlr_box geom;
    struct root *root = tag_get_root(tag);
    geom = root->geom;
    return geom;
}

struct monitor *tag_get_selected_monitor(struct tag *tag)
{
    assert(tag != NULL);
    return tag->m;
}

struct monitor *tag_get_monitor(struct tag *tag)
{
    assert(tag != NULL);

    if (tag->current_m) {
        return tag->current_m;
    }
    if (tag->m) {
        return tag->m;
    }
    return NULL;
}

void tag_set_selected_monitor(struct tag *tag, struct monitor *m)
{
    if (!tag)
        return;
    tag->m = m;
}

void tag_set_current_monitor(struct tag *tag, struct monitor *m)
{
    tag->current_m = m;
}

// swap everything but the bits at ws*->id
static void tag_swap_supplementary_tags(
        struct tag *tag,
        struct tag *tag2)
{
    bool prev_tag1_value = bitset_test(tag->tags, tag->id);
    bool prev_tag1_tag2_value = bitset_test(tag->tags, tag2->id);
    bool prev_tag2_value = bitset_test(tag2->tags, tag2->id);
    bool prev_tag2_tag1_value = bitset_test(tag2->tags, tag->id);
    bitset_swap(tag->tags, tag2->tags);
    bitset_assign(tag->tags, tag->id, prev_tag1_value);
    bitset_assign(tag->tags, tag2->id, prev_tag2_tag1_value);
    bitset_assign(tag2->tags, tag2->id, prev_tag2_value);
    bitset_assign(tag2->tags, tag->id, prev_tag1_tag2_value);
}
/**
 * A helper function to make swapping tags more natural. This is just my
 * preference. So if you don't like it just use the dumb version of swap
 * tags. And define your own helper function in lua.
 */
static void tag_swap_tags_smart(struct tag *tag1, struct tag *tag2)
{
    monitor_set_selected_tag(server_get_selected_monitor(), tag1);
    for (GList *iterator = server_get_tags(); iterator; iterator = iterator->next) {
        struct tag *tag = iterator->data;

        if (tag->id == tag1->id || tag->id == tag2->id) {
            continue;
        }
        bool b1 = bitset_test(tag->tags, tag1->id);
        bool b2 = bitset_test(tag->tags, tag2->id);

        bitset_assign(tag->tags, tag1->id, b2);
        bitset_assign(tag->tags, tag2->id, b1);
    }

    tag_swap_supplementary_tags(tag1, tag2);
}

void tag_swap(struct tag *tag1, struct tag *tag2)
{
    GPtrArray *future_tag2_containers = g_ptr_array_new();
    for (int i = 0; i < tag1->con_set->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(tag1->con_set->tiled_containers, i);
        if (tag1->id != con->tag_id)
            continue;

        g_ptr_array_add(future_tag2_containers, con);
    }

    for (int i = 0; i < tag2->con_set->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(tag2->con_set->tiled_containers, i);
        if (tag2->id != con->tag_id)
            continue;
        con->tag_id = tag1->id;
        bitset_reset_all(con->client->sticky_tags);
        bitset_set(con->client->sticky_tags, con->tag_id);
    }

    for (int i = 0; i < future_tag2_containers->len; i++) {
        struct container *con = g_ptr_array_index(future_tag2_containers, i);
        con->tag_id = tag2->id;
        bitset_reset_all(con->client->sticky_tags);
        bitset_set(con->client->sticky_tags, con->tag_id);
    }
    g_ptr_array_unref(future_tag2_containers);
}

void tag_swap_smart(struct tag *tag1, struct tag *tag2)
{
    tag_swap(tag1, tag2);
    tag_swap_tags_smart(tag1, tag2);
}

BitSet *tag_get_tags(struct tag *tag)
{
    return tag->tags;
}

BitSet *tag_get_prev_tags(struct tag *tag)
{
    printf("set prev_tags:");
    print_bitset(tag->prev_tags);
    return tag->prev_tags;
}

void push_layout(struct tag *tag, const char *layout_name)
{
    tag->previous_layout = tag->current_layout;
    tag->current_layout = layout_name;
}

void set_default_layout(struct tag *tag)
{
    push_layout(tag, server.default_layout->name);
}

static void load_layout_file(lua_State *L, struct layout *lt)
{
    init_local_config_variables(L, lt);
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

static int _load_layout(struct tag *tag, const char *layout_name)
{
    struct layout *lt = create_layout(L);

    lt->tag_id = tag->id;
    copy_layout_safe(lt, server.default_layout);

    lt->name = strdup(layout_name);

    int insert_position = tag->loaded_layouts->len;
    g_ptr_array_add(tag->loaded_layouts, lt);

    load_layout_file(L, lt);
    return insert_position;
}

// returns position of the layout if it was found and -1 if not
int tag_load_layout(struct tag *tag, const char *layout_name)
{
    guint i;
    bool found = g_ptr_array_find_with_equal_func(tag->loaded_layouts, layout_name, cmp_layout_to_string, &i);
    if (found) {
        return i;
    }

    int insert_position = _load_layout(tag, layout_name);
    struct layout *lt = g_ptr_array_index(tag->loaded_layouts, insert_position);
    for (int i = 0; i < lt->linked_layouts->len; i++) {
        char *linked_layout_name = g_ptr_array_index(lt->linked_layouts, i);
        tag_load_layout(tag, linked_layout_name);
    }

    return insert_position;
}

void focus_layout(struct tag *tag, const char *name)
{
    assert(tag != NULL);
    assert(name != NULL);

    int i = tag_load_layout(tag, name);

    // if it is already at position 0 it is already focused. A value smaller
    // than 0 would be an error
    assert(i >= 0);
    if (i == 0)
        return;

    struct layout *lt = g_ptr_array_steal_index(tag->loaded_layouts, i);
    g_ptr_array_insert(tag->loaded_layouts, 0, lt);
}

static void destroy_layout0(void *lt)
{
    destroy_layout(lt);
}

void tag_remove_loaded_layouts(struct tag *tag)
{
    list_clear(tag->loaded_layouts, destroy_layout0);
}

void tags_remove_loaded_layouts(GList *tags)
{
    for (GList *iter = tags; iter; iter = iter->next) {
        struct tag *tag = iter->data;
        tag_remove_loaded_layouts(tag);
    }
}

void tag_rename(struct tag *tag, const char *name)
{
    if (!tag)
        return;
    free(tag->name);
    tag->name = strdup(name);
}

static struct container *tag_get_local_focused_container(struct tag *tag)
{
    if (!tag)
        return NULL;

    for (int i = 0; i < length_of_composed_list(tag->visible_focus_set->focus_stack_lists); i++) {
        struct container *con = get_in_composed_list(tag->visible_focus_set->focus_stack_lists, i);
        if (con->tag_id != tag->id)
            continue;
        return con;
    }
    return NULL;
}

void tag_this_focus_container(struct container *con)
{
    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    tag_focus_container(tag, con);
}

void tag_focus_container(struct tag *tag, struct container *con)
{
    if (!con)
        return;
    if (!con->focusable)
        return;
    if (con->is_xwayland_popup)
        return;
    if (container_get_hidden(con))
        return;

    struct monitor *m = tag_get_monitor(tag);
    struct container *sel = monitor_get_focused_container(m);

    /* Put the new client atop the focus stack */
    tag_repush_on_focus_stack(tag, con, 0);

    struct container *new_sel = monitor_get_focused_container(m);

    // it is a waste of resources to call unfocus and refocus when the
    // focused container didn't change
    if (sel != new_sel) {
        call_on_unfocus_function(server.event_handler, sel);
        call_on_focus_function(server.event_handler, con);
    }

    struct client *old_c = sel ? sel->client : NULL;
    struct client *new_c = new_sel ? new_sel->client : NULL;
    struct seat *seat = input_manager_get_default_seat();
    focus_client(seat, old_c, new_c);

    if (sel == new_sel)
        return;

    tag_update_names(server_get_tags());
    ipc_event_tag();
}


void tag_update_name(struct tag *tag)
{
    struct layout *lt = tag_get_layout(tag);
    if (!lt)
        return;
    int tag_id = tag->id;

    const char *default_name;
    // no number has more than 11 digits when int is 32 bit long
    char number_tmp[12];
    if (tag_id < lt->options->tag_names->len) {
        default_name = g_ptr_array_index(lt->options->tag_names, tag_id);
    } else {
        // TODO explain why +1
        snprintf(number_tmp, 12, "%d:%d", tag_id, c_idx_to_lua_idx(tag_id));
        default_name = number_tmp;
    }
    struct container *con = tag_get_local_focused_container(tag);

    const char *name = default_name;

    //TODO refactor
    char *num_name = strdup("");

    const char *app_id = container_get_app_id(con);
    if (con
            && app_id != NULL
            && g_strcmp0(app_id, "") != 0
       ) {

        name = app_id;

        char tag_number[12];
        char tag_name_number[12];
        sprintf(tag_number, "%lu:", tag->id);
        sprintf(tag_name_number, "%lu:", tag->id+1);
        append_string(&num_name, tag_number);
        append_string(&num_name, tag_name_number);
    }

    append_string(&num_name, name);

    char final_name[12];
    strncpy(final_name, num_name, 12);
    tag_rename(tag, final_name);
    free(num_name);
}

void tag_update_names(GList *tags)
{
    for (GList *iterator = tags; iterator; iterator = iterator->next) {
        struct tag *tag = iterator->data;
        tag_update_name(tag);
    }
}

struct container *tag_get_focused_container(struct tag *tag)
{
    if (!tag)
        return NULL;

    for (int i = 0; i < length_of_composed_list(tag->visible_focus_set->focus_stack_lists); i++) {
        struct container *con = get_in_composed_list(tag->visible_focus_set->focus_stack_lists, i);
        return con;
    }
    return NULL;
}

void list_set_add_container_to_containers(struct container_set *con_set, struct container *con, int i)
{
    list_insert(con_set->tiled_containers, i, con);
}

void tag_add_container_to_containers(struct tag *tag, int i, struct container *con)
{
    assert(con != NULL);

    DO_ACTION_GLOBALLY(server_get_tags(),
        list_set_add_container_to_containers(con_set, con, i);
    );
}

void tag_remove_container_from_containers_locally(struct tag *tag, struct container *con)
{
    struct layout *lt = tag_get_layout(tag);
    if (lt->options->arrange_by_focus) {
        remove_in_composed_list(tag->focus_set->focus_stack_lists, cmp_ptr, con);
        remove_in_composed_list(tag->visible_focus_set->focus_stack_lists, cmp_ptr, con);
    } else {
        DO_ACTION_LOCALLY(tag,
            g_ptr_array_remove(con_set->tiled_containers, con);
        );
    }
}

void tag_add_container_to_containers_locally(struct tag *tag, int i, struct container *con)
{
    struct layout *lt = tag_get_layout(tag);
    if (lt->options->arrange_by_focus) {
        list_set_insert_container_to_focus_stack(tag->focus_set, 0, con);
        update_reduced_focus_stack(tag);
    } else {
        DO_ACTION_LOCALLY(tag,
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

void tag_add_container_to_focus_stack(struct tag *tag, int pos, struct container *con)
{
    // TODO: refactor me
    struct container *prev_sel = tag_get_focused_container(tag);
    for (GList *iter = server_get_tags(); iter; iter = iter->next) {
        struct tag *tag = iter->data;
        list_set_insert_container_to_focus_stack(tag->focus_set, pos, con);
        update_reduced_focus_stack(tag);
    }
    struct container *sel = tag_get_focused_container(tag);
    if (prev_sel != sel) {
        struct event_handler *ev = server.event_handler;
        call_on_unfocus_function(ev, prev_sel);
        call_on_focus_function(ev, sel);
    }
}

void tag_remove_container_from_focus_stack_locally(struct tag *tag, struct container *con)
{
    remove_in_composed_list(tag->focus_set->focus_stack_lists, cmp_ptr, con);
    update_reduced_focus_stack(tag);
}

void tag_add_container_to_focus_stack_locally(struct tag *tag, struct container *con)
{
    list_set_insert_container_to_focus_stack(tag->focus_set, 0, con);
    update_reduced_focus_stack(tag);
}

void tag_remove_container_from_floating_stack_locally(struct tag *tag, struct container *con)
{
    DO_ACTION_LOCALLY(tag, 
            g_ptr_array_remove(con_set->tiled_containers, con);
            );
}

void tag_add_container_to_floating_stack_locally(struct tag *tag, int i, struct container *con)
{
    DO_ACTION_LOCALLY(tag,
            g_ptr_array_insert(con_set->tiled_containers, i, con);
            );
}

void tag_remove_container_from_visual_stack_layer(struct tag *tag, struct container *con)
{
    remove_in_composed_list(server.layer_visual_stack_lists, cmp_ptr, con);
}

void tag_add_container_to_visual_stack_layer(struct tag *tag, struct container *con)
{
    add_container_to_layer_stack(con);
}

void add_container_to_layer_stack(struct container *con)
{
    assert(con->client->type == LAYER_SHELL);

    con->client->layer = con->client->surface.layer->current.layer;
    g_ptr_array_insert(get_layer_list(con->client->layer), 0, con);
    return;
}

void remove_container_from_stack(struct tag *tag, struct container *con)
{
    g_ptr_array_remove(server.container_stack, con);
}

void add_container_to_stack(struct tag *tag, struct container *con)
{
    if (!con)
        return;
    assert(con->client->type != LAYER_SHELL);

    g_ptr_array_insert(server.container_stack, 0, con);
}

void tag_remove_container(struct tag *tag, struct container *con)
{
    DO_ACTION_GLOBALLY(server_get_tags(),
            g_ptr_array_remove(con_set->tiled_containers, con);
            );
}

void tag_remove_container_from_focus_stack(struct tag *tag, struct container *con)
{
    struct container *prev_sel = tag_get_focused_container(tag);
    for (GList *iter = server_get_tags(); iter; iter = iter->next) {
        struct tag *tag = iter->data;
        remove_in_composed_list(tag->focus_set->focus_stack_lists, cmp_ptr, con);
        remove_in_composed_list(tag->visible_focus_set->focus_stack_lists, cmp_ptr, con);
    }
    struct container *sel = tag_get_focused_container(tag);
    if (prev_sel != sel) {
        struct event_handler *ev = server.event_handler;
        call_on_unfocus_function(ev, prev_sel);
        call_on_focus_function(ev, sel);
    }
}

void tag_set_tags(struct tag *tag, BitSet *tags)
{
    bitset_assign_bitset(&tag->tags, tags);
}

void tag_set_prev_tags(struct tag *tag, struct BitSet *tags)
{
    bitset_assign_bitset(&tag->prev_tags, tags);
}

static int get_in_container_stack(struct container *con)
{
    if (!con)
        return INVALID_POSITION;

    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    guint position = 0;
    g_ptr_array_find(tag->con_set->tiled_containers, con, &position);
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

void tag_repush_containers(GPtrArray *array, int i, int abs_index)
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

void tag_swap_containers(GPtrArray *array, int i, int j)
{
    if (array->len <= 0) {
        return;
    }
    if (i < 0 || i >= array->len) {
        return;
    }
    if (i < 0 || j >= array->len) {
        return;
    }

    do {
      typeof(((array)->pdata)[i]) tmp = ((array)->pdata)[i];
      ((array)->pdata)[i] = ((array)->pdata)[j];
      ((array)->pdata)[j] = tmp;
    } while (0);
}

void tag_repush_tag(struct tag *tag, struct container *con, int new_pos)
{
    GPtrArray *tiled_list = tag_get_tiled_list_copy(tag);
    g_ptr_array_remove(tiled_list, con);
    g_ptr_array_insert(tiled_list, new_pos, con);

    GPtrArray *actual_tiled_list = tag_get_tiled_list(tag);
    sub_list_write_to_parent_list1D(actual_tiled_list, tiled_list);

    g_ptr_array_unref(tiled_list);

    tag_write_to_tags(tag);
}

void tag_repush_on_focus_stack(struct tag *tag, struct container *con, int new_pos)
{
    remove_in_composed_list(tag->visible_focus_set->focus_stack_lists, cmp_ptr, con);
    list_set_insert_container_to_focus_stack(tag->visible_focus_set, new_pos, con);
    focus_set_write_to_parent(tag->focus_set, tag->visible_focus_set);
}

bool tag_sticky_contains_client(struct tag *tag, struct client *client)
{
    return bitset_test(client->sticky_tags, tag->id);
}
