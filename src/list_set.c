#include "list_set.h"

#include <assert.h>

#include "ipc-server.h"
#include "server.h"
#include "utils/coreUtils.h"

void setup_list_set(struct list_set *list_set)
{
    wlr_list_init(&list_set->change_affected_list_sets);
    wlr_list_push(&list_set->change_affected_list_sets, list_set);

    wlr_list_init(&list_set->container_lists);
    wlr_list_init(&list_set->visible_container_lists);

    wlr_list_init(&list_set->independent_containers);
    wlr_list_init(&list_set->tiled_containers);
    wlr_list_init(&list_set->hidden_containers);
    wlr_list_init(&list_set->floating_containers);

    wlr_list_push(&list_set->container_lists, &list_set->tiled_containers);
    wlr_list_push(&list_set->container_lists, &list_set->floating_containers);
    wlr_list_push(&list_set->container_lists, &list_set->hidden_containers);

    wlr_list_push(&list_set->visible_container_lists, &list_set->tiled_containers);
    wlr_list_push(&list_set->visible_container_lists, &list_set->floating_containers);

    wlr_list_init(&list_set->focus_stack_lists);
    wlr_list_init(&list_set->focus_stack_visible_lists);
    wlr_list_init(&list_set->focus_stack_lists_with_layer_shell);

    wlr_list_init(&list_set->focus_stack_layer_background);
    wlr_list_init(&list_set->focus_stack_layer_bottom);
    wlr_list_init(&list_set->focus_stack_layer_top);
    wlr_list_init(&list_set->focus_stack_layer_overlay);
    wlr_list_init(&list_set->focus_stack_layer_bottom);
    wlr_list_init(&list_set->focus_stack_on_top);
    wlr_list_init(&list_set->focus_stack_normal);
    wlr_list_init(&list_set->focus_stack_hidden);
    wlr_list_init(&list_set->focus_stack_not_focusable);

    wlr_list_push(&list_set->focus_stack_lists, &list_set->focus_stack_layer_top);
    wlr_list_push(&list_set->focus_stack_lists, &list_set->focus_stack_on_top);
    wlr_list_push(&list_set->focus_stack_lists, &list_set->focus_stack_normal);
    wlr_list_push(&list_set->focus_stack_lists, &list_set->focus_stack_not_focusable);
    wlr_list_push(&list_set->focus_stack_lists, &list_set->focus_stack_hidden);

    wlr_list_push(&list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_layer_overlay);
    wlr_list_push(&list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_layer_top);
    wlr_list_push(&list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_on_top);
    wlr_list_push(&list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_normal);
    wlr_list_push(&list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_not_focusable);
    wlr_list_push(&list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_layer_bottom);
    wlr_list_push(&list_set->focus_stack_lists_with_layer_shell, &list_set->focus_stack_layer_background);

    wlr_list_push(&list_set->focus_stack_visible_lists, &list_set->focus_stack_on_top);
    wlr_list_push(&list_set->focus_stack_visible_lists, &list_set->focus_stack_normal);
    wlr_list_push(&list_set->focus_stack_visible_lists, &list_set->focus_stack_not_focusable);

    wlr_list_init(&list_set->all_lists);
    wlr_list_push(&list_set->all_lists, &list_set->floating_containers);
    wlr_list_push(&list_set->all_lists, &list_set->tiled_containers);
    wlr_list_push(&list_set->all_lists, &list_set->hidden_containers);
    wlr_list_push(&list_set->all_lists, &list_set->independent_containers);
    wlr_list_push(&list_set->all_lists, &list_set->focus_stack_layer_background);
    wlr_list_push(&list_set->all_lists, &list_set->focus_stack_layer_bottom);
    wlr_list_push(&list_set->all_lists, &list_set->focus_stack_layer_top);
    wlr_list_push(&list_set->all_lists, &list_set->focus_stack_layer_overlay);
    wlr_list_push(&list_set->all_lists, &list_set->focus_stack_on_top);
    wlr_list_push(&list_set->all_lists, &list_set->focus_stack_normal);
    wlr_list_push(&list_set->all_lists, &list_set->focus_stack_hidden);
    wlr_list_push(&list_set->all_lists, &list_set->focus_stack_not_focusable);
}

void add_change_affected_list_set(struct list_set *dest, struct list_set *src)
{
    wlr_list_push(&dest->change_affected_list_sets, src);
}

void add_container_to_containers(struct list_set *list_set, struct container *con, int i)
{
    assert(con != NULL);

    for (int j = 0; j < list_set->change_affected_list_sets.length; j++) {
        struct list_set *ls = list_set->change_affected_list_sets.items[j];
        if (con->floating) {
            wlr_list_insert(&ls->floating_containers, i, con);
            continue;
        }
        if (con->hidden) {
            wlr_list_insert(&ls->hidden_containers, i, con);
            continue;
        }
        wlr_list_insert(&ls->tiled_containers, i, con);
    }
}

