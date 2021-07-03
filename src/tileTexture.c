#include "tileTexture.h"

#include <cairo/cairo.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/util/log.h>
#include <stdlib.h>
#include <libdrm/drm_fourcc.h>
#include <pango/pangocairo.h>

#include "client.h"
#include "container.h"
#include "monitor.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "utils/stringUtils.h"
#include "utils/writeFile.h"
#include "tile/tileUtils.h"
#include "root.h"

static cairo_t* create_layout_context()
{
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
            0, 0);
    cairo_t *context = cairo_create (surface);
    cairo_surface_destroy(surface);
    return context;
}

struct pos_texture *create_textbox(struct wlr_box *box, float box_color[4],
                                 float text_color[4], const char* msg)
{
    cairo_t *layout_context = create_layout_context();
    /* Create a PangoLayout, set the font and text */
    PangoLayout *layout = pango_cairo_create_layout (layout_context);
    cairo_destroy(layout_context);

    pango_layout_set_width(layout, box->width*PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD);

    pango_layout_set_text (layout, msg, -1);
    PangoFontDescription *desc = pango_font_description_from_string ("12");
    pango_layout_set_font_description (layout, desc);
    pango_font_description_free (desc);

    int text_width, text_height;
    pango_layout_get_size(layout, &text_width, &text_height);
    text_width /= PANGO_SCALE;
    text_height /= PANGO_SCALE;

    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, text_width);
    cairo_surface_t *render_surface = cairo_image_surface_create_for_data(NULL, CAIRO_FORMAT_ARGB32, text_width, text_height, stride);
    cairo_t *render_context = cairo_create(render_surface);

    cairo_set_source_rgba(render_context,
            box_color[0], box_color[1], box_color[2], box_color[3]);
    cairo_rectangle(render_context, 0, 0, text_width, text_height);
    cairo_fill(render_context);

    cairo_set_source_rgba(render_context, text_color[0], text_color[1], text_color[2], text_color[3]);
    pango_cairo_show_layout(render_context, layout);

    unsigned char *cdata = cairo_image_surface_get_data(render_surface);

    struct wlr_texture *cTexture = 
        wlr_texture_from_pixels(drw, DRM_FORMAT_ARGB8888, stride, 
                text_width, text_height, cdata);

    /* free the layout object */
    g_object_unref (layout);
    cairo_destroy (render_context);
    cairo_surface_destroy(render_surface);

    struct pos_texture *posTexture = calloc(1, sizeof(struct pos_texture));

    posTexture->texture = cTexture;
    posTexture->x = box->x;
    posTexture->y = box->y;
    posTexture->m = selected_monitor;

    box->height = text_height;
    return posTexture;
}

void create_test_box(const char *msg)
{
    float color[4] = {1.0, 0.0, 0.0, 1.0};
    float text_color[4] = {1.0, 1.0, 1.0, 1.0};
    struct wlr_box geom = {2, 0, 100, 100};
    struct pos_texture *ptexture =
        create_textbox(&geom, color, text_color, msg);
    wlr_list_push(&render_data.textures, ptexture);
}

static bool postexture_contains_client(struct tagset *ts, struct pos_texture *ptexture)
{
    BitSet bitset;
    bitset_setup(&bitset, server.workspaces.length);
    workspace_id_to_tag(&bitset, ptexture->ws_id);
    bitset_and(&bitset, &ts->workspaces);
    return bitset_any(&bitset);
}

bool postexture_visible_on(struct pos_texture *ptexture, struct monitor *m, struct tagset *tagset)
{
    if (!m || !ptexture)
        return false;
    if (ptexture->m != m)
        return false;

    return postexture_contains_client(tagset, ptexture);
}
