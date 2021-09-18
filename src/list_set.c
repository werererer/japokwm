#include "list_set.h"

#include <assert.h>

#include "ipc-server.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "container.h"

struct list_set *create_list_set()
{
    struct list_set *list_set = calloc(1, sizeof(struct list_set));

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

void destroy_list_set(struct list_set *list_set)
{
    g_ptr_array_free(list_set->tiled_containers, FALSE);
    g_ptr_array_free(list_set->hidden_containers, FALSE);
    g_ptr_array_free(list_set->floating_containers, FALSE);
    g_ptr_array_free(list_set->container_lists, FALSE);
    g_ptr_array_free(list_set->visible_container_lists, FALSE);
    free(list_set);
}

void append_list_set(struct list_set *dest, struct list_set *src)
{
    for (int i = 0; i < dest->container_lists->len; i++) {
        GPtrArray *dest_list = g_ptr_array_index(dest->container_lists, i);
        GPtrArray *src_list = g_ptr_array_index(src->container_lists, i);
        wlr_list_cat(dest_list, src_list);
    }
}

void clear_list_set(struct list_set *list_set)
{
    /* debug_print("\nclear_list_set\n"); */
    for (int i = 0; i < list_set->container_lists->len; i++) {
        GPtrArray *dest_list = g_ptr_array_index(list_set->container_lists, i);
        /* debug_print("prev list data: %p\n", dest_list->pdata); */
        list_clear(dest_list, NULL);
        /* debug_print("list data: %p\n", dest_list->pdata); */
    }
}
