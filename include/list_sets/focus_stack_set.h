#ifndef FOCUS_STACK_SET
#define FOCUS_STACK_SET

#include <glib.h>

#include "utils/coreUtils.h"

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
    GPtrArray *focus_stack_hidden;
    GPtrArray *focus_stack_not_focusable;
};

#endif /* FOCUS_STACK_SET */
