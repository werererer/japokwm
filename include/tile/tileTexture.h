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
struct posTexture *create_textbox(struct wlr_box box, float boxColor[4],
                                 float textColor[4], char* text);
/* same as createOverlay but clears renderData texture before */
void create_new_overlay();
/* creates overlay (that thing that covers the windows) */
void create_overlay();
void init_overlay();
void update_client_overlay(struct client *c);
void update_overlay();
void update_overlay_count(size_t count);
void write_overlay(struct monitor *m, const char *layout);

extern bool overlay;
#endif /* TILE_TEXTURE_H */
