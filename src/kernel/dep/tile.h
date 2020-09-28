#ifndef TILE_H
#define TILE_H
#include "coreUtils.h"
#include "tileUtils.h"
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_xdg_shell.h>

/*
 * This file is used by julia to tile windows
*/

void monocle(Monitor *m);
void tile(Monitor *m);
void resize(Client *c, int x, int y, int w, int h, int interact);

#endif /* TILE_H */
