#include "tile/tileTexture.h"
#include <cairo/cairo.h>
#include <wayland-util.h>
#include <wlr/backend.h>

struct posTexture* createTextbox(struct wlr_box box, float boxColor[], 
                                 float textColor[], char* text)
{
    cairo_format_t cFormat = CAIRO_FORMAT_ARGB32;

    int width = box.width;
    int height = box.height;
    int stride = cairo_format_stride_for_width(cFormat, width);

    cairo_surface_t *surface =
        cairo_image_surface_create(cFormat, width, height);
    //draw rect
    cairo_t *cr = cairo_create(surface);
    cairo_set_line_width(cr, 0.1);
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.5);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
    cairo_surface_flush(surface);

    //write text
    cairo_select_font_face(cr, "serif", 
            CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 32.0);
    cairo_move_to(cr, width/2, height/2);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_show_text(cr, text);
    cairo_surface_flush(surface);

    unsigned char *Cdata = cairo_image_surface_get_data(surface);

    struct wlr_texture *cTexture = 
        wlr_texture_from_pixels(drw, WL_SHM_FORMAT_ARGB8888, stride, 
                width, height, Cdata);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    struct posTexture *posTexture = malloc(sizeof(posTexture));

    posTexture->texture = cTexture;
    posTexture->x = box.x;
    posTexture->y = box.y;
    return posTexture;
}

void createNewOverlay()
{
    // recreate list
    wlr_list_finish(&renderData.textures);
    wlr_list_init(&renderData.textures);

    createOverlay();
}

void createOverlay()
{
    float color[4] = {1, 1, 0, 0.5};
    float color2[4] = {1, 1, 0, 0.5};
    Client *c;

    int i = 1;
    char text[6];
    /* our stacking list is ordered front-to-back, we iterate over it backwards. */
    wl_list_for_each(c, &stack, slink) {
        if (!visibleon(c, c->mon))
            continue;

        sprintf(text, "%i", i);
        wlr_list_push(&renderData.textures,
                createTextbox(c->geom, color, color2, text));
        i++;
    }
}
