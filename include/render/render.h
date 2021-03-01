#ifndef RENDER_H
#define RENDER_H
#include "utils/coreUtils.h"
#include "client.h"
#include "monitor.h"
#include <wayland-util.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_box.h>
#include <wlr/util/edges.h>

struct render_data {
    struct wlr_output *output;
    struct timespec *when;
    int x, y; /* layout-relative */
    /* textures that will be rendered with a new frame
     * list should be filled with posTexture
     * */
    struct wlr_list textures;
    /* The textures before doing any kind of transformations
     * */
    struct wlr_list base_textures;
    pixman_region32_t *damage;
};

typedef void (*surface_iterator_func_t)(struct monitor *m, struct
        wlr_surface *surface, struct wlr_box *box, void *user_data);

struct surface_iterator_data {
    surface_iterator_func_t user_iterator;
    void *user_data;

    struct monitor *m;

    /* Output-local coordinates. */
    double ox, oy;
};

void render_monitor(struct monitor *m, pixman_region32_t *damage);
void scale_box(struct wlr_box *box, float scale);
void output_damage_surface(struct monitor *m, struct wlr_surface *surface,
        struct wlr_box *geom, bool whole);

extern struct wlr_renderer *drw;
extern struct render_data render_data;
#endif /* RENDER_H */
