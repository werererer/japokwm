#ifndef GAP_UTILS_H
#define GAP_UTILS_H

#include <wlr/util/edges.h>
#include "utils/coreUtils.h"

void containerAddGaps(struct wlr_box *con, double gap, enum wlr_edges edges);
void containerSurroundGaps(struct wlr_box *con, double gap);
/* correctly configure gaps so that they work as expected */
void configure_gaps(int *innerGap, int *outerGap);

#endif /* GAP_UTILS_H */
