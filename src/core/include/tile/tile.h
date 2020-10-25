#ifndef TILE_H
#define TILE_H
#include "tileUtils.h"
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_xdg_shell.h>

/*
 * This file is used by julia to tile windows
*/

//relative position
void create(Monitor *m);
void tile(Monitor *m);
void addClient(struct client *c, int x1, int y1, int x2, int y2, Monitor *m);

jl_value_t* getWlrBox(struct wlr_box w);

#endif /* TILE_H */
