#include "tile/tileTexture.h"
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
    Client *c, *sel = selClient();
    double ox, oy;
    int i, w, h;
    struct render_data rdata;
    struct wlr_surface *surface;
    struct wlr_box box = c->geom;

    /* our stacking list is ordered front-to-back, we iterate over it backwards. */
    wl_list_for_each_reverse(c, &stack, slink) {
        /* Only render visible clients which show on this monitor */
        if (!visibleon(c, c->mon))
            continue;

        surface = getWlrSurface(c);
        ox = c->geom.x, oy = c->geom.y;
        wlr_output_layout_output_coords(output_layout, m->wlr_output,
                &ox, &oy);
        };

        /* Draw window borders */
        color = (c == sel) ? focuscolor : bordercolor;
        for (i = 0; i < 4; i++) {
            scalebox(&borders[i], m->wlr_output->scale);
            wlr_render_rect(drw, &borders[i], color,
                    m->wlr_output->transform_matrix);
        }

        /* This calls our render function for each surface among the
         * xdg_surface's toplevel and popups. */

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        rdata.output = m->wlr_output;
        rdata.when = &now;
        rdata.x = c->geom.x + c->bw;
        rdata.y = c->geom.y + c->bw;

        wlr_surface_for_each_surface(getWlrSurface(c), render, &rdata);
    }
}
