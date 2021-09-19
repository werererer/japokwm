#ifndef CONTAINER_STACK_H
#define CONTAINER_STACK_H

#include <glib.h>

#include "utils/coreUtils.h"

struct workspace;

/*
 * list_set is used by tagsets and workspaces to hold all containers on them
 * */
struct container_set {
    /* consists out of the lists of tiled_containers, hidden_containers and
     * floating_containers */
    GPtrArray2D *container_lists;
    GPtrArray2D *global_floating_container_lists;
    GPtrArray2D *visible_container_lists;

    GPtrArray *floating_containers;
    GPtrArray *tiled_containers;
    GPtrArray *hidden_containers;
};

struct container_set *create_container_set();
void destroy_container_set(struct container_set *container_set);

void container_set_append(
        struct workspace *ws,
        struct container_set *dest,
        struct container_set *src);
void container_set_clear(struct container_set *list_set);

#endif /* CONTAINER_STACK_H */
