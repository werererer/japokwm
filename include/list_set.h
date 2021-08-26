#ifndef LIST_SET_H
#define LIST_SET_H

#include <stdlib.h>
#include <glib.h>

#include "utils/coreUtils.h"

/*
 * list_set is used by tagsets and workspaces to hold all containers on them
 * */
struct list_set {
    /* list of all one dimensonal lists in list_set */
    GPtrArray2D *all_lists;
    /* consists out of the lists of tiled_containers, hidden_containers and
     * floating_containers */
    GPtrArray2D *container_lists;
    GPtrArray2D *visible_container_lists;

    GPtrArray *floating_containers;
    GPtrArray *tiled_containers;
    GPtrArray *hidden_containers;

    GPtrArray *independent_containers;

    GPtrArray2D *focus_stack_lists_with_layer_shell;
    GPtrArray2D *focus_stack_visible_lists;
    GPtrArray2D *focus_stack_lists;

    GPtrArray *focus_stack_layer_background;
    GPtrArray *focus_stack_layer_bottom;
    GPtrArray *focus_stack_layer_top;
    GPtrArray *focus_stack_layer_overlay;
    GPtrArray *focus_stack_on_top;
    GPtrArray *focus_stack_normal;
    GPtrArray *focus_stack_hidden;
    GPtrArray *focus_stack_not_focusable;
};

struct list_set *create_list_set();
void destroy_list_set(struct list_set *list_set);

void append_list_set(struct list_set *dest, struct list_set *src);
void clear_list_set(struct list_set *list_set);
// estimated time efficiency O(n*m)
void list_set_remove_list_set(struct list_set *dest, struct list_set *src);
void list_set_remove_containers(struct list_set *dest, GPtrArray *containers);

#endif /* LIST_SET_H */
