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

static void list_append_list_under_condition(
        GPtrArray *dest,
        GPtrArray *src,
        is_condition_t is_condition,
        struct workspace *ws
        )
{
    printf("\nstart\n");
    for (int i = 0; i < src->len; i++) {
        struct container *src_con = g_ptr_array_index(src, i);
        printf("src: %p\n", src_con);

        if (!is_condition(ws, src, src_con))
            continue;

        add_to_list(dest, src, src_con);
    }
    printf("end\n");
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

void lists_clear(GPtrArray2D *lists)
{
    for (int i = 0; i < lists->len; i++) {
        GPtrArray *list = g_ptr_array_index(lists, i);
        list_clear(list, NULL);
    }
}

