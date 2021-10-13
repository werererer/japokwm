#ifndef TILEUTILS
#define TILEUTILS

#include "utils/coreUtils.h"

struct container;
struct tagset;
struct monitor;

struct client *focustop(struct monitor *m);
void arrange();
void arrange_monitor(struct monitor *m);
void arrange_containers(struct workspace *ws, struct wlr_box root_geom,
        GPtrArray *tiled_containers);
void container_update_size(struct container *con);
void update_hidden_status_of_containers(struct monitor *m, GPtrArray *tiled_containers);
int lib_get_this_container_count();
int get_container_count(struct workspace *ws);
int get_container_area_count(struct workspace *ws);
int get_tiled_container_count(struct workspace *ws);
int get_slave_container_count(struct workspace *ws);
int get_floating_container_count(struct workspace *ws);
int get_master_container_count(struct workspace *ws);

#endif
