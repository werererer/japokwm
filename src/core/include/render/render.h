#ifndef RENDER_H
#define RENDER_H
#include "coreUtils.h"
#include "client.h"
#include <wlr/types/wlr_matrix.h>

struct render_data {
    struct wlr_output *output;
    struct timespec *when;
    int x, y; /* layout-relative */
};

void render(struct wlr_surface *surface, int sx, int sy, void *data);
void renderClients(Monitor *m);
void renderFrame(struct wl_listener *listener, void *data);
void renderIndependents(struct wlr_output *output);
void renderTexture(Monitor *m, struct wlr_texture *texture);
void scalebox(struct wlr_box *box, float scale);

extern struct wlr_renderer *drw;
extern struct wl_list independents;
#endif /* RENDER_H */
