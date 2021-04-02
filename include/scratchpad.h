#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

#include <wayland-server.h>

#include "container.h"

void move_to_scratchpad(struct container *con, int position);
void show_scratchpad();

extern struct wl_list scratchpad;
#endif /* SCRATCHPAD_H */
