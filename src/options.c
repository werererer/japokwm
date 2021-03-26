#include "options.h"

#include <string.h>

#include "client.h"
#include "utils/coreUtils.h"
#include "layout.h"

void reset_tag_names(struct wlr_list *tag_names)
{
    wlr_list_init(tag_names);
    wlr_list_clear(tag_names);
    wlr_list_push(tag_names, "1:1");
    wlr_list_push(tag_names, "2:2");
    wlr_list_push(tag_names, "3:3");
    wlr_list_push(tag_names, "4:4");
    wlr_list_push(tag_names, "5:5");
    wlr_list_push(tag_names, "6:6");
    wlr_list_push(tag_names, "7:7");
    wlr_list_push(tag_names, "8:8");
    wlr_list_push(tag_names, "9:9");
}

struct options get_default_options()
{
    struct options options = {
        .resize_dir = 0,
        .layout_constraints = {
            .min_width = 0.1f,
            .min_height = 0.1f,
            .max_width = 1.0f,
            .max_height = 1.0f
        },
        .master_constraints = {
            .min_width = 0.1f,
            .min_height = 0.1f,
            .max_width = 1.0f,
            .max_height = 1.0f
        },
        .root_color = {0.3f, 0.3f, 0.3f, 1.0f},
        .focus_color = {1.0f, 0.0f, 0.0f, 1.0f},
        .border_color = {0.0f, 0.0f, 1.0f, 1.0f},
        .repeat_rate = 25,
        .repeat_delay = 600,
        .tile_border_px = 3,
        .float_border_px = 3,
        .inner_gap = 0,
        .outer_gap = 0,
        .event_handler = get_default_event_handler(),
        .monrule_count = 0,
        .monrules = NULL,
        .rule_count = 0,
        .rules = NULL,
        .modkey = 0,
        .arrange_by_focus = false,
        .hidden_edges = WLR_EDGE_NONE,
    };

    reset_tag_names(&options.tag_names);
    return options;
}

void copy_options(struct options *dest_option, struct options *src_option)
{
    memcpy(dest_option, src_option, sizeof(struct options));

    reset_tiled_client_borders(dest_option->tile_border_px);
    reset_floating_client_borders(dest_option->tile_border_px);
}
