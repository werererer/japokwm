#include "list_sets/list_set.h"

#include <assert.h>

#include "ipc-server.h"
#include "list_sets/container_stack_set.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "container.h"
#include "workspace.h"

static int find_insert_position(
        GPtrArray *src_list,
        GPtrArray *dest_list,
        struct container *con)
{
    // TODO further refactoring
    const int dest_len = dest_list->len;
    int dest_positions[dest_len];
    for (int i = 0; i < dest_list->len; i++) {
        struct container *dest_con = g_ptr_array_index(dest_list, i);

        guint pos = 0;
        g_ptr_array_find(src_list, dest_con, &pos);
        dest_positions[i] = pos;
    }

    int src_pos = 0;
    guint pos;
    g_ptr_array_find(src_list, con, &pos);
    src_pos = pos;

    int final_pos = 1 + lower_bound(
            &src_pos,
            dest_positions,
            dest_len,
            sizeof(int),
            cmp_int);

    return final_pos;
}

static void add_to_list(GPtrArray *dest, GPtrArray *src, struct container *src_con)
{
    if (dest->len == 0) {
        g_ptr_array_add(dest, src_con);
        return;
    }

    int insert_position = find_insert_position(src, dest, src_con);
    g_ptr_array_insert(dest, insert_position, src_con);
}

void list_append_list_under_condition(
        GPtrArray *dest,
        GPtrArray *src,
        is_condition_t is_condition,
        void *arg
        )
{
    for (int i = 0; i < src->len; i++) {
        struct container *src_con = g_ptr_array_index(src, i);

        if (!is_condition(arg, src, src_con))
            continue;

        add_to_list(dest, src, src_con);
    }
}

GPtrArray *list_create_sub_list(GPtrArray *list, int start, int end)
{
    GPtrArray *sub_list = g_ptr_array_new();

    assert(start >= 0);
    assert(end <= list->len);

    if (list->len == 0)
        return sub_list;
    if (start >= end)
        return sub_list;


    for (int i = start; i < end; i++) 
    {
        struct container *con = g_ptr_array_index(list, i);
        g_ptr_array_add(sub_list, con);
    }
    return sub_list;
}

GPtrArray *list2D_create_filtered_sub_list(
        GPtrArray2D *lists,
        bool is_condition(struct container *con))
{
    GPtrArray *visible_global_floating_list_copy = g_ptr_array_new();
    for (int i = 0; i < length_of_composed_list(lists); i++) {
        struct container *con = get_in_composed_list(lists, i);
        if (!is_condition(con)) {
            continue;
        }
        g_ptr_array_add(visible_global_floating_list_copy, con);
    }
    return visible_global_floating_list_copy;
}

GPtrArray *list_create_filtered_sub_list_with_order(
        GPtrArray *list,
        GPtrArray *conditions)
{
    GPtrArray *final_list = g_ptr_array_new();

    for (int i = 0; i < conditions->len; i++) {
        bool (*arg_is_condition)(struct container *) = g_ptr_array_index(conditions, i);
        GPtrArray *visible_global_floating_list_copy = 
            list_create_filtered_sub_list(list, arg_is_condition); 
        wlr_list_cat(final_list, visible_global_floating_list_copy);
        g_ptr_array_free(visible_global_floating_list_copy, FALSE);
    }

    return final_list;
}

GPtrArray *list_create_filtered_sub_list(
        GPtrArray *list,
        bool is_condition(struct container *con))
{
    GPtrArray *visible_global_floating_list_copy = g_ptr_array_new();
    for (int i = 0; i < list->len; i++) {
        struct container *con = g_ptr_array_index(list, i);
        if (!is_condition(con)) {
            continue;
        }
        g_ptr_array_add(visible_global_floating_list_copy, con);
    }
    return visible_global_floating_list_copy;
}


void lists_cat_to_list(GPtrArray *dest, GPtrArray2D *src)
{
    for (int i = 0; i < src->len; i++) {
        GPtrArray *src_list = g_ptr_array_index(src, i);
        wlr_list_cat(dest, src_list);
    }
}

void lists_append_list_under_condition(
        GPtrArray2D *dest,
        GPtrArray2D *src,
        is_condition_t is_condition,
        struct workspace *ws
        )
{
    assert(src->len == dest->len);

    for (int i = 0; i < dest->len; i++) {
        GPtrArray *dest_list = g_ptr_array_index(dest, i);
        GPtrArray *src_list = g_ptr_array_index(src, i);
        list_append_list_under_condition(dest_list, src_list, is_condition, ws);
    }
}

static GArray *child_lists_get_positions_array(GPtrArray2D *child, GPtrArray2D *parent)
{
    GArray *positions = g_array_new(false, false, sizeof(int));
    for (int i = 0; i < length_of_composed_list(child); i++) {
        struct container *con = get_in_composed_list(child, i);
        int position = find_in_composed_list(parent, cmp_ptr, con);
        g_array_append_val(positions, position);
    }
    return positions;
}

static GArray *child_list_get_positions_array(GPtrArray *child, GPtrArray *parent)
{
    GArray *positions = g_array_new(false, false, sizeof(int));
    for (int i = 0; i < child->len; i++) {
        struct container *con = g_ptr_array_index(child, i);
        guint position;
        g_ptr_array_find(parent, con, &position);
        g_array_append_val(positions, position);
    }
    return positions;
}


void sub_list_write_to_parent_list(GPtrArray2D *parent,
        GPtrArray2D *child)
{
    GArray *positions = child_lists_get_positions_array(child, parent);

    GArray *prev_positions = g_array_copy(positions);
    g_array_sort(prev_positions, cmp_int);

    GPtrArray *parent_containers = g_ptr_array_new();
    lists_cat_to_list(parent_containers, parent);

    for (int i = 0; i < prev_positions->len; i++) {
        int prev_position = g_array_index(prev_positions, int, i);
        int position = g_array_index(positions, int, i);
        struct container *prev_con = g_ptr_array_index(parent_containers, position);
        set_in_composed_list(parent, prev_position, prev_con);
    }

    g_ptr_array_free(parent_containers, false);
    g_array_free(prev_positions, false);
    g_array_free(positions, false);
}

void sub_list_write_to_parent_list1D(GPtrArray *parent, GPtrArray *child)
{
    GArray *positions = child_list_get_positions_array(child, parent);

    GArray *prev_positions = g_array_copy(positions);
    g_array_sort(prev_positions, cmp_int);

    GPtrArray *parent_containers = g_ptr_array_new();
    wlr_list_cat(parent_containers, parent);

    for (int i = 0; i < prev_positions->len; i++) {
        int prev_position = g_array_index(prev_positions, int, i);
        int position = g_array_index(positions, int, i);
        struct container *prev_con = g_ptr_array_index(parent_containers, position);
        g_ptr_array_index(parent, prev_position) = prev_con;
    }

    g_ptr_array_free(parent_containers, false);
    g_array_free(prev_positions, false);
    g_array_free(positions, false);
}

void lists_clear(GPtrArray2D *lists)
{
    for (int i = 0; i < lists->len; i++) {
        GPtrArray *list = g_ptr_array_index(lists, i);
        list_clear(list, NULL);
    }
}

