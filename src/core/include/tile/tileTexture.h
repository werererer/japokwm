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
#include "render/render.h"
#include "utils/writeFile.h"

/* should be called before wlr_begin_renderer() */
struct posTexture* createTextbox(struct wlr_box box, float boxColor[4],
                                 float textColor[4], char* text);
/* same as createOverlay but clears renderData texture before */
void createNewOverlay();
/* creates overlay (that thing that covers the windows) */
void createOverlay();
void writeOverlay(char *file);
#endif /* TILE_TEXTURE_H */
