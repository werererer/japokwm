#ifndef TILEUTILS
#define TILEUTILS
#include "utils/coreUtils.h"
#include "container.h"
#include "monitor.h"

enum layout_actions {
    LAYOUT_NOOP,
    LAYOUT_RESET,
};

struct client *focustop(struct monitor *m);
void arrange(enum layout_actions action);
void arrange_monitor(struct monitor *m, enum layout_actions action);
void arrange_container(struct container *con, int count, bool preserve);
void resize(struct container *con, struct wlr_box geom, bool preserve_geometry);
void update_hidden_containers(struct monitor *m);
int this_tiled_client_count();
int tiled_container_count(struct monitor *m);
#endif
