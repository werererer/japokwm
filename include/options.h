#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <stdlib.h>
#include <wlr/types/wlr_list.h>

#define BLACK {0.0f, 0.0f, 0.0f, 1.0f}
#define WHITE {1.0f, 1.0f, 1.0f, 1.0f}
#define RED {1.0f, 0.0f, 0.0f, 1.0f}
#define GREEN {0.0f, 1.0f, 0.0f, 1.0f}
#define BLUE {0.0f, 0.0f, 1.0f, 1.0f}

struct options {
    bool sloppy_focus;
    int border_px;
    float root_color[4];
    float border_color[4];
    float focus_color[4];
    float text_color[4];
    float sel_overlay_color[4];
    float sel_text_color[4];

    struct wlr_list tag_names;
    struct rule *rules;
    size_t rule_count;
    struct mon_rule *monrules;
    size_t monrule_count;

    int repeat_rate;
    int repeat_delay;
    int inner_gap;
    int outer_gap;

    char *termcmd;
};

struct options get_default_options();
void copy_options(struct options *dest_option, struct options *src_option);

#endif /* OPTIONS_H */
