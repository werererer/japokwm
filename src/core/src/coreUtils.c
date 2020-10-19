#include "coreUtils.h"
struct wlr_seat *seat;

void wlr_list_clear(struct wlr_list *list)
{
    wlr_list_finish(list);
    wlr_list_init(list);
}
