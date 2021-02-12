#ifndef TILEUTILS
#define TILEUTILS
#include "utils/coreUtils.h"
#include "container.h"
#include "monitor.h"

struct client *focustop(struct monitor *m);
void arrange();
void arrange_monitor(struct monitor *m);
void arrange_container(struct container *con, int arrange_position, int count);
void resize(struct container *con, struct wlr_box geom);
void update_hidden_containers(struct monitor *m);
int lib_get_this_container_count();
int get_tiled_container_count(struct monitor *m);
int get_slave_container_count(struct monitor *m);
int get_master_container_count(struct monitor *m);

#endif
