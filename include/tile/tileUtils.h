#ifndef TILEUTILS
#define TILEUTILS

#include "utils/coreUtils.h"

struct container;
struct tagset;
struct monitor;

struct client *focustop(struct monitor *m);
void arrange();
void arrange_monitor(struct monitor *m);
void arrange_containers(struct tagset *tagset, struct wlr_box root_geom,
        GPtrArray *tiled_containers);
void resize(struct container *con, struct wlr_box geom);
void update_hidden_status_of_containers(struct monitor *m, 
        GPtrArray2D *visible_container_lists, GPtrArray *tiled_containers);
int lib_get_this_container_count();
int get_container_count(struct tagset *tagset);
int get_container_area_count(struct tagset *tagset);
int get_tiled_container_count(struct tagset *tagset);
int get_slave_container_count(struct tagset *tagset);
int get_floating_container_count(struct tagset *tagset);
int get_master_container_count(struct tagset *tagset);

#endif
