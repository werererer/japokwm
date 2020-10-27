#include "tile/tileTexture.h"
#include <cairo/cairo.h>
#include <wayland-util.h>
#include <wlr/backend.h>

// TODO: rewrite getPosition
/* static struct wlr_box getPosition(struct posTexture *pTexture) */
/* { */
/*     struct wlr_box container; */
/*     container.x = pTexture->x; */
/*     container.y = pTexture->y; */
/*     container.width = pTexture->texture->width; */
/*     container.height = pTexture->texture->height; */
/*     return container; */
/* } */

struct posTexture *createTextbox(struct wlr_box box, float boxColor[],
                                 float textColor[], char* text)
{
    cairo_format_t cFormat = CAIRO_FORMAT_ARGB32;

    int width = box.width;
    int height = box.height;
    int stride = cairo_format_stride_for_width(cFormat, width);

    cairo_surface_t *surface =
        cairo_image_surface_create(cFormat, width, height);
    //draw box
    cairo_t *cr = cairo_create(surface);
    cairo_set_line_width(cr, 0.1);
    cairo_set_source_rgba(cr,
            boxColor[0], boxColor[1], boxColor[2], boxColor[3]);
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
    cairo_set_source_rgba(cr,
            textColor[0], textColor[1], textColor[2], textColor[3]);
    cairo_show_text(cr, text);
    cairo_surface_flush(surface);

    unsigned char *Cdata = cairo_image_surface_get_data(surface);

    struct wlr_texture *cTexture = 
        wlr_texture_from_pixels(drw, WL_SHM_FORMAT_ARGB8888, stride, 
                width, height, Cdata);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    struct posTexture *posTexture = calloc(1, sizeof(*posTexture));

    posTexture->texture = cTexture;
    posTexture->dataType = OVERLAY;
    posTexture->x = box.x;
    posTexture->y = box.y;
    return posTexture;
}

void createNewOverlay()
{
    // recreate list
    wlr_list_clear(&renderData.textures);
    createOverlay();
}

void createOverlay()
{
    struct client *c;

    int i = 1;
    char text[NUM_DIGITS];
    wl_list_for_each(c, &clients, link) {
        if (!visibleon(c, c->mon))
            continue;

        sprintf(text, "%i", i);

        wlr_list_push(&renderData.textures, 
                createTextbox(c->geom, overlayColor, textColor, text));
        i++;
    }
}

void writeOverlay(char *file)
{
    // TODO wite logic
    /* struct posTexture *pTexture; */
    /* struct wlr_list *listPtr,list = renderData.textures; */
    /* listPtr = &list; */

    /* writeContainerArrayToFile(file, listPtr, length); */
}
