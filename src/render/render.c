#include "render/render.h"
#include "client.h"
#include "monitor.h"
#include "popup.h"
#include "root.h"
#include "server.h"
#include "tile/tileTexture.h"
#include "tile/tileUtils.h"

#include <assert.h>
#include <stdio.h>
#include <wayland-util.h>
#include <wlr/render/gles2.h>
#include <wlr/types/wlr_output_damage.h>
#include <wlr/util/edges.h>
#include <wlr/util/region.h>

struct wlr_renderer *drw;
struct render_data render_data;

static void render(struct wlr_surface *surface, int sx, int sy, void *data);
static void render_container(struct monitor *m, pixman_region32_t *output_damage);
static void render_independents(struct monitor *m);
static void render_texture(struct pos_texture *texture);
static void scissor_output(struct wlr_output *output, pixman_box32_t *rect);
static void render_t(struct wlr_output *wlr_output, pixman_region32_t *output_damage,
        struct wlr_texture *texture, const struct wlr_box *box,
        const float matrix[static 9]);

// _box.x and .y are expected to be layout-local
// _box.width and .height are expected to be output-buffer-local
void render_rect(struct monitor *m, pixman_region32_t *output_damage,
        const struct wlr_box *_box, const float color[static 4]) {
    struct wlr_output *wlr_output = m->wlr_output;
    struct wlr_renderer *renderer =
        wlr_backend_get_renderer(wlr_output->backend);

    struct wlr_box box;
    memcpy(&box, _box, sizeof(struct wlr_box));
    box.x -= m->geom.x * wlr_output->scale;
    box.y -= m->geom.y * wlr_output->scale;

    pixman_region32_t damage;
    pixman_region32_init(&damage);
    pixman_region32_union_rect(&damage, &damage, box.x, box.y,
        box.width, box.height);
    pixman_region32_intersect(&damage, &damage, output_damage);
    bool damaged = pixman_region32_not_empty(&damage);
    if (!damaged) {
        pixman_region32_fini(&damage);
        return;
    }

    int nrects;
    pixman_box32_t *rects = pixman_region32_rectangles(&damage, &nrects);
    for (int i = 0; i < nrects; ++i) {
        scissor_output(wlr_output, &rects[i]);
        wlr_render_rect(renderer, &box, color,
            wlr_output->transform_matrix);
    }
}

static bool intersects_with_output(struct monitor *m,
        struct wlr_output_layout *output_layout, struct wlr_box *surface_box)
{
        /* Since the surface_box's x- and y-coordinates are already output local,
         * the x- and y-coordinates of this box need to be 0 for this function to
         * work correctly. */
        struct wlr_box output_box = {0};
        wlr_output_effective_resolution(m->wlr_output, &output_box.width, &output_box.height);

        struct wlr_box intersection;
        return wlr_box_intersection(&intersection, &output_box, surface_box);
}

static void output_for_each_surface_iterator(struct wlr_surface *surface, int sx, int sy, void *user_data)
{
        struct surface_iterator_data *data = user_data;
        struct monitor *m = data->m;

        if (!wlr_surface_has_buffer(surface)) {
                return;
        }

        struct wlr_box surface_box = {
                .x = data->ox + sx + surface->sx,
                .y = data->oy + sy + surface->sy,
                .width = surface->current.width,
                .height = surface->current.height,
        };

        if (!intersects_with_output(m, server.output_layout, &surface_box)) {
                return;
        }

        data->user_iterator(data->m, surface, &surface_box, data->user_data);
}


void output_surface_for_each_surface(struct monitor *m, struct wlr_surface
        *surface, double ox, double oy, surface_iterator_func_t iterator,
        void *user_data)
{
    struct surface_iterator_data data = {
        .user_iterator = iterator,
        .user_data = user_data,
        .m = m,
        .ox = ox,
        .oy = oy,
    };

    wlr_surface_for_each_surface(surface, output_for_each_surface_iterator, &data);
}

static void render_t(struct wlr_output *wlr_output,
        pixman_region32_t *output_damage, struct wlr_texture *texture,
        const struct wlr_box *box, const float matrix[static 9])
{
    struct wlr_renderer *renderer = wlr_backend_get_renderer(wlr_output->backend);

    pixman_region32_t damage;
    pixman_region32_init(&damage);
    pixman_region32_union_rect(&damage, &damage, box->x, box->y, box->width, box->height);
    pixman_region32_intersect(&damage, &damage, output_damage);
    if (!pixman_region32_not_empty(&damage)) {
        pixman_region32_fini(&damage);
        return;
    }

    int nrects;
    pixman_box32_t *rects = pixman_region32_rectangles(&damage, &nrects);
    for (int i = 0; i < nrects; i++) {
        scissor_output(wlr_output, &rects[i]);
        wlr_render_texture_with_matrix(renderer, texture, matrix, 1.0f);
    }
}

