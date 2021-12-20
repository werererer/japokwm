#ifndef FOCUS_STACK_SET
#define FOCUS_STACK_SET

#include <glib.h>

#include "utils/coreUtils.h"

struct tag;

struct focus_set {
    GPtrArray2D *focus_stack_lists_with_layer_shell;
    GPtrArray2D *focus_stack_visible_lists;
    GPtrArray2D *focus_stack_lists;

    GPtrArray *focus_stack_layer_background;
    GPtrArray *focus_stack_layer_bottom;
    GPtrArray *focus_stack_layer_top;
    GPtrArray *focus_stack_layer_overlay;
    GPtrArray *focus_stack_on_top;
    GPtrArray *focus_stack_normal;
    GPtrArray *focus_stack_not_focusable;
};

struct focus_set *focus_set_create();
void focus_set_destroy(struct focus_set *focus_set);

void focus_set_write_to_parent(
        struct focus_set *parent,
        struct focus_set *child);
void focus_set_append(
        struct tag *ws,
        struct focus_set *dest,
        struct focus_set *src);
void focus_set_clear(struct focus_set *focus_set);

#endif /* FOCUS_STACK_SET */
