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
struct posTexture *createTextbox(struct wlr_box box, float boxColor[4],
                                 float textColor[4], char* text);
/* same as createOverlay but clears renderData texture before */
void create_new_overlay();
/* creates overlay (that thing that covers the windows) */
void create_overlay();
void writeThisOverlay(char *layout);
void write_overlay(struct monitor *m, char *filename);

extern bool overlay;
void init_overlay();
void set_overlay(bool ol);
bool get_overlay();
#endif /* TILE_TEXTURE_H */
