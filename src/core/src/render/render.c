#include "render/render.h"
#include "tile/tileUtils.h"
#include "tile/tileTexture.h"

struct wlr_renderer *drw;
struct wl_list independents;

static void render(struct wlr_surface *surface, int sx, int sy, void *data);
static void renderClient(Monitor *m, Client *c, float *borderColor);
static void renderIndependents(struct wlr_output *output);
static void renderTexture(Monitor *m, struct wlr_texture *texture);

static void render(struct wlr_surface *surface, int sx, int sy, void *data)
{
    /* This function is called for every surface that needs to be rendered. */
    struct render_data *rdata = data;
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

static void renderClients(Monitor *m)
{
    Client *c, *sel = selClient();
    const float *color;
    double ox, oy;
    int i, w, h;
    struct render_data rdata;
    struct wlr_box *borders;
    struct wlr_surface *surface;
    /* Each subsequent window we render is rendered on top of the last. Because
     * k
     * our stacking list is ordered front-to-back, we iterate over it backwards. */
    wl_list_for_each_reverse(c, &stack, slink) {
        /* Only render visible clients which show on this monitor */
        if (!visibleon(c, c->mon) || !wlr_output_layout_intersects(
                    output_layout, m->wlr_output, &c->geom))
            continue;

        surface = getWlrSurface(c);
        ox = c->geom.x, oy = c->geom.y;
        wlr_output_layout_output_coords(output_layout, m->wlr_output,
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
        switch (c->type) {
            case XDGShell:
                wlr_xdg_surface_for_each_surface(c->surface.xdg, render, &rdata);
                break;
            case LayerShell:
                wlr_layer_surface_v1_for_each_surface(c->surface.layer, render, &rdata);
                break;
            case X11Managed:
            case X11Unmanaged:
                wlr_surface_for_each_surface(c->surface.xwayland->surface, render, &rdata);
                break;
        }
    }
}

static void renderTexture(Monitor *m, struct wlr_texture *texture)
{
    wlr_render_texture(drw, texture, m->wlr_output->transform_matrix, 
                       0, 0, 1);
}

static void renderIndependents(struct wlr_output *output)
{
    Client *c;
    struct render_data rdata;
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

void renderFrame(struct wl_listener *listener, void *data)
{
    struct wlr_fbox box = {0, 0, 1, 1};
    float color[4] = {0, 1, 1, 0.5};
    struct wlr_texture *cTexture = createBox(box, color);
    Client *c;
    int render = 1;

    /* This function is called every time an output is ready to display a frame,
     * generally at the output's refresh rate (e.g. 60Hz). */
    Monitor *m = wl_container_of(listener, m, frame);

    if (selMon) {
        struct wlr_box b = selMon->m;
        selMon->m = *wlr_output_layout_get_box(output_layout, selMon->wlr_output);
        if (selMon->m.width != b.width || selMon->m.height != b.height) {
            arrange(selMon, false);
        }
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wl_list_for_each(c, &stack, slink) {
        if (c->resize) {
            wlr_surface_send_frame_done(getWlrSurface(c), &now);
            render = 0;
        }
    }

    /* wlr_output_attach_render makes the OpenGL context current. */
    if (!wlr_output_attach_render(m->wlr_output, NULL))
        return;

    if (render) {
        /* Begin the renderer (calls glViewport and some other GL sanity checks) */
        wlr_renderer_begin(drw, m->wlr_output->width, m->wlr_output->height);
        wlr_renderer_clear(drw, rootcolor);

        renderClients(m);
        renderIndependents(m->wlr_output);
        renderTexture(m, cTexture);

        /* Hardware cursors are rendered by the GPU on a separate plane, and can be
         * moved around without re-rendering what's beneath them - which is more
         * efficient. However, not all hardware supports hardware cursors. For this
         * reason, wlroots provides a software fallback, which we ask it to render
         * here. wlr_cursor handles configuring hardware vs software cursors for you,
         * and this function is a no-op when hardware cursors are in use. */
        wlr_output_render_software_cursors(m->wlr_output, NULL);

        /* Conclude rendering and swap the buffers, showing the final frame
         * on-screen. */
        wlr_renderer_end(drw);
    }

    wlr_output_commit(m->wlr_output);
}

void scalebox(struct wlr_box *box, float scale)
{
    box->x *= scale;
    box->y *= scale;
    box->width *= scale;
    box->height *= scale;
}
