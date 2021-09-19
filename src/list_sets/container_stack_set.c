#include "list_sets/container_stack_set.h"

#include "client.h"
#include "container.h"
#include "list_sets/list_set.h"
#include "workspace.h"

static bool is_valid_for_container_list(
        struct workspace *ws,
        GPtrArray *src_list,
        struct container *src_con
        )
{
    if (src_con->client->ws_id != ws->id) {
        return false;
    }

    return true;
}

void container_set_append(
        struct workspace *ws,
        struct container_set *dest,
        struct container_set *src)
{
    lists_append_list_under_condition(
            dest->container_lists,
            src->container_lists,
            is_valid_for_container_list,
            ws);
}
