#ifndef TAGSET_H
#define TAGSET_H

#include <stdlib.h>
#include <wlr/types/wlr_list.h>

#include "bitset/bitset.h"

struct tagset {
    BitSet workspaces;

    /* consists out of the lists of tiled_containers, hidden_containers and
     * floating_containers */
    struct wlr_list container_lists;
    struct wlr_list visible_container_lists;

    struct wlr_list floating_containers;
    struct wlr_list tiled_containers;
    struct wlr_list hidden_containers;

    struct wlr_list independent_containers;

    struct wlr_list focus_stack_lists_with_layer_shell;
    struct wlr_list focus_stack_visible_lists;
    struct wlr_list focus_stack_lists;

    struct wlr_list focus_stack_layer_background;
    struct wlr_list focus_stack_layer_bottom;
    struct wlr_list focus_stack_layer_top;
    struct wlr_list focus_stack_layer_overlay;
    struct wlr_list focus_stack_on_top;
    struct wlr_list focus_stack_normal;
    struct wlr_list focus_stack_hidden;
    struct wlr_list focus_stack_not_focusable;
};

#endif /* TAGSET_H */
