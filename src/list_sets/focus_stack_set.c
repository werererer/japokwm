#include "list_sets/focus_stack_set.h"

#include "client.h"
#include "container.h"
#include "tag.h"
#include "list_sets/list_set.h"

struct focus_set *focus_set_create()
{
    struct focus_set *focus_set = calloc(1, sizeof(*focus_set));
    focus_set->focus_stack_lists_with_layer_shell = g_ptr_array_new();
    focus_set->focus_stack_lists = g_ptr_array_new();
    focus_set->focus_stack_visible_lists = g_ptr_array_new();

    focus_set->focus_stack_layer_background = g_ptr_array_new();
    focus_set->focus_stack_layer_bottom = g_ptr_array_new();
    focus_set->focus_stack_layer_top = g_ptr_array_new();
    focus_set->focus_stack_layer_overlay = g_ptr_array_new();
    focus_set->focus_stack_on_top = g_ptr_array_new();
    focus_set->focus_stack_normal = g_ptr_array_new();
    focus_set->focus_stack_not_focusable = g_ptr_array_new();

    g_ptr_array_add(focus_set->focus_stack_lists, focus_set->focus_stack_layer_top);
    g_ptr_array_add(focus_set->focus_stack_lists, focus_set->focus_stack_on_top);
    g_ptr_array_add(focus_set->focus_stack_lists, focus_set->focus_stack_normal);
    g_ptr_array_add(focus_set->focus_stack_lists, focus_set->focus_stack_not_focusable);

    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_layer_overlay);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_layer_top);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_on_top);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_normal);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_not_focusable);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_layer_bottom);
    g_ptr_array_add(focus_set->focus_stack_lists_with_layer_shell, focus_set->focus_stack_layer_background);

    g_ptr_array_add(focus_set->focus_stack_visible_lists, focus_set->focus_stack_on_top);
    g_ptr_array_add(focus_set->focus_stack_visible_lists, focus_set->focus_stack_normal);

    return focus_set;
}

void focus_set_destroy(struct focus_set *focus_set)
{
    g_ptr_array_free(focus_set->focus_stack_layer_background, FALSE);
    g_ptr_array_unref(focus_set->focus_stack_layer_bottom);
    g_ptr_array_unref(focus_set->focus_stack_layer_top);
    g_ptr_array_unref(focus_set->focus_stack_layer_overlay);
    g_ptr_array_unref(focus_set->focus_stack_on_top);
    g_ptr_array_unref(focus_set->focus_stack_normal);
    g_ptr_array_unref(focus_set->focus_stack_not_focusable);

    g_ptr_array_unref(focus_set->focus_stack_lists);
    g_ptr_array_unref(focus_set->focus_stack_visible_lists);
    g_ptr_array_unref(focus_set->focus_stack_lists_with_layer_shell);

    free(focus_set);
}


void focus_set_write_to_parent(
        struct focus_set *parent,
        struct focus_set *child)
{
    sub_list_write_to_parent_list2D(
            parent->focus_stack_lists,
            child->focus_stack_lists);
}

static bool is_container_valid_to_append(
        void *workspace_ptr,
        GPtrArray *src_list,
        struct container *src_con
        )
{
    struct tag *tag = workspace_ptr;
    if (src_con->ws_id != tag->id) {
        return false;
    }

    return true;
}

void focus_set_append(
        struct tag *tag,
        struct focus_set *dest,
        struct focus_set *src)
{
    lists_append_list_under_condition(
            dest->focus_stack_lists,
            src->focus_stack_lists,
            is_container_valid_to_append,
            tag);
}

void focus_set_clear(struct focus_set *focus_set)
{
    lists_clear(focus_set->focus_stack_lists);
}
