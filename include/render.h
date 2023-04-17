#ifndef RENDER_H
#define RENDER_H

#include <wlr/types/wlr_scene.h>

#define BORDER_COUNT 4

struct scene_surface {
    struct wlr_surface *wlr;
    struct wlr_scene_tree *surface_tree;
    struct wlr_scene_surface *scene_surface;
    struct wlr_scene_rect *borders[4];
    struct wl_list link;

    struct wl_listener commit;
    struct wl_listener destroy;
};

void scene_create(struct wlr_scene **scene);
void server_handle_new_surface(struct wl_listener *listener, void *data);

struct client *wlr_surface_get_client(struct wlr_surface *wlr_surface);

#endif /* RENDER_H */
