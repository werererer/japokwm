#ifndef TILE_TEXTURE_H
#define TILE_TEXTURE_H
#include <cairo/cairo.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_box.h>
#include <wlr/render/wlr_renderer.h>
#include "render/render.h"

/* should be called before wlr_begin_renderer() */
struct wlr_texture* createBox(struct wlr_fbox box, float color[static 4]);
struct wlr_texture* createTextbox(struct wlr_fbox box, float color[static 4],
                                  char* text);
struct wl_list createOverlay(Monitor *m, float color[static 4]);
#endif /* TILE_TEXTURE_H */
