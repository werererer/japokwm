#ifndef LIST_SET_H
#define LIST_SET_H

#include <stdlib.h>
#include <glib.h>

#include "utils/coreUtils.h"

/*
 * list_set is used by tagsets and workspaces to hold all containers on them
 * */
struct list_set {
    /* consists out of the lists of tiled_containers, hidden_containers and
     * floating_containers */
    GPtrArray2D *container_lists;
    GPtrArray2D *global_floating_container_lists;
    GPtrArray2D *visible_container_lists;

    GPtrArray *floating_containers;
    GPtrArray *tiled_containers;
    GPtrArray *hidden_containers;
};

struct list_set *create_list_set();
void destroy_list_set(struct list_set *list_set);

void append_list_set(struct list_set *dest, struct list_set *src);
void clear_list_set(struct list_set *list_set);

#endif /* LIST_SET_H */
