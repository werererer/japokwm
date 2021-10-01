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

struct workspace;

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
    GPtrArray *rules;
    GPtrArray *mon_rules;

    // timeout in milliseconds
    int key_combo_timeout;
    int repeat_rate;
    int repeat_delay;
    int inner_gap;
    int outer_gap;

    bool arrange_by_focus;
    int resize_dir;

    struct event_handler *event_handler;

    enum wlr_edges hidden_edges;
    bool smart_hidden_edges;
    bool automatic_workspace_naming;

    GPtrArray *keybindings;

    int new_position_func_ref;
    int new_focus_position_func_ref;
};

struct options *create_options();
void destroy_options(struct options *options);
void options_reset(struct options *options);
void load_default_keybindings();
GPtrArray *create_tagnames();
void copy_options(struct options *dest_option, struct options *src_option);

int workspace_get_new_position(struct workspace *ws);
int workspace_get_new_focus_position(struct workspace *ws);

#endif /* OPTIONS_H */
