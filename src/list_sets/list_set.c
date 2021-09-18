#include "list_sets/list_set.h"

#include <assert.h>

#include "ipc-server.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "container.h"

struct container_set *create_list_set()
{
    struct container_set *list_set = calloc(1, sizeof(struct container_set));

    list_set->container_lists = g_ptr_array_new();
    list_set->visible_container_lists = g_ptr_array_new();
    list_set->global_floating_container_lists = g_ptr_array_new();

    list_set->tiled_containers = g_ptr_array_new();
    list_set->hidden_containers = g_ptr_array_new();
    list_set->floating_containers = g_ptr_array_new();

    g_ptr_array_add(list_set->container_lists, list_set->tiled_containers);
    g_ptr_array_add(list_set->container_lists, list_set->floating_containers);
    g_ptr_array_add(list_set->container_lists, list_set->hidden_containers);

    g_ptr_array_add(list_set->visible_container_lists, list_set->tiled_containers);
    g_ptr_array_add(list_set->visible_container_lists, list_set->floating_containers);

    g_ptr_array_add(list_set->global_floating_container_lists, list_set->tiled_containers);
    g_ptr_array_add(list_set->global_floating_container_lists, server.floating_containers);

    return list_set;
}

void destroy_list_set(struct container_set *list_set)
{
    g_ptr_array_free(list_set->tiled_containers, FALSE);
    g_ptr_array_free(list_set->hidden_containers, FALSE);
    g_ptr_array_free(list_set->floating_containers, FALSE);
    g_ptr_array_free(list_set->container_lists, FALSE);
    g_ptr_array_free(list_set->visible_container_lists, FALSE);
    free(list_set);
}

void clear_list_set(struct container_set *list_set)
{
    for (int i = 0; i < list_set->container_lists->len; i++) {
        GPtrArray *dest_list = g_ptr_array_index(list_set->container_lists, i);
        list_clear(dest_list, NULL);
    }
}
