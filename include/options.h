#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <stdlib.h>
#include <wlr/util/edges.h>
#include <glib.h>
#include "event_handler.h"

#include "color.h"

struct workspace;
struct keybinding;

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
    struct color root_color;
    struct color border_color;
    struct color focus_color;
    struct color text_color;
    struct color sel_overlay_color;
    struct color sel_text_color;

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
    enum wlr_edges resize_dir;

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

void options_add_keybinding(GPtrArray *keybindings, struct keybinding *keybinding);

int workspace_get_new_position(struct workspace *ws);
int workspace_get_new_focus_position(struct workspace *ws);

#endif /* OPTIONS_H */
