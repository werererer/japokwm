#ifndef XDG_SHELL_H
#define XDG_SHELL_H

#include <wayland-server-core.h>

typedef struct {
    struct wl_listener request_mode;
    struct wl_listener destroy;
} Decoration;

void create_notify_xdg(struct wl_listener *listener, void *data);
void destroy_notify(struct wl_listener *listener, void *data);
void map_request(struct wl_listener *listener, void *data);
void unmap_notify(struct wl_listener *listener, void *data);
void createxdeco(struct wl_listener *listener, void *data);

#endif /* XDG_SHELL_H */
