#include "render/render.h"
#include "client.h"
#include "tile/tileUtils.h"
#include "tile/tileTexture.h"
#include <stdio.h>
#include <wayland-util.h>
#include <wlr/util/edges.h>
#include "root.h"
#include "popup.h"

struct wlr_renderer *drw;
struct renderData renderData;

static void render(struct wlr_surface *surface, int sx, int sy, void *data);
static void renderClients(struct monitor *m);
static void renderIndependents(struct wlr_output *output);
static void renderTexture(void *texture);

static void render(struct wlr_surface *surface, int sx, int sy, void *data)
{
    /* This function is called for every surface that needs to be rendered. */
    struct renderData *rdata = data;
    struct wlr_output *output = rdata->output;
    double ox = 0, oy = 0;
    struct wlr_box obox;
    float matrix[9];
    enum wl_output_transform transform;

    /* We first obtain a wlr_texture, which is a GPU resource. wlroots
     * automatically handles negotiating these with the client. The underlying
     * resource could be an opaque handle passed from the client, or the client
     * could have sent a pixel buffer which we copied to the GPU, or a few other
     * means. You don't have to worry about this, wlroots takes care of it. */
    struct wlr_texture *texture = wlr_surface_get_texture(surface);
    if (!texture)
        return;

    /* The client has a position in layout coordinates. If you have two displays,
     * one next to the other, both 1080p, a client on the rightmost display might
     * have layout coordinates of 2000,100. We need to translate that to
     * output-local coordinates, or (2000 - 1920). */
    wlr_output_layout_output_coords(output_layout, output, &ox, &oy);

    /* We also have to apply the scale factor for HiDPI outputs. This is only
     * part of the puzzle, dwl does not fully support HiDPI. */
    // TODO: gaps here
    obox.x = ox + rdata->x + sx;
    obox.y = oy + rdata->y + sy;
    obox.width = surface->current.width;
    obox.height = surface->current.height;
    scalebox(&obox, output->scale);

    /*
     * Those familiar with OpenGL are also familiar with the role of matrices
     * in graphics programming. We need to prepare a matrix to render the
     * client with. wlr_matrix_project_box is a helper which takes a box with
     * a desired x, y coordinates, width and height, and an output geometry,
     * then prepares an orthographic projection and multiplies the necessary
     * transforms to produce a model-view-projection matrix.
     *
     * Naturally you can do this any way you like, for example to make a 3D
     * compositor.
     */
    transform = wlr_output_transform_invert(surface->current.transform);
    wlr_matrix_project_box(matrix, &obox, transform, 0,
        output->transform_matrix);

    /* This takes our matrix, the texture, and an alpha, and performs the actual
     * rendering on the GPU. */
    wlr_render_texture_with_matrix(drw, texture, matrix, 1);

    /* This lets the client know that we've displayed that frame and it can
     * prepare another one now if it likes. */
    wlr_surface_send_frame_done(surface, rdata->when);
}

static void renderClients(struct monitor *m)
{
    struct client *c, *sel = selected_client();
    const float *color;
    double ox, oy;
    int w, h;
    struct renderData rdata;
    struct wlr_box *borders;
    struct wlr_surface *surface;
    /* Each subsequent window we render is rendered on top of the last. Because
     * our stacking list is ordered front-to-back, we iterate over it backwards. */
    wl_list_for_each_reverse(c, &stack, slink) {
        /* Only render visible clients which are shown on this monitor */
        if (!visibleon(c, c->mon) || !wlr_output_layout_intersects(
                    output_layout, m->output, &c->geom))
            continue;

        surface = get_wlrsurface(c);
        ox = c->geom.x, oy = c->geom.y;
        wlr_output_layout_output_coords(output_layout, m->output,
                &ox, &oy);
        w = surface->current.width;
        h = surface->current.height;
        borders = (struct wlr_box[4]) {
            {ox, oy, w + 2 * c->bw, c->bw},             /* top */
                {ox, oy + c->bw, c->bw, h},                 /* left */
                {ox + c->bw + w, oy + c->bw, c->bw, h},     /* right */
                {ox, oy + c->bw + h, w + 2 * c->bw, c->bw}, /* bottom */
        };

        /* Draw window borders */
        color = (c == sel) ? focusColor : borderColor;
        for (int i = 0; i < 4; i++) {
            scalebox(&borders[i], m->output->scale);
            wlr_render_rect(drw, &borders[i], color,
                    m->output->transform_matrix);
        }

        /* This calls our render function for each surface among the
         * xdg_surface's toplevel and popups. */

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        rdata.output = m->output;
        rdata.when = &now;
        rdata.x = c->geom.x + c->bw;
        rdata.y = c->geom.y + c->bw;

        render(get_wlrsurface(c), 0, 0, &rdata);
    }
}

