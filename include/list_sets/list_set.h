#ifndef LIST_SET_H
#define LIST_SET_H

#include <stdlib.h>
#include <glib.h>

#include "utils/coreUtils.h"

struct workspace;
struct container;

typedef bool is_condition_t(
        struct workspace *ws,
        GPtrArray *src_list,
        struct container *con
        );

void lists_append_list_under_condition(
        GPtrArray2D *dest,
        GPtrArray2D *src,
        is_condition_t condition,
        struct workspace *ws
        );
void lists_clear(GPtrArray2D *lists);

#endif /* LIST_SET_H */
