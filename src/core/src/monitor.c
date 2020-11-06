#include "monitor.h"
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <stdlib.h>

#include "parseConfig.h"
#include "render/render.h"
#include "server.h"
#include "tagset.h"
#include "tile/tileUtils.h"

/* monitors */
static const struct monRule monrules[] = {
    /* name       mfact nmaster scale layout       rotate/reflect */
    /* example of a HiDPI laptop monitor:
    { "eDP-1",    0.5,  1,      2,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL },
    */
    /* defaults */
    { NULL, 0.55, 1, 1, &defaultLayout, WL_OUTPUT_TRANSFORM_NORMAL },
};

struct wl_list mons;
struct monitor *selMon = NULL;

void createMonitor(struct wl_listener *listener, void *data)
{
    /* This event is raised by the backend when a new output (aka a display or
     * monitor) becomes available. */
    struct wlr_output *wlr_output = data;
    struct monitor *m;
    const struct monRule *r;

    /* The mode is a tuple of (width, height, refresh rate), and each
     * monitor supports only a specific set of modes. We just pick the
     * monitor's preferred mode; a more sophisticated compositor would let
     * the user configure it. */
    wlr_output_set_mode(wlr_output, wlr_output_preferred_mode(wlr_output));

    /* Allocates and configures monitor state using configured rules */
    m = wlr_output->data = calloc(1, sizeof(*m));
    m->wlr_output = wlr_output;
    tagsetCreate(&m->tagset);
    pushSelTags(&m->tagset, TAG_ONE);
    for (r = monrules; r < END(monrules); r++) {
        if (!r->name || strstr(wlr_output->name, r->name)) {
            m->mfact = r->mfact;
            m->nmaster = r->nmaster;
            wlr_output_set_scale(wlr_output, r->scale);
            wlr_xcursor_manager_load(server.cursorMgr, r->scale);
            setSelLayout(&m->tagset, *r->lt);
            wlr_output_set_transform(wlr_output, r->rr);
            break;
        }
    }
    /* Set up event listeners */
    m->frame.notify = renderFrame;
    wl_signal_add(&wlr_output->events.frame, &m->frame);
    m->destroy.notify = cleanupMonitor;
    wl_signal_add(&wlr_output->events.destroy, &m->destroy);

    wl_list_insert(&mons, &m->link);

    wlr_output_enable(wlr_output, 1);
    if (!wlr_output_commit(wlr_output))
        return;

    /* Adds this to the output layout. The add_auto function arranges outputs
     * from left-to-right in the order they appear. A more sophisticated
     * compositor would let the user configure the arrangement of outputs in the
     * layout.
     *
     * The output layout utility automatically adds a wl_output global to the
     * display, which Wayland clients can see to find out information about the
     * output (such as DPI, scale factor, manufacturer, etc).
     */
    wlr_output_layout_add_auto(output_layout, wlr_output);
    sgeom = *wlr_output_layout_get_box(output_layout, NULL);
}

void setMonitor(struct monitor *m)
{
    selMon = m;
}


void cleanupMonitor(struct wl_listener *listener, void *data)
{
    struct wlr_output *wlr_output = data;
    struct monitor *m = wlr_output->data;

    wl_list_remove(&m->destroy.link);
    free(m);
}
