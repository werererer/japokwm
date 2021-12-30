#ifndef RENDER_H
#define RENDER_H
#include "utils/coreUtils.h"
#include "client.h"
#include "output.h"
#include <wayland-util.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_box.h>
#include <wlr/util/edges.h>

typedef void (*surface_iterator_func_t)(struct output *m, struct
        wlr_surface *surface, struct wlr_box *box, void *user_data);

struct surface_iterator_data {
    surface_iterator_func_t user_iterator;
    void *user_data;

    struct output *m;

    /* Output-local coordinates. */
    struct wlr_box render_box;
};

struct render_texture_data {
    pixman_region32_t *output_damage;
    float alpha;
};

void render_monitor(struct output *m, pixman_region32_t *damage);
void scale_box(struct wlr_box *box, float scale);
void output_damage_surface(struct output *m, struct wlr_surface *surface,
        struct wlr_box *geom, bool whole);

extern struct wlr_renderer *drw;
#endif /* RENDER_H */
