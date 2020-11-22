#include "tile/tileTexture.h"
#include "client.h"
#include "tagset.h"
#include "utils/coreUtils.h"
#include "utils/stringUtils.h"
#include "utils/writeFile.h"
#include <cairo/cairo.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <stdlib.h>

bool overlay;
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
    overlay = false;
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

void writeThisOverlay(char *layout)
{
    write_overlay(selected_monitor, layout);
}

void write_overlay(struct monitor *m, char *layout)
{
    if (!overlay)
        return;
    struct client *c;
    char file[NUM_CHARS];
    char filename[NUM_DIGITS];

    mkdir(layout, 0755);
    // tags are counted from 1
    for (int i = 1; i <= 9; i++) {
        intToString(filename, i);
        strcpy(file, layout);
        join_path(file, filename);

        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        for (int j = 0; j < renderData.textures.length; j++) {
            // TODO: algorithm is not really efficient fix it
            wl_list_for_each(c, &clients, link) {
                if (visibleonTag(c, m, i)) {
                    write_container_to_file(fd,
                            posTextureToContainer(renderData.textures.items[j]));
                }
            }
        }

        close(fd);
    }
}

void set_overlay(bool ol)
{
    overlay = ol;
}

bool get_overlay()
{
    return overlay;
}
