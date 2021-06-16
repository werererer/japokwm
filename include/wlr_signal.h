#ifndef WLR_SIGNAL
#define WLR_SIGNAL

#include <stdlib.h>
#include <wlr/types/wlr_list.h>

struct wlr_signal {
    struct wlr_list listener_list;
};

#endif /* WLR_SIGNAL */
