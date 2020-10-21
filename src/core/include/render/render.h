#ifndef RENDER_H
#define RENDER_H
#include "utils/coreUtils.h"
#include "client.h"
#include <wayland-util.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/render/wlr_texture.h>

struct renderData {
    struct wlr_output *output;
    struct timespec *when;
    int x, y; /* layout-relative */
    struct wlr_list textures;
};

/* a texture at a given position */
struct posTexture {
    int x, y;
    struct wlr_texture *texture;
};

void renderFrame(struct wl_listener *listener, void *data);
void scalebox(struct wlr_box *box, float scale);

extern struct wlr_renderer *drw;
extern struct wl_list independents;
extern struct renderData renderData;
#endif /* RENDER_H */
