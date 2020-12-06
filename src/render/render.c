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
struct renderData render_data;

static void render(struct wlr_surface *surface, int sx, int sy, void *data);
static void render_clients(struct monitor *m);
static void render_independents(struct wlr_output *output);
static void render_texture(struct posTexture *texture);

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

static void render_clients(struct monitor *m)
{
    struct container *con, *sel = selected_container(m);

    /* Each subsequent window we render is rendered on top of the last. Because
     * our stacking list is ordered front-to-back, we iterate over it backwards. */
    wl_list_for_each_reverse(con, &m->stack, slink) {
        /* Only render visible clients which are shown on this monitor */
        if (!visibleon(con, m))
            continue;

        double ox, oy;
        int w, h;
        struct wlr_surface *surface = get_wlrsurface(con->client);
        ox = con->geom.x - con->client->bw;
        oy = con->geom.y - con->client->bw;
        wlr_output_layout_output_coords(output_layout, m->wlr_output, &ox, &oy);
        w = surface->current.width;
        h = surface->current.height;

        struct wlr_box *borders;
        borders = (struct wlr_box[4]) {
            {ox, oy, w + 2 * con->client->bw, con->client->bw},             /* top */
                {ox, oy + con->client->bw, con->client->bw, h},                 /* left */
                {ox + con->client->bw + w, oy + con->client->bw, con->client->bw, h},     /* right */
                {ox, oy + con->client->bw + h, w + 2 * con->client->bw, con->client->bw}, /* bottom */
        };

        /* Draw window borders */
        const float *color = (con == sel) ? focusColor : borderColor;
        for (int i = 0; i < 4; i++) {
            scalebox(&borders[i], m->wlr_output->scale);
            wlr_render_rect(drw, &borders[i], color,
                    m->wlr_output->transform_matrix);
        }

        /* This calls our render function for each surface among the
         * xdg_surface's toplevel and popups. */

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        struct renderData rdata;
        rdata.output = m->wlr_output;
        rdata.when = &now;
        rdata.x = con->geom.x;
        rdata.y = con->geom.y;

        render(surface, 0, 0, &rdata);
    }
}

static void render_layershell(struct monitor *m, enum zwlr_layer_shell_v1_layer layer)
{
    struct container *con;
    struct renderData rdata;
    /* Each subsequent window we render is rendered on top of the last. Because
     * our stacking list is ordered front-to-back, we iterate over it backwards. */
    wl_list_for_each_reverse(con, &m->layer_stack, llink) {
        if (con->client->type != LAYER_SHELL)
            continue;
        if (con->client->surface.layer->current.layer != layer)
            continue;

        /* Only render visible clients which are shown on this monitor */
        if (!visibleon(con, m) || !wlr_output_layout_intersects(
                    output_layout, m->wlr_output, &con->geom))
            continue;

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        rdata.output = m->wlr_output;
        rdata.when = &now;
        rdata.x = con->geom.x + con->client->bw;
        rdata.y = con->geom.y + con->client->bw;

        render(get_wlrsurface(con->client), 0, 0, &rdata);
    }
}

static void render_texture(struct posTexture *texture)
{
    if (postexture_visible_on_flag(
                texture,
                selected_monitor,
                selected_monitor->tagset->selTags[0])) {
        wlr_render_texture(drw, texture->texture,
                selected_monitor->wlr_output->transform_matrix, texture->x,
                texture->y, 1);
    }
}


static void render_independents(struct wlr_output *output)
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

static void render_popups(struct monitor *m) {
    struct xdg_popup *popup;
    wl_list_for_each_reverse(popup, &popups, link) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        struct renderData rdata;
        rdata.output = m->wlr_output;
        rdata.when = &now;
        rdata.x = popup->geom.x;
        rdata.y = popup->geom.y;
        render(popup->xdg->base->surface, 0, 0, &rdata);
    }
}

void render_frame(struct wl_listener *listener, void *data)
{
    bool render = true;

    /* This function is called every time an output is ready to display a frame,
     * generally at the output's refresh rate (e.g. 60Hz). */
    struct monitor *m = wl_container_of(listener, m, frame);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    /* wlr_output_attach_render makes the OpenGL context current. */
    if (!wlr_output_attach_render(m->wlr_output, NULL))
        return;

    if (render) {
        /* Begin the renderer (calls glViewport and some other GL sanity checks) */
        wlr_renderer_begin(drw, m->wlr_output->width, m->wlr_output->height);
        wlr_renderer_clear(drw, root.color);

        render_layershell(m, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND);
        render_layershell(m, ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM);
        render_clients(m);
        render_independents(m->wlr_output);
        render_layershell(m, ZWLR_LAYER_SHELL_V1_LAYER_TOP);
        render_layershell(m, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY);

        wlr_list_for_each(&render_data.textures, (void*)render_texture);
        render_popups(m);

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
