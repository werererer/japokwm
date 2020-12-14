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
    renderDataType_t dataType;
    char *text;
    char *id;
    int x, y;
    struct monitor *mon;
    struct tagset *tagset;
    struct wlr_texture *texture;
};

/* should be called before wlr_begin_renderer() */
struct pos_texture *create_textbox(struct wlr_box box, float boxColor[4],
                                 float textColor[4], char* text);
/* same as createOverlay but clears renderData texture before */
void create_new_overlay();
/* creates overlay (that thing that covers the windows) */
void create_overlay();
void init_overlay();
void update_container_overlay(struct container *con);
void update_overlay();
void update_overlay_count(size_t count);
void write_overlay(struct monitor *m, const char *layout);
struct wlr_box postexture_to_container(struct pos_texture *pTexture);
bool postexture_visible_on_flag(struct pos_texture *pTexture, struct monitor *m, size_t focusedTag);

extern bool overlay;
#endif /* TILE_TEXTURE_H */
