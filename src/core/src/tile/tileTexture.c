#include "tile/tileTexture.h"
#include <cairo/cairo.h>
#include <wayland-util.h>
#include <wlr/backend.h>

static struct wlr_box getPosition(struct posTexture *pTexture)
{
    return NULL;
}

struct posTexture* createTextbox(struct wlr_box box, float boxColor[],
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

    struct posTexture *posTexture = malloc(sizeof(posTexture));

    posTexture->texture = cTexture;
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
    Client *c;

    int i = 1;
    // TODO This number shouldn't be hard coded
    char text[NUM_DIGITS];
    wl_list_for_each(c, &stack, slink) {
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
    Client *c;
    struct wlr_list *list = &renderData.textures;
    wlr_list_for_each(c, &stack, slink) {
        struct wlr_box box = getPosition(wlr_list_peek(list));
        writeArrayToFile(file, arr, 0, 0);
    }
}
