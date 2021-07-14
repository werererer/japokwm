#ifndef LAYER_SHELL_H
#define LAYER_SHELL_H

#include <wayland-server.h>
#include "client.h"
#include "monitor.h"

struct edge {
    uint32_t singular_anchor;
    uint32_t anchor_triplet;
    int *positive_axis;
    int *negative_axis;
    int margin;
};

void maplayersurfacenotify(struct wl_listener *listener, void *data);
void unmaplayersurfacenotify(struct wl_listener *listener, void *data);
void destroylayersurfacenotify(struct wl_listener *listener, void *data);
void commitlayersurfacenotify(struct wl_listener *listener, void *data);
void create_notify_layer_shell(struct wl_listener *listener, void *data);
void arrangelayers(struct monitor *m);
void arrangelayer(struct monitor *m, struct wlr_list *list, struct wlr_box *usable_area, int exclusive);
void apply_exclusive(struct wlr_box *usable_area,
        uint32_t anchor, int32_t exclusive,
        int32_t margin_top, int32_t margin_right,
        int32_t margin_bottom, int32_t margin_left);
struct wlr_list *get_layer_list(enum zwlr_layer_shell_v1_layer layer);

#endif /* LAYER_SHELL_H */
