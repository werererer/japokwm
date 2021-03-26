#include "utils/gapUtils.h"
#include <math.h>

static void container_add_gap_left(struct wlr_box *con, double gap)
{
    con->x += gap;
    con->width -= gap;
}

static void container_add_gap_right(struct wlr_box *con, double gap)
{
    con->width = MAX(1, con->width - gap);
}

static void container_add_gap_top(struct wlr_box *con, double gap)
{
    con->y += gap;
    con->height -= gap;
}

static void container_add_gap_bottom(struct wlr_box *con, double gap)
{
    con->height = MAX(1, con->height - gap);
}

void container_add_gaps(struct wlr_box *con, double gap, enum wlr_edges edges) {
    if (edges & WLR_EDGE_LEFT)
        container_add_gap_left(con, gap);
    if (edges & WLR_EDGE_RIGHT)
        container_add_gap_right(con, gap);
    if (edges & WLR_EDGE_TOP)
        container_add_gap_top(con, gap);
    if (edges & WLR_EDGE_BOTTOM)
        container_add_gap_bottom(con, gap);
}

void container_surround_gaps(struct wlr_box *con, double gap)
{
    /* gaps are applied twice (because auf adjacent sides) therefore half the
     * gap*/
    container_add_gaps(con, gap/2, WLR_EDGE_TOP | WLR_EDGE_BOTTOM | WLR_EDGE_LEFT | WLR_EDGE_RIGHT);
}

void configure_gaps(int *inner_gap, int *outer_gap)
{
    /* innerGaps are applied twice because gaps don't overlap from two
     containers therefore it has to be divided by 2*/
    *inner_gap /= 2;
    /* outerGap + innerGap = resultGap but we only want the outerGap */
    *outer_gap -= *inner_gap;
}
