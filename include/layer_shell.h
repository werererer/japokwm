#ifndef LAYER_SHELL_H
#define LAYER_SHELL_H

#include <wayland-server.h>
#include "client.h"

void create_notify_layer_shell(struct wl_listener *listener, void *data);

#endif /* LAYER_SHELL_H */
