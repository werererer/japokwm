#include "options.h"

#include <string.h>

struct options get_default_options()
{
    return (struct options) {
        .root_color = {0.0f, 0.0f, 0.0f, 1.0f},
        .border_color = {0.0f, 0.0f, 0.0f, 1.0f},
        .focus_color = {0.0f, 0.0f, 0.0f, 1.0f},
        .border_px = 3,
        .inner_gap = 10,
        .outer_gap = 10,
        .monrule_count = 0,
        .monrules = NULL,
        .rule_count = 0,
        .rules = NULL,
    };
}
