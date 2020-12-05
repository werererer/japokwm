#ifndef RENDER_H
#define RENDER_H
#include "utils/coreUtils.h"
#include "client.h"
#include <wayland-util.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_box.h>
#include <wlr/util/edges.h>

typedef enum {
    OVERLAY,
    WORKSPACES,
} renderDataType_t;

struct renderData {
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
};

void render_frame(struct wl_listener *listener, void *data);
void scalebox(struct wlr_box *box, float scale);

extern struct wlr_renderer *drw;
extern struct renderData render_data;
#endif /* RENDER_H */