void list_set_add_container_to_focus_stack(struct list_set *list_set, struct container *con)
{
    for (int j = 0; j < list_set->change_affected_list_sets.length; j++) {
        struct list_set *ls = list_set->change_affected_list_sets.items[j];
        if (con->client->type == LAYER_SHELL) {
            switch (con->client->surface.layer->current.layer) {
                case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
                    wlr_list_insert(&ls->focus_stack_layer_background, 0, con);
                    break;
                case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
                    wlr_list_insert(&ls->focus_stack_layer_bottom, 0, con);
                    break;
                case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
                    wlr_list_insert(&ls->focus_stack_layer_top, 0, con);
                    break;
                case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
                    wlr_list_insert(&ls->focus_stack_layer_overlay, 0, con);
                    break;
            }
            return;
        }
        if (con->on_top) {
            wlr_list_insert(&ls->focus_stack_on_top, 0, con);
            return;
        }
        if (!con->focusable) {
            wlr_list_insert(&ls->focus_stack_not_focusable, 0, con);
            return;
        }

        wlr_list_insert(&ls->focus_stack_normal, 0, con);
    }
}

void add_container_to_stack(struct container *con)
{
    if (!con)
        return;

    if (con->client->type == LAYER_SHELL) {
        switch (con->client->surface.layer->current.layer) {
            case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
                wlr_list_insert(&server.layer_visual_stack_background, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
                wlr_list_insert(&server.layer_visual_stack_bottom, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
                wlr_list_insert(&server.layer_visual_stack_top, 0, con);
                break;
            case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
                wlr_list_insert(&server.layer_visual_stack_overlay, 0, con);
                break;
        }
        return;
    }

    if (con->floating) {
        wlr_list_insert(&server.floating_visual_stack, 0, con);
        return;
    }

    wlr_list_insert(&server.tiled_visual_stack, 0, con);
}

void append_list_set(struct list_set *dest, struct list_set *src)
{
    for (int i = 0; i < dest->all_lists.length; i++) {
        struct wlr_list *dest_list = dest->all_lists.items[i];
        struct wlr_list *src_list = src->all_lists.items[i];
        wlr_list_cat(dest_list, src_list);
    }
}

void clear_list_set(struct list_set *list_set)
{
    for (int i = 0; i < list_set->all_lists.length; i++) {
        struct wlr_list *dest_list = list_set->all_lists.items[i];
        wlr_list_clear(dest_list, NULL);
    }
}

void subscribe_list_set(struct list_set *dest, struct list_set *src)
{
    wlr_list_push(&src->change_affected_list_sets, dest);
    append_list_set(dest, src);
}

void unsubscribe_list_set(struct list_set *dest, struct list_set *src)
{
    wlr_list_remove(&dest->change_affected_list_sets, cmp_ptr, &src);
    write_list_set(dest, src);
}

void write_list_set(struct list_set *dest, struct list_set *src)
{
    clear_list_set(dest);
    append_list_set(dest, src);
}

struct wlr_list *get_visible_lists(struct list_set *list_set)
{
    return &list_set->visible_container_lists;
}

struct wlr_list *get_tiled_list(struct list_set *list_set)
{
    return &list_set->tiled_containers;
}

struct wlr_list *get_floating_list(struct list_set *list_set)
{
    return &list_set->floating_containers;
}

struct wlr_list *get_hidden_list(struct list_set *list_set)
{
    return &list_set->hidden_containers;
}

void list_set_remove_container(struct list_set *list_set, struct container *con)
{
    for (int i = 0; i < list_set->change_affected_list_sets.length; i++) {
        struct list_set *ls = list_set->change_affected_list_sets.items[i];
        printf("listset: %p\n", ls);
        remove_in_composed_list(&ls->container_lists, cmp_ptr, con);
    }
}

void list_set_remove_container_from_focus_stack(struct list_set *list_set, struct container *con)
{
    for (int i = 0; i < list_set->change_affected_list_sets.length; i++) {
        struct list_set *ls = list_set->change_affected_list_sets.items[i];
        remove_in_composed_list(&ls->focus_stack_lists, cmp_ptr, con);
    }
}

void list_set_remove_independent_container(struct list_set *list_set, struct container *con)
{
    for (int i = 0; i < list_set->change_affected_list_sets.length; i++) {
        struct list_set *ls = list_set->change_affected_list_sets.items[i];
        wlr_list_remove(&ls->independent_containers, cmp_ptr, con);
    }
}
