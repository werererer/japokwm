#ifndef VISUAL_STACK_SET_H
#define VISUAL_STACK_SET_H

#include <glib.h>
#include "utils/coreUtils.h"

struct visual_set {
    GPtrArray *stack_lists;
    GPtrArray2D *visual_stack_lists;

    GPtrArray *tiled_visual_stack;
    GPtrArray *floating_visual_stack;
};

struct visual_set *visual_set_create();
void visual_set_destroy(struct visual_set *visual_set);

#endif /* VISUAL_STACK_SET_H */
