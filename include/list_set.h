#ifndef LIST_SET_H
#define LIST_SET_H

#include <stdlib.h>
#include <wlr/types/wlr_list.h>

struct list_set {
    /* list of all one dimensonal lists in list_set */
    struct wlr_list all_lists;
    /* consists out of the lists of tiled_containers, hidden_containers and
     * floating_containers */
    struct wlr_list container_lists;
    struct wlr_list visible_container_lists;

    struct wlr_list floating_containers;
    struct wlr_list tiled_containers;
    struct wlr_list hidden_containers;

    struct wlr_list independent_containers;

    /* 2D lists */
    struct wlr_list focus_stack_lists_with_layer_shell;
    struct wlr_list focus_stack_visible_lists;
    struct wlr_list focus_stack_lists;

    struct wlr_list focus_stack_layer_background;
    struct wlr_list focus_stack_layer_bottom;
    struct wlr_list focus_stack_layer_top;
    struct wlr_list focus_stack_layer_overlay;
    struct wlr_list focus_stack_on_top;
    struct wlr_list focus_stack_normal;
    struct wlr_list focus_stack_hidden;
    struct wlr_list focus_stack_not_focusable;

    /* this list must include a pointer to its parent */
    struct wlr_list change_affected_list_sets;
};

typedef void (*operation_t)(struct list_set *, void *);

void setup_list_set(struct list_set *ls);

void add_change_affected_list_set(struct list_set *dest, struct list_set *src);
void clear_list_set(struct list_set *list_set);
void unsubscribe_list_set(struct list_set *dest, struct list_set *src);
void subscribe_list_set(struct list_set *dest, struct list_set *src);
void write_list_set(struct list_set *dest, struct list_set *src);

struct wlr_list *get_visible_lists(struct list_set *list_set);
struct wlr_list *get_tiled_list(struct list_set *list_set);
struct wlr_list *get_floating_list(struct list_set *list_set);
struct wlr_list *get_hidden_list(struct list_set *list_set);

#endif /* LIST_SET_H */
