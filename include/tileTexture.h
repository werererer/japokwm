#ifndef TILE_TEXTURE_H
#define TILE_TEXTURE_H
/*
 * This helps creating things with cairo
 * TODO: better name?
 * */
#include <cairo/cairo.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_box.h>
#include <wlr/render/wlr_renderer.h>
#include "utils/writeFile.h"
#include "container.h"


/* a texture at a given position */
struct pos_texture {
    char *text;
    char *id;
    int x, y;
    struct monitor *m;
    struct wlr_texture *texture;
    int ws_id;
};

/* should be called before wlr_begin_renderer() */
struct pos_texture *create_textbox(struct wlr_box *box, float box_color[4],
                                 float text_color[4], const char* msg);
void create_test_box(const char *msg);
bool postexture_visible_on(struct pos_texture *ptexture, struct monitor *m, struct tagset *tagset);
#endif /* TILE_TEXTURE_H */