static void renderLayerShell(struct monitor *m, enum zwlr_layer_shell_v1_layer layer)
{
    struct client *c;
    struct renderData rdata; 
    /* Each subsequent window we render is rendered on top of the last. Because
     * our stacking list is ordered front-to-back, we iterate over it backwards. */
    wl_list_for_each_reverse(c, &layerstack, llink) {
        if (c->type != LAYER_SHELL)
            continue;
        if (c->surface.layer->current.layer != layer)
            continue;

        /* Only render visible clients which show on this monitor */
        if (!visibleon(c, c->mon) || !wlr_output_layout_intersects(
                    output_layout, m->output, &c->geom))
            continue;

        /* This calls our render function for each surface among the
         * xdg_surface's toplevel and popups. */

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        rdata.output = m->output;
        rdata.when = &now;
        rdata.x = c->geom.x + c->bw;
        rdata.y = c->geom.y + c->bw;

        render(get_wlrsurface(c), 0, 0, &rdata);
    }
}

/* will be called foreach texture in renderData */
static void renderTexture(void *texture)
{
    struct posTexture *text = texture;
    if (postexture_visible_on_flag(text, selected_monitor, selected_monitor->tagset->selTags[0])) {
        wlr_render_texture(drw, text->texture, selected_monitor->output->transform_matrix,
                text->x, text->y, 1);
    }
}


static void renderIndependents(struct wlr_output *output)
{
    struct client *c;
    struct renderData rdata;
    struct wlr_box geom;

    wl_list_for_each_reverse(c, &independents, link) {
        geom.x = c->surface.xwayland->x;
        geom.y = c->surface.xwayland->y;
        geom.width = c->surface.xwayland->width;
        geom.height = c->surface.xwayland->height;

        /* Only render visible clients which show on this output */
        if (!wlr_output_layout_intersects(output_layout, output, &geom))
            continue;

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        rdata.output = output;
        rdata.when = &now;
        rdata.x = c->surface.xwayland->x;
        rdata.y = c->surface.xwayland->y;

        wlr_surface_for_each_surface(c->surface.xwayland->surface, render, &rdata);
    }
}

static void renderPopups(struct monitor *m) {
    struct xdg_popup *popup;
    wl_list_for_each_reverse(popup, &popups, link) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        struct renderData rdata;
        rdata.output = m->output;
        rdata.when = &now;
        rdata.x = popup->geom.x;
        rdata.y = popup->geom.y;
        render(popup->xdg->base->surface, 0, 0, &rdata);
    }
}

/* called from a wl_signal in setup() */
void renderFrame(struct wl_listener *listener, void *data)
{
    bool render = true;

    /* This function is called every time an output is ready to display a frame,
     * generally at the output's refresh rate (e.g. 60Hz). */
    struct monitor *m = wl_container_of(listener, m, frame);

    if (selected_monitor) {
        struct wlr_box b = selected_monitor->m;
        selected_monitor->m = *wlr_output_layout_get_box(output_layout, selected_monitor->output);
        // resize clients
        if (selected_monitor->m.width != b.width || selected_monitor->m.height != b.height) {
            arrange(selected_monitor, false);
            struct client *c;
            wl_list_for_each(c, &layerstack, llink) {
                wlr_layer_surface_v1_configure(c->surface.layer,
                        selected_monitor->output->width, selected_monitor->output->height);
            }
        }
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    /* wlr_output_attach_render makes the OpenGL context current. */
    if (!wlr_output_attach_render(m->output, NULL))
        return;

    if (render) {
        /* Begin the renderer (calls glViewport and some other GL sanity checks) */
        wlr_renderer_begin(drw, m->output->width, m->output->height);
        wlr_renderer_clear(drw, root.color);

        renderLayerShell(m, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND);
        renderLayerShell(m, ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM);
        renderClients(m);
        renderIndependents(m->output);
        renderLayerShell(m, ZWLR_LAYER_SHELL_V1_LAYER_TOP);
        renderLayerShell(m, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY);

        wlr_list_for_each(&renderData.textures, renderTexture);
        renderPopups(m);

        /* Hardware cursors are rendered by the GPU on a separate plane, and can be
         * moved around without re-rendering what's beneath them - which is more
         * efficient. However, not all hardware supports hardware cursors. For this
         * reason, wlroots provides a software fallback, which we ask it to render
         * here. wlr_cursor handles configuring hardware vs software cursors for you,
         * and this function is a no-op when hardware cursors are in use. */
        wlr_output_render_software_cursors(m->output, NULL);

        /* Conclude rendering and swap the buffers, showing the final frame
         * on-screen. */
        wlr_renderer_end(drw);
    }

    wlr_output_commit(m->output);
}

void scalebox(struct wlr_box *box, float scale)
{
    box->x *= scale;
    box->y *= scale;
    box->width *= scale;
    box->height *= scale;
}
