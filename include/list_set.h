#ifndef LIST_SET_H
#define LIST_SET_H

#include <stdlib.h>
#include <wlr/types/wlr_list.h>
#include <glib.h>

struct list_set {
    /* list of all one dimensonal lists in list_set */
    GPtrArray *all_lists;
    /* consists out of the lists of tiled_containers, hidden_containers and
     * floating_containers */
    GPtrArray *container_lists;
    GPtrArray *visible_container_lists;

    GPtrArray *floating_containers;
    GPtrArray *tiled_containers;
    GPtrArray *hidden_containers;

    GPtrArray *independent_containers;

    /* 2D lists */
    GPtrArray *focus_stack_lists_with_layer_shell;
    GPtrArray *focus_stack_visible_lists;
    GPtrArray *focus_stack_lists;

    GPtrArray *focus_stack_layer_background;
    GPtrArray *focus_stack_layer_bottom;
    GPtrArray *focus_stack_layer_top;
    GPtrArray *focus_stack_layer_overlay;
    GPtrArray *focus_stack_on_top;
    GPtrArray *focus_stack_normal;
    GPtrArray *focus_stack_hidden;
    GPtrArray *focus_stack_not_focusable;

    /* this list must include a pointer to its parent */
    GPtrArray *change_affected_list_sets;
};

struct list_set *create_list_set();
void destroy_list_set(struct list_set *list_set);

void append_list_set(struct list_set *dest, struct list_set *src);
void clear_list_set(struct list_set *list_set);
void subscribe_list_set(struct list_set *dest, struct list_set *src);
void unsubscribe_list_set(struct list_set *dest, struct list_set *src);

#endif /* LIST_SET_H */
