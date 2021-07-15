#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <stdlib.h>
#include <wlr/util/edges.h>
#include <glib.h>
#include "event_handler.h"

#define BLACK {0.0f, 0.0f, 0.0f, 1.0f}
#define WHITE {1.0f, 1.0f, 1.0f, 1.0f}
#define RED {1.0f, 0.0f, 0.0f, 1.0f}
#define GREEN {0.0f, 1.0f, 0.0f, 1.0f}
#define BLUE {0.0f, 0.0f, 1.0f, 1.0f}

enum hidden_edge_borders {
    NONE,
    VERTICAL,
    HORIZONTAL,
    BOTH,
    SMART
};

struct resize_constraints {
    float min_width;
    float max_width;
    float min_height;
    float max_height;
};

struct options {
    bool sloppy_focus;
    int tile_border_px;
    int float_border_px;
    int modkey;
    float root_color[4];
    float border_color[4];
    float focus_color[4];
    float text_color[4];
    float sel_overlay_color[4];
    float sel_text_color[4];

    struct resize_constraints layout_constraints;
    struct resize_constraints master_constraints;

    GPtrArray *tag_names;
    struct rule *rules;
    size_t rule_count;
    struct monrule *monrules;
    size_t monrule_count;

    int repeat_rate;
    int repeat_delay;
    int inner_gap;
    int outer_gap;

    bool arrange_by_focus;
    int resize_dir;

    struct event_handler *event_handler;
    int tag_names_ref;
    int default_layout_ref;

    enum wlr_edges hidden_edges;
    bool smart_hidden_edges;

    GPtrArray *keybindings;
};

struct options get_default_options();
GPtrArray *create_tagnames();
void copy_options(struct options *dest_option, struct options *src_option);

#endif /* OPTIONS_H */
