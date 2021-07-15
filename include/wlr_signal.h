#ifndef WLR_SIGNAL
#define WLR_SIGNAL

#include <stdlib.h>
#include <glib.h>

struct wlr_signal {
    GPtrArray *listener_list;
};

#endif /* WLR_SIGNAL */
