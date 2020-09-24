#ifndef TILE
#define TILE
#include "coreUtils.h"
#include "tileUtils.h"
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_xdg_shell.h>

void monocle(Monitor *m);
void tile(Monitor *m);
void resize(Client *c, int x, int y, int w, int h, int interact);

#endif
