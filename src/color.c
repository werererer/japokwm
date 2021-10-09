#include "color.h"

void color_to_wlr_color(float *dest_wlr_color, struct color color)
{
    dest_wlr_color[0] = color.red;
    dest_wlr_color[1] = color.green;
    dest_wlr_color[2] = color.blue;
    dest_wlr_color[3] = color.alpha;
}
