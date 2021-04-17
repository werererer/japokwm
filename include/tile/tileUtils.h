#ifndef TILEUTILS
#define TILEUTILS
#include "utils/coreUtils.h"
#include "container.h"
#include "monitor.h"

struct client *focustop(struct monitor *m);
void arrange();
void arrange_monitor(struct monitor *m);
void arrange_containers(struct workspace *ws, struct wlr_box root_geom,
        struct wlr_list *tiled_containers);
void resize(struct container *con, struct wlr_box geom);
void update_hidden_status_of_containers(struct monitor *m, 
        struct wlr_list *visible_container_lists, struct wlr_list *tiled_containers,
        struct wlr_list *hidden_containers);
int lib_get_this_container_count();
int get_container_count(struct workspace *ws);
int get_container_area_count(struct workspace *ws);
int get_tiled_container_count(struct workspace *ws);
int get_slave_container_count(struct workspace *ws);
int get_floating_container_count(struct workspace *ws);
int get_master_container_count(struct workspace *ws);

#endif
