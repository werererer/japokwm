#include "utils/gapUtils.h"

static void containerAddGapLeft(Container *con, float gap)
{
    con->x += gap;
}

static void containerAddGapRight(Container *con, float gap)
{
    con->width -= gap;
}

static void containerAddGapTop(Container *con, float gap)
{
    con->y += gap;
}

static void containerAddGapBottom(Container *con, float gap)
{
    con->height -= gap;
}

void containerAddGaps(Container *con, double gap, enum wlr_edges edges) {
    if (edges & WLR_EDGE_LEFT)
        containerAddGapLeft(con, gap);
    if (edges & WLR_EDGE_RIGHT)
        containerAddGapRight(con, gap);
    if (edges & WLR_EDGE_TOP)
        containerAddGapTop(con, gap);
    if (edges & WLR_EDGE_BOTTOM)
        containerAddGapBottom(con, gap);
}

