#include "list_sets/container_stack_set.h"

#include "client.h"
#include "container.h"
#include "list_sets/list_set.h"
#include "server.h"
#include "workspace.h"

struct container_set *create_container_set()
{
    struct container_set *con_set = calloc(1, sizeof(struct container_set));

    con_set->container_lists = g_ptr_array_new();
    con_set->visible_container_lists = g_ptr_array_new();

    con_set->tiled_containers = g_ptr_array_new();

    g_ptr_array_add(con_set->container_lists, con_set->tiled_containers);

    g_ptr_array_add(con_set->visible_container_lists, con_set->tiled_containers);

    return con_set;
}

void destroy_container_set(struct container_set *con_set)
{
    g_ptr_array_free(con_set->tiled_containers, FALSE);
    g_ptr_array_free(con_set->container_lists, FALSE);
    g_ptr_array_free(con_set->visible_container_lists, FALSE);
    free(con_set);
}

static bool is_container_valid_to_append(
        struct workspace *ws,
        GPtrArray *src_list,
        struct container *src_con
        )
{
    if (src_con->client->ws_id != ws->id) {
        return false;
    }

    return true;
}

void container_set_write_to_parent(
        struct container_set *parent,
        struct container_set *child)
{
    sub_list_write_to_parent_list(
            parent->container_lists,
            child->container_lists);
}

void container_set_append(
        struct workspace *ws,
        struct container_set *dest,
        struct container_set *src)
{
    lists_append_list_under_condition(
            dest->container_lists,
            src->container_lists,
            is_container_valid_to_append,
            ws);
}

void container_set_clear(struct container_set *list_set)
{
    lists_clear(list_set->container_lists);
}
