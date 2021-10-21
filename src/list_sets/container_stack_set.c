#include "list_sets/container_stack_set.h"

#include "client.h"
#include "container.h"
#include "list_sets/list_set.h"
#include "server.h"
#include "workspace.h"
#include "monitor.h"
#include "tagset.h"

struct container_set *create_container_set()
{
    struct container_set *con_set = calloc(1, sizeof(*con_set));

    con_set->tiled_containers = g_ptr_array_new();

    return con_set;
}

void destroy_container_set(struct container_set *con_set)
{
    g_ptr_array_unref(con_set->tiled_containers);
    free(con_set);
}

static bool is_container_valid_to_append(
        void *monitor_ptr,
        GPtrArray *src_list,
        struct container *src_con
        )
{
    struct monitor *m = monitor_ptr;
    if (tagset_exist_on(m, src_con)) {
        return true;
    }
    return false;
}

void container_set_write_to_parent(
        struct container_set *parent,
        struct container_set *child)
{
    sub_list_write_to_parent_list1D(
            parent->tiled_containers,
            child->tiled_containers);
}

void container_set_append(
        struct monitor *m,
        struct container_set *dest,
        struct container_set *src)
{
    list_append_list_under_condition(
            dest->tiled_containers,
            src->tiled_containers,
            is_container_valid_to_append,
            m);
}

void container_set_clear(struct container_set *list_set)
{
    list_clear(list_set->tiled_containers, NULL);
}
