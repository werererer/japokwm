#include "utils/gapUtils.h"
#include <math.h>

static void containerAddGapLeft(struct wlr_box *con, float gap)
{
    con->x += gap;
}

static void containerAddGapRight(struct wlr_box *con, float gap)
{
    con->width = MAX(1, con->width - gap);
}

static void containerAddGapTop(struct wlr_box *con, float gap)
{
    con->y += gap;
}

static void containerAddGapBottom(struct wlr_box *con, float gap)
{
    con->height = MAX(1, con->height - gap);
}

void containerAddGaps(struct wlr_box *con, double gap, enum wlr_edges edges) {
    if (edges & WLR_EDGE_LEFT)
        containerAddGapLeft(con, gap);
    if (edges & WLR_EDGE_RIGHT)
        containerAddGapRight(con, gap);
    if (edges & WLR_EDGE_TOP)
        containerAddGapTop(con, gap);
    if (edges & WLR_EDGE_BOTTOM)
        containerAddGapBottom(con, gap);
}

void container_surround_gaps(struct wlr_box *con, double gap)
{
    /* *
     * left = x and top = y
     * right = width and bottom = height
     * +------------+
     * |            |
     * |   +----+   |
     * |-->|    |<--|
     * | x +----+ x |
     * |            |
     * +------------+
     * therefore x and y need to be 1/2ed and the width has to be decreased
     * by the whole amound
     * */
        containerAddGaps(con, gap/2, WLR_EDGE_TOP | WLR_EDGE_LEFT);
        containerAddGaps(con, gap, WLR_EDGE_RIGHT | WLR_EDGE_BOTTOM);
}

void configure_gaps(int *innerGap, int *outerGap)
{
    /* innerGaps are applied twice because gaps don't overlap from two
     containers therefore it has to be divided by 2*/
    *innerGap /= 2;
    /* outerGap + innerGap = resultGap but we only want the outerGap */
    *outerGap -= *innerGap;
}
