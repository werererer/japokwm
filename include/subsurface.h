#ifndef SUBSURFACE_H
#define SUBSURFACE_H

#include <wayland-server-core.h>

struct subsurface {
    struct container *parent;
    struct wlr_subsurface *wlr_subsurface;

    struct wl_listener commit;
    struct wl_listener destroy;
};

void handle_new_subsurface(struct wl_listener *listener, void *data);

#endif /* SUBSURFACE_H */
