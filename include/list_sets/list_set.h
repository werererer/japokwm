#ifndef LIST_SET_H
#define LIST_SET_H

#include <stdlib.h>
#include <glib.h>

#include "utils/coreUtils.h"

struct container_set *create_list_set();
void destroy_list_set(struct container_set *list_set);

void append_list_set(struct container_set *dest, struct container_set *src);
void clear_list_set(struct container_set *list_set);

#endif /* LIST_SET_H */
