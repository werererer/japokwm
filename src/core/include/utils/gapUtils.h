#ifndef GAP_UTILS_H
#define GAP_UTILS_H

#include <wlr/util/edges.h>
#include "utils/coreUtils.h"

void containerAddGaps(Container *con, double gap, enum wlr_edges edges);
void containerSurroundGaps(Container *con, double gap);

#endif /* GAP_UTILS_H */
