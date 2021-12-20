#ifndef LAYER_SHELL_H
#define LAYER_SHELL_H

#include <wayland-server.h>
#include "client.h"
#include "monitor.h"
#include "tag.h"

struct edge {
    uint32_t singular_anchor;
    uint32_t anchor_triplet;
    int *positive_axis;
    int *negative_axis;
    int margin;
};

void map_layer_surface_notify(struct wl_listener *listener, void *data);
void unmap_layer_surface_notify(struct wl_listener *listener, void *data);
void destroy_layer_surface_notify(struct wl_listener *listener, void *data);
void commitlayersurfacenotify(struct wl_listener *listener, void *data);
void create_notify_layer_shell(struct wl_listener *listener, void *data);
void arrange_layers(struct monitor *m);
void arrangelayer(struct monitor *m, GPtrArray *array, struct wlr_box *usable_area, bool exclusive);
void apply_exclusive(struct wlr_box *usable_area,
        uint32_t anchor, int32_t exclusive,
        int32_t margin_top, int32_t margin_right,
        int32_t margin_bottom, int32_t margin_left);

bool layer_shell_is_bar(struct container *con);

GPtrArray *get_layer_list(struct monitor *m, enum zwlr_layer_shell_v1_layer layer);

#endif /* LAYER_SHELL_H */
