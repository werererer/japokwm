#include "tile/tileTexture.h"
#include "client.h"
#include "tagset.h"
#include "utils/coreUtils.h"
#include "utils/stringUtils.h"
#include "utils/writeFile.h"
#include "tile/tileUtils.h"
#include <cairo/cairo.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/util/log.h>
#include <stdlib.h>

bool overlay = false;
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

struct posTexture *create_textbox(struct wlr_box box, float boxColor[],
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
    cairo_move_to(cr, width/2.0, height/2.0);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_source_rgba(cr,
            textColor[0], textColor[1], textColor[2], textColor[3]);
    cairo_show_text(cr, text);
    cairo_surface_flush(surface);

    unsigned char *cdata = cairo_image_surface_get_data(surface);

    struct wlr_texture *cTexture = 
        wlr_texture_from_pixels(drw, WL_SHM_FORMAT_ARGB8888, stride, 
                width, height, cdata);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    struct posTexture *posTexture = calloc(1, sizeof(*posTexture));

    posTexture->texture = cTexture;
    posTexture->dataType = OVERLAY;
    posTexture->x = box.x;
    posTexture->y = box.y;
    return posTexture;
}

void init_overlay()
{
    wlr_list_init(&renderData.textures);
}

void create_new_overlay()
{
    // recreate list
    wlr_list_clear(&renderData.textures);
    create_overlay();
}

void create_overlay()
{
    struct client *c;

    char text[NUM_DIGITS];
    int i = 0;
    wl_list_for_each_reverse(c, &stack, slink) {
        c->position = i;
        if (!visibleon(c, c->mon))
            continue;
        intToString(text, c->textPosition+1);


        struct posTexture *pTexture =
            create_textbox(c->geom, overlayColor, textColor, text);
        // sync properties
        pTexture->mon = c->mon;
        pTexture->tagset = c->tagset;
        wlr_list_push(&renderData.textures,
                create_textbox(c->geom, overlayColor, textColor, text));
        i++;
    }
}

void update_client_overlay(struct client *c)
{
    if (overlay) {
        if (renderData.textures.length >= c->position+1) {
            wlr_list_del(&renderData.textures, c->position);
        }

        char text[NUM_DIGITS];
        if (c->hidden)
            return;

        intToString(text, c->textPosition+1);

        wlr_list_insert(&renderData.textures, c->position,
                create_textbox(c->geom, overlayColor, textColor, text));
    } else {
        if (&renderData.textures.length > 0)
            wlr_list_clear(&renderData.textures);
    }
}

void update_overlay_count(size_t count)
{
    if (count == 0) {
        wlr_list_clear(&renderData.textures);
    } else {
        int len = renderData.textures.length;
        for (size_t i = count; i < len; i++) {
            wlr_list_del(&renderData.textures, count);
        }
    }
}

void update_overlay()
{
    if (overlay) {
        create_new_overlay();
    } else {
        wlr_list_clear(&renderData.textures);
    }
}

bool postexture_visible_on_tag(struct posTexture *pTexture, struct monitor *m, size_t focusedTag)
{
    if (m && pTexture) {
        if (pTexture->mon == m) {
            return pTexture->tagset->selTags[0] & position_to_flag(focusedTag);
        }
    }
    return false;
}

void write_overlay(struct monitor *m, const char *layout)
{
    if (!overlay)
        return;
    char file[NUM_CHARS];
    char filetmp[NUM_CHARS];
    char filename[NUM_DIGITS];

    strcpy(file, get_config_layout_path());
    join_path(file, layout);
    mkdir(file, 0755);
    join_path(file, filename);
    mkdir(file, 0755);
    strcpy(filetmp, file);
    for (int i = 0; i < 8; i++) {
        strcpy(file, filetmp);
        intToString(filename, i+1);
        join_path(file, filename);

        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        wlr_log(WLR_DEBUG, "create file %s", file);
        if (fd == -1) {
            wlr_log(WLR_ERROR, "file didn't open correctly: %s", file);
            return;
        }

        int k = 0;
        for (int j = 0; j < renderData.textures.length; j++) {
            // TODO: algorithm is not really efficient fix it
            struct posTexture *pTexture = renderData.textures.items[j];
            if (postexture_visible_on_tag(pTexture, m, i)) {
                struct wlr_box container = postexture_to_container(pTexture);
                struct wlr_fbox box =
                    get_relative_box(container, selected_monitor->m);
                write_container_to_file(fd, box);
                k++;
            }
        }

        close(fd);
        if (!k) {
            unlink(file);
        }
    }
}

struct wlr_box postexture_to_container(struct posTexture *pTexture)
{
    struct wlr_box box;
    if (!pTexture) {
        box.x = 0;
        box.y = 0;
        box.width = 0;
        box.height = 0;
        return box;
    }

    box.x = pTexture->x;
    box.y = pTexture->y;
    box.width = pTexture->texture->width;
    box.height = pTexture->texture->height;
    return box;
}
