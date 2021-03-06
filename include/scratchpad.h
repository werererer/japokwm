#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

#include <wayland-server.h>

#include "container.h"

void move_to_scratchpad(struct container *con, int position);
void remove_container_from_scratchpad(struct container *con);
void show_scratchpad();
#endif /* SCRATCHPAD_H */
