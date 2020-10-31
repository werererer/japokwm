#include "utils/gapUtils.h"

static void containerAddGapLeft(Container *con, float gap)
{
    con->x += gap;
}

static void containerAddGapRight(Container *con, float gap)
{
    con->width = MAX(1, con->width - gap);
}

static void containerAddGapTop(Container *con, float gap)
{
    con->y += gap;
}

static void containerAddGapBottom(Container *con, float gap)
{
    con->height = MAX(1, con->height - gap);
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

void containerSurroundGaps(Container *con, double gap)
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
     * therefore x and y need to be 1/2ed and the width has to be decreased by
     * the whole amound
     * */
    containerAddGaps(con, gap/2, WLR_EDGE_TOP | WLR_EDGE_LEFT);
    containerAddGaps(con, gap, WLR_EDGE_RIGHT | WLR_EDGE_BOTTOM);
}
