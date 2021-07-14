#ifndef LAYER_SHELL_H
#define LAYER_SHELL_H

#include <wayland-server.h>
#include "client.h"
#include "monitor.h"

typedef struct {
    struct wlr_layer_surface_v1 *layer_surface;
    struct monitor *m;

    struct wl_listener destroy;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener surface_commit;

    struct wlr_box geom;
    bool hidden;

    enum zwlr_layer_shell_v1_layer layer;
} LayerSurface;

struct edge {
    uint32_t singular_anchor;
    uint32_t anchor_triplet;
    int *positive_axis;
    int *negative_axis;
    int margin;
};

struct wlr_surface *layer_surface_get_wlr_surface(LayerSurface *layer_surface);

void damage_layer_shell_area(LayerSurface *layer_surface, struct wlr_box *geom, bool whole);
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