static void render_surface_iterator(struct monitor *m, struct wlr_surface *surface, struct wlr_box *box, pixman_region32_t *output_damage)
{
    struct wlr_texture *texture = wlr_surface_get_texture(surface);
    struct wlr_output *wlr_output = m->wlr_output;

    if (!texture) {
        return;
    }

    scale_box(box, m->wlr_output->scale);

    float matrix[9];
    enum wl_output_transform transform = wlr_output_transform_invert(surface->current.transform);
    wlr_matrix_project_box(matrix, box, transform, 0.0f, wlr_output->transform_matrix);

    render_t(wlr_output, output_damage, texture, box, matrix);
}

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
    wlr_output_layout_output_coords(server.output_layout, output, &ox, &oy);

    /* We also have to apply the scale factor for HiDPI outputs. This is only
     * part of the puzzle, dwl does not fully support HiDPI. */
    obox.x = ox + rdata->x + sx;
    obox.y = oy + rdata->y + sy;
    obox.width = surface->current.width;
    obox.height = surface->current.height;
    scale_box(&obox, output->scale);

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

static void
damage_surface_iterator(struct monitor *m, struct wlr_surface *surface, struct wlr_box *box, void *user_data)
{
    struct wlr_output *wlr_output = m->wlr_output;
    bool whole = *(bool *) user_data;

    scale_box(box, wlr_output->scale);

    if (whole) {
        wlr_output_damage_add_box(m->damage, box);
    } else if (pixman_region32_not_empty(&surface->buffer_damage)) {
        pixman_region32_t damage;
        pixman_region32_init(&damage);
        wlr_surface_get_effective_damage(surface, &damage);

        wlr_region_scale(&damage, &damage, wlr_output->scale);
        if (ceil(wlr_output->scale) > surface->current.scale) {
            /* When scaling up a surface it'll become
               blurry, so we need to expand the damage
               region. */
            wlr_region_expand(&damage, &damage, ceil(wlr_output->scale) - surface->current.scale);
        }
        pixman_region32_translate(&damage, box->x, box->y);
        wlr_output_damage_add(m->damage, &damage);
        pixman_region32_fini(&damage);
    }
}

void output_damage_surface(struct monitor *m, struct wlr_surface *surface, double lx, double ly, bool whole)
{
    if (!m->wlr_output->enabled) {
        return;
    }

    double ox = lx, oy = ly;
    wlr_output_layout_output_coords(server.output_layout, m->wlr_output, &ox, &oy);
    output_surface_for_each_surface(m, surface, ox, oy, damage_surface_iterator, &whole);
}

static void scissor_output(struct wlr_output *output, pixman_box32_t *rect)
{
    struct wlr_renderer *renderer = wlr_backend_get_renderer(output->backend);

    struct wlr_box box = {
        .x = rect->x1,
        .y = rect->y1,
        .width = rect->x2 - rect->x1,
        .height = rect->y2 - rect->y1,
    };

    int output_width, output_height;
    wlr_output_transformed_resolution(output, &output_width, &output_height);
    enum wl_output_transform transform = wlr_output_transform_invert(output->transform);
    wlr_box_transform(&box, &box, transform, output_width, output_height);

    wlr_renderer_scissor(renderer, &box);
}

static void render_container(struct monitor *m, pixman_region32_t *output_damage)
{
    struct container *con, *sel = selected_container(m);

    /* Each subsequent window we render is rendered on top of the last. Because
     * our stacking list is ordered front-to-back, we iterate over it backwards. */
    wl_list_for_each_reverse(con, &stack, slink) {
        if (!visibleon(con, m) && !con->floating)
            continue;

        double ox, oy;
        int w, h;
        struct wlr_surface *surface = get_wlrsurface(con->client);
        ox = con->geom.x - con->client->bw;
        oy = con->geom.y - con->client->bw;
        wlr_output_layout_output_coords(server.output_layout, m->wlr_output, &ox, &oy);
        w = con->geom.width;
        h = con->geom.height;

        struct wlr_box *borders;
        borders = (struct wlr_box[4]) {
            {ox, oy, w + 2 * con->client->bw, con->client->bw},             /* top */
                {ox, oy + con->client->bw, con->client->bw, h},                 /* left */
                {ox + con->client->bw + w, oy + con->client->bw, con->client->bw, h},     /* right */
                {ox, oy + con->client->bw + h, w + 2 * con->client->bw, con->client->bw}, /* bottom */
        };

        /* Draw window borders */
        const float *color = (con == sel) ? focus_color : border_color;
        for (int i = 0; i < 4; i++) {
            scale_box(&borders[i], m->wlr_output->scale);
            render_rect(m, output_damage, &borders[i], color);
        }

        /* This calls our render function for each surface among the
         * xdg_surface's toplevel and popups. */

        render_surface_iterator(m, get_wlrsurface(con->client), &con->geom, output_damage);

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        wlr_surface_send_frame_done(surface, &now);
    }
}

