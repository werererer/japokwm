#include "list_set.h"

#include <assert.h>

#include "ipc-server.h"
#include "server.h"
#include "utils/coreUtils.h"

struct list_set *create_list_set()
{
    struct list_set *list_set = calloc(1, sizeof(struct list_set));
    list_set->change_affected_list_sets = g_ptr_array_new();
    g_ptr_array_add(list_set->change_affected_list_sets, list_set);

    list_set->container_lists = g_ptr_array_new();
    list_set->visible_container_lists = g_ptr_array_new();

    list_set->independent_containers = g_ptr_array_new();
    list_set->tiled_containers = g_ptr_array_new();
    list_set->hidden_containers = g_ptr_array_new();
    list_set->floating_containers = g_ptr_array_new();

    g_ptr_array_add(list_set->container_lists, &list_set->tiled_containers);
    g_ptr_array_add(list_set->container_lists, &list_set->floating_containers);
    g_ptr_array_add(list_set->container_lists, &list_set->hidden_containers);

    g_ptr_array_add(list_set->visible_container_lists, &list_set->tiled_containers);
    g_ptr_array_add(list_set->visible_container_lists, &list_set->floating_containers);

    list_set->focus_stack_lists = g_ptr_array_new();
    list_set->focus_stack_visible_lists = g_ptr_array_new();
    list_set->focus_stack_lists_with_layer_shell = g_ptr_array_new();

    list_set->focus_stack_layer_background = g_ptr_array_new();
    list_set->focus_stack_layer_bottom = g_ptr_array_new();
    list_set->focus_stack_layer_top = g_ptr_array_new();
    list_set->focus_stack_layer_overlay = g_ptr_array_new();
    list_set->focus_stack_layer_bottom = g_ptr_array_new();
    list_set->focus_stack_on_top = g_ptr_array_new();
    list_set->focus_stack_normal = g_ptr_array_new();
    list_set->focus_stack_hidden = g_ptr_array_new();
    list_set->focus_stack_not_focusable = g_ptr_array_new();

    g_ptr_array_add(list_set->focus_stack_lists, &list_set->focus_stack_layer_top);
    g_ptr_array_add(list_set->focus_stack_lists, &list_set->focus_stack_on_top);
    g_ptr_array_add(list_set->focus_stack_lists, &list_set->focus_stack_normal);
    g_ptr_array_add(list_set->focus_stack_lists, &list_set->focus_stack_not_focusable);
    g_ptr_array_add(list_set->focus_stack_lists, &list_set->focus_stack_hidden);

    g_ptr_array_add(list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_layer_overlay);
    g_ptr_array_add(list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_layer_top);
    g_ptr_array_add(list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_on_top);
    g_ptr_array_add(list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_normal);
    g_ptr_array_add(list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_not_focusable);
    g_ptr_array_add(list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_layer_bottom);
    g_ptr_array_add(list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_layer_background);

    g_ptr_array_add(list_set->focus_stack_visible_lists, &list_set->focus_stack_on_top);
    g_ptr_array_add(list_set->focus_stack_visible_lists, &list_set->focus_stack_normal);
    g_ptr_array_add(list_set->focus_stack_visible_lists, &list_set->focus_stack_not_focusable);

    list_set->all_lists = g_ptr_array_new();
    g_ptr_array_add(list_set->all_lists, &list_set->floating_containers);
    g_ptr_array_add(list_set->all_lists, &list_set->tiled_containers);
    g_ptr_array_add(list_set->all_lists, &list_set->hidden_containers);
    g_ptr_array_add(list_set->all_lists, &list_set->independent_containers);
    g_ptr_array_add(list_set->all_lists, &list_set->focus_stack_layer_background);
    g_ptr_array_add(list_set->all_lists, &list_set->focus_stack_layer_bottom);
    g_ptr_array_add(list_set->all_lists, &list_set->focus_stack_layer_top);
    g_ptr_array_add(list_set->all_lists, &list_set->focus_stack_layer_overlay);
    g_ptr_array_add(list_set->all_lists, &list_set->focus_stack_on_top);
    g_ptr_array_add(list_set->all_lists, &list_set->focus_stack_normal);
    g_ptr_array_add(list_set->all_lists, &list_set->focus_stack_hidden);
    g_ptr_array_add(list_set->all_lists, &list_set->focus_stack_not_focusable);
    return list_set;
}

void destroy_list_set(struct list_set *list_set)
{
    g_ptr_array_free(list_set->change_affected_list_sets, TRUE);
    g_ptr_array_free(list_set->container_lists, TRUE);
    g_ptr_array_free(list_set->visible_container_lists, TRUE);
    g_ptr_array_free(list_set->independent_containers, TRUE);
    g_ptr_array_free(list_set->tiled_containers, TRUE);
    g_ptr_array_free(list_set->hidden_containers, TRUE);
    g_ptr_array_free(list_set->floating_containers, TRUE);
    g_ptr_array_free(list_set->focus_stack_lists, TRUE);
    g_ptr_array_free(list_set->focus_stack_visible_lists, TRUE);
    g_ptr_array_free(list_set->focus_stack_lists_with_layer_shell, TRUE);
    g_ptr_array_free(list_set->focus_stack_layer_background, TRUE);
    g_ptr_array_free(list_set->focus_stack_layer_bottom, TRUE);
    g_ptr_array_free(list_set->focus_stack_layer_top, TRUE);
    g_ptr_array_free(list_set->focus_stack_layer_overlay, TRUE);
    g_ptr_array_free(list_set->focus_stack_layer_bottom, TRUE);
    g_ptr_array_free(list_set->focus_stack_on_top, TRUE);
    g_ptr_array_free(list_set->focus_stack_normal, TRUE);
    g_ptr_array_free(list_set->focus_stack_hidden, TRUE);
    g_ptr_array_free(list_set->focus_stack_not_focusable, TRUE);
    g_ptr_array_free(list_set->all_lists, TRUE);
    free(list_set);
}

void append_list_set(struct list_set *dest, struct list_set *src)
{
    for (int i = 0; i < dest->all_lists->len; i++) {
        struct wlr_list *dest_list = g_ptr_array_index(dest->all_lists, i);
        struct wlr_list *src_list = g_ptr_array_index(src->all_lists, i);
        wlr_list_cat(dest_list, src_list);
    }
}

void clear_list_set(struct list_set *list_set)
{
    for (int i = 0; i < list_set->all_lists->len; i++) {
        GPtrArray *dest_list = g_ptr_array_index(list_set->all_lists, i);
        wlr_list_clear(dest_list, NULL);
    }
}

void subscribe_list_set(struct list_set *dest, struct list_set *src)
{
    g_ptr_array_add(src->change_affected_list_sets, dest);
    append_list_set(dest, src);
}

void unsubscribe_list_set(struct list_set *dest, struct list_set *src)
{
    g_ptr_array_remove(src->change_affected_list_sets, dest);
}
