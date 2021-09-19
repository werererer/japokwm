#include "list_sets/visual_stack_set.h"
#include "server.h"

struct visual_set *visual_set_create()
{
    struct visual_set *visual_set = calloc(1, sizeof(struct visual_set));
    visual_set->all_stack_lists = g_ptr_array_new();
    visual_set->stack_lists = g_ptr_array_new();
    visual_set->visual_stack_lists = g_ptr_array_new();

    visual_set->tiled_visual_stack = g_ptr_array_new();
    visual_set->floating_visual_stack = g_ptr_array_new();

    g_ptr_array_add(visual_set->all_stack_lists, server.layer_visual_stack_overlay);
    g_ptr_array_add(visual_set->all_stack_lists, server.layer_visual_stack_top);
    g_ptr_array_add(visual_set->all_stack_lists, server.floating_stack);
    g_ptr_array_add(visual_set->all_stack_lists, visual_set->tiled_visual_stack);
    g_ptr_array_add(visual_set->all_stack_lists, server.layer_visual_stack_bottom);
    g_ptr_array_add(visual_set->all_stack_lists, server.layer_visual_stack_background);

    g_ptr_array_add(visual_set->stack_lists, server.layer_visual_stack_overlay);
    g_ptr_array_add(visual_set->stack_lists, server.layer_visual_stack_top);
    g_ptr_array_add(visual_set->stack_lists, visual_set->floating_visual_stack);
    g_ptr_array_add(visual_set->stack_lists, visual_set->tiled_visual_stack);
    g_ptr_array_add(visual_set->stack_lists, server.layer_visual_stack_bottom);
    g_ptr_array_add(visual_set->stack_lists, server.layer_visual_stack_background);

    g_ptr_array_add(visual_set->visual_stack_lists, visual_set->floating_visual_stack);
    g_ptr_array_add(visual_set->visual_stack_lists, visual_set->tiled_visual_stack);
    return visual_set;
}

void visual_set_destroy(struct visual_set *visual_set)
{
    // TODO fix me
    /* visual_set->visual_stack_lists = g_ptr_array_new(); */
    /* visual_set->normal_visual_stack_lists = g_ptr_array_new(); */
    /* visual_set->layer_visual_stack_lists = g_ptr_array_new(); */

    /* visual_set->tiled_visual_stack = g_ptr_array_new(); */
    /* visual_set->floating_visual_stack = g_ptr_array_new(); */
    /* visual_set->layer_visual_stack_background = g_ptr_array_new(); */
    /* visual_set->layer_visual_stack_bottom = g_ptr_array_new(); */
    /* visual_set->layer_visual_stack_top = g_ptr_array_new(); */
    /* visual_set->layer_visual_stack_overlay = g_ptr_array_new(); */
    free(visual_set);
}
