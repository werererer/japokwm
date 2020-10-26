#include "tile/tileUtils.h"
#include <client.h>
#include <julia.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <wayland-util.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_box.h>

#include "monitor.h"
#include "tagset.h"
#include "utils/coreUtils.h"
#include "parseConfig.h"
#include "tile/tileTexture.h"
#include "utils/parseConfigUtils.h"

bool overlay = false;

struct containerList {
    struct wlr_fbox *container;
    int size;
};

/* *
 * the wlr_fbox has includes the window size in percent.
 * It will we mulitiplicated with the screen width and height
 * */
static struct wlr_box getAbsoluteBox(struct monitor *m, struct wlr_fbox b)
{
    struct wlr_box w = m->tagset.w;

    w.x = w.width * b.x;
    w.y = w.height * b.y;
    w.width = w.width * b.width;
    w.height = w.height * b.height;
    return w;
}

void arrange(struct monitor *m, bool reset)
{
    /* Get effective monitor geometry to use for window area */
    struct tagset *tagset = &m->tagset;
    selMon->m = *wlr_output_layout_get_box(output_layout, selMon->wlr_output);
    //TODO: this doesn't belong to this function
    tagset->w = selMon->m;
    if (tagset->lt.arrange) {
        struct client *c = NULL;
        jl_value_t *v = NULL;

        int n = tiledClientCount(selMon);
        /* call arrange function
         * if previous layout is different or reset -> reset layout */
        if (strcmp(prevLayout.symbol, selMon->tagset.lt.symbol) != 0 || reset) {
            prevLayout = tagset->lt;
            jl_value_t *arg1 = jl_box_int64(n);
            v = jl_call1(tagset->lt.arrange, arg1);
        } else {
            jl_function_t *f = jl_eval_string("Layouts.update");
            jl_value_t *arg1 = jl_box_int64(n);
            v = jl_call1(f, arg1);
        }

        struct containerList *containerList = NULL;
        if (v) {
            containerList = jl_unbox_voidpointer(v);

            // place clients
            int i = 0;
            printf("numb clients: %i\n", wl_list_length(&clients));
            printf("mon set: %i\n", m->tagset.selTags[0]);
            wl_list_for_each(c, &clients, link) {
                printf("pos %i\n", c->tagset.selTags[0]);
                if (!visibleon(c, selMon) || c->floating)
                    continue;
                struct wlr_box b =
                    getAbsoluteBox(selMon, containerList->container[i]);
                resize(c, b.x, b.y, b.width, b.height, false);
                i = MIN(i + 1, containerList->size-1);
            }
            // create overlay

            if (overlay) {
                createNewOverlay();
            } else {
                wlr_list_clear(&renderData.textures);
            }

        } else {
            printf("Empty function with symbol: %s\n", tagset->lt.symbol);
        }
    }
}


void arrangeThis(bool reset)
{
    arrange(selMon, reset);
}

void resize(struct client *c, int x, int y, int w, int h, bool interact)
{
    /*
     * Note that I took some shortcuts here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at
     * the new size, then commit any movement that was prepared.
     */
    struct wlr_box box;
    box.x = x;
    box.y = y;
    box.width = w;
    box.height = h;
    c->geom = box;
    applybounds(c, box);

    /* wlroots makes this a no-op if size hasn't changed */
    switch (c->type) {
        case XDGShell:
            c->resize = wlr_xdg_toplevel_set_size(c->surface.xdg,
                    c->geom.width - 2 * c->bw, c->geom.height - 2 * c->bw);
            break;
        case LayerShell:
            wlr_layer_surface_v1_configure(c->surface.layer,
                    c->geom.width - 2 * c->bw, c->geom.height - 2 * c->bw);
            break;
        case X11Managed:
        case X11Unmanaged:
            wlr_xwayland_surface_configure(c->surface.xwayland,
                    c->geom.x, c->geom.y,
                    c->geom.width - 2 * c->bw, c->geom.height - 2 * c->bw);
    }
}

void updateLayout()
{
    selMon->tagset.lt = getConfigLayout("layout");
    arrange(selMon, true);
}

int thisTiledClientCount()
{
    return tiledClientCount(selMon);
}

int tiledClientCount(struct monitor *m)
{
    struct client *c;
    int n = 0;

    wl_list_for_each(c, &clients, link)
        if (visibleon(c, m) && !c->floating)
            n++;
    return n;
}

int clientPos()
{
    struct monitor *m = selMon;
    struct client *c;
    int n = 0;

    wl_list_for_each(c, &clients, link) {
        if (visibleon(c, m) && !c->floating) {
            if (c == selClient())
                return n;
            n++;
        }
    }
    return 0;
}

void setOverlay(bool ol)
{
    overlay = ol;
}

bool getOverlay()
{
    return overlay;
}
