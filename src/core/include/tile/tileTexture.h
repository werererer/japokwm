#ifndef TILE_TEXTURE_H
#define TILE_TEXTURE_H
/*
 * This helps creating things with cairo
 * */

#include <cairo/cairo.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_box.h>
#include <wlr/render/wlr_renderer.h>
#include "render/render.h"

/* should be called before wlr_begin_renderer() */
struct posTexture* createBox(struct wlr_box box, float color[static 4]);
struct posTexture* createTextbox(struct wlr_box box, float color[static 4],
                                  char* text);
void createOverlay();
#endif /* TILE_TEXTURE_H */