static void render_layershell(struct monitor *m, enum zwlr_layer_shell_v1_layer layer)
{
    struct container *con;
    struct render_data rdata;
    /* Each subsequent window we render is rendered on top of the last. Because
     * our stacking list is ordered front-to-back, we iterate over it backwards. */
    wl_list_for_each_reverse(con, &layer_stack, llink) {
        if (con->client->type != LAYER_SHELL)
            continue;
        if (con->client->surface.layer->current.layer != layer)
            continue;

        /* Only render visible clients which are shown on this monitor */
        if (!visibleon(con, m))
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

static void render_texture(struct pos_texture *texture)
{
    if (postexture_visible_on(
                texture,
                selected_monitor,
                selected_monitor->ws)) {
        wlr_render_texture(drw, texture->texture,
                selected_monitor->wlr_output->transform_matrix, texture->x,
                texture->y, 1);
    }
}

static void render_independents(struct monitor *m)
{
    struct client *c;

    wl_list_for_each_reverse(c, &server.independents, ilink) {
        struct wlr_box geom = (struct wlr_box) {
            .x = c->surface.xwayland->x,
            .y = c->surface.xwayland->y,
            .width = c->surface.xwayland->width,
            .height = c->surface.xwayland->height,
        };

        /* Only render visible clients which show on this output */
        if (!wlr_output_layout_intersects(server.output_layout, m->wlr_output, &geom))
            continue;

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        struct render_data rdata = (struct render_data) {
            .output = m->wlr_output,
            .when = &now,
            .x = c->surface.xwayland->x,
            .y = c->surface.xwayland->y,
        };
        wlr_surface_for_each_surface(c->surface.xwayland->surface, render, &rdata);
    }
}

static void render_popups(struct monitor *m, pixman_region32_t *output_damage)
{
    struct xdg_popup *popup;
    wl_list_for_each_reverse(popup, &popups, plink) {
        struct wlr_surface *surface = popup->xdg->base->surface;
        render_surface_iterator(m, surface, &popup->geom, output_damage);

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        wlr_surface_send_frame_done(surface, &now);
    }
}

static void clear_frame(struct monitor *m, float color[4], pixman_region32_t *damage)
{
    struct wlr_renderer *renderer = wlr_backend_get_renderer(server.backend);

    /* // debug stuff */
    /* float color2[4] = {0.4f, 0.1f, 0.0f, 1.0f}; */
    /* wlr_renderer_clear(renderer, color2); */

    int nrects;
    pixman_box32_t *rects = pixman_region32_rectangles(damage, &nrects);
    for (int i = 0; i < nrects; i++) {
        scissor_output(m->wlr_output, &rects[i]);
        wlr_renderer_clear(renderer, color);
    }
}

void render_frame(struct monitor *m, pixman_region32_t *damage)
{
    /* This function is called every time an output is ready to display a frame,
     * generally at the output's refresh rate (e.g. 60Hz). */
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    /* wlr_output_attach_render makes the OpenGL context current. */
    if (!wlr_output_attach_render(m->wlr_output, NULL))
        return;

    /* Begin the renderer (calls glViewport and some other GL sanity checks) */
    wlr_renderer_begin(drw, m->wlr_output->width, m->wlr_output->height);

    printf("render frame\n");
    clear_frame(m, m->root->color, damage);
    render_layershell(m, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND);
    render_layershell(m, ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM);
    render_container(m, damage);
    render_independents(m);
    render_layershell(m, ZWLR_LAYER_SHELL_V1_LAYER_TOP);
    render_layershell(m, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY);

    wlr_list_for_each(&render_data.textures, (void*)render_texture);
    render_popups(m, damage);

    /* Hardware cursors are rendered by the GPU on a separate plane, and can be
     * moved around without re-rendering what's beneath them - which is more
     * efficient. However, not all hardware supports hardware cursors. For this
     * reason, wlroots provides a software fallback, which we ask it to render
     * here. wlr_cursor handles configuring hardware vs software cursors for you,
     * and this function is a no-op when hardware cursors are in use. */
    wlr_output_render_software_cursors(m->wlr_output, damage);

    /* Conclude rendering and swap the buffers, showing the final frame
     * on-screen. */
    wlr_renderer_end(drw);

    wlr_output_commit(m->wlr_output);
}

void scale_box(struct wlr_box *box, float scale)
{
    box->x *= scale;
    box->y *= scale;
    box->width *= scale;
    box->height *= scale;
}
