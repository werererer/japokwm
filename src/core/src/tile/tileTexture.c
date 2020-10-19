#include "tile/tileTexture.h"
#include "render/render.h"
#include <wayland-util.h>
#include <wlr/backend.h>

struct wlr_texture* createBox(struct wlr_fbox box, float color[static 4])
{
    cairo_format_t cFormat = CAIRO_FORMAT_ARGB32;

    int width = 300;
    int height = 300;
    int stride = cairo_format_stride_for_width(cFormat, width);

    cairo_surface_t *surface =
        cairo_image_surface_create(cFormat, width, height);
    cairo_t *cr = cairo_create(surface);
    cairo_set_line_width(cr, 0.1);
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.5);
    cairo_rectangle(cr, 0, 0, 100, 100);
    cairo_fill(cr);
    cairo_surface_flush(surface);

    unsigned char *Cdata = cairo_image_surface_get_data(surface);

    struct wlr_texture *cTexture = wlr_texture_from_pixels(drw, WL_SHM_FORMAT_ARGB8888, stride, width, height, Cdata);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return cTexture;
}

struct wlr_texture* createTextbox(struct wlr_fbox box, float color[static 4],
                                  char* text)
{
    cairo_format_t cFormat = CAIRO_FORMAT_ARGB32;

    int width = 300;
    int height = 300;
    int stride = cairo_format_stride_for_width(cFormat, width);

    cairo_surface_t *surface =
        cairo_image_surface_create(cFormat, width, height);
    cairo_t *cr = cairo_create(surface);
    cairo_set_line_width(cr, 0.1);
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.5);
    cairo_rectangle(cr, 0, 0, 100, 100);
    cairo_fill(cr);
    cairo_surface_flush(surface);

    unsigned char *Cdata = cairo_image_surface_get_data(surface);

    struct wlr_texture *cTexture =
        wlr_texture_from_pixels(drw, WL_SHM_FORMAT_ARGB8888, stride,
                                width, height, Cdata);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return cTexture;
}

struct wl_list createOverlay(Monitor *m, float color[])
{
    Client *c;
    struct wlr_fbox box = c->geom;

    /* our stacking list is ordered front-to-back, we iterate over it backwards. */
    wl_list_for_each_reverse(c, &stack, slink) {
        /* Only render visible clients which show on this monitor */
        if (!visibleon(c, c->mon))
            continue;

        /* Draw window borders */
        struct wlr_texture *texture = createBox(box, color);

        renderTexture(m, texture);
    }
}
