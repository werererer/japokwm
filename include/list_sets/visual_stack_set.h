#ifndef VISUAL_STACK_SET_H
#define VISUAL_STACK_SET_H

#include <glib.h>
#include "utils/coreUtils.h"

struct visual_set {
    GPtrArray *all_stack_lists;
    GPtrArray2D *stack_lists;
    GPtrArray2D *visual_stack_lists;

    GPtrArray *tiled_visual_stack;
    GPtrArray *floating_visual_stack;
};

#endif /* VISUAL_STACK_SET_H */
