#include "monitor.h"
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_list.h>
#include <stdlib.h>
#include <wlr/util/log.h>

#include "parseConfig.h"
#include "render/render.h"
#include "server.h"
#include "tagset.h"
#include "tile/tileUtils.h"

/* monitors */
static const struct mon_rule monrules[] = {
    /* name       mfact nmaster scale layout       rotate/reflect */
    /* example of a HiDPI laptop monitor:
    { "eDP-1",    0.5,  1,      2,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL },
    */
    /* defaults */
    { NULL, 0.55, 1, 1, &defaultLayout, WL_OUTPUT_TRANSFORM_NORMAL },
};

struct wl_list mons;
struct monitor *selected_monitor = NULL;

void create_monitor(struct wl_listener *listener, void *data)
{
    /* This event is raised by the backend when a new output (aka a display or
     * monitor) becomes available. */
    struct wlr_output *output = data;
    struct monitor *m;
    const struct mon_rule *r;

    /* The mode is a tuple of (width, height, refresh rate), and each
     * monitor supports only a specific set of modes. We just pick the
     * monitor's preferred mode; a more sophisticated compositor would let
     * the user configure it. */
    wlr_output_set_mode(output, wlr_output_preferred_mode(output));

    /* Allocates and configures monitor state using configured rules */
    m = output->data = calloc(1, sizeof(struct monitor));
    m->output = output;
    m->tagset = create_tagset(&tagNames, 0, 0);
    push_seleceted_tags(m->tagset, TAG_ONE);
    for (r = monrules; r < END(monrules); r++) {
        if (!r->name || strstr(output->name, r->name)) {
            m->mfact = r->mfact;
            m->nmaster = r->nmaster;
            wlr_output_set_scale(output, r->scale);
            wlr_xcursor_manager_load(server.cursorMgr, r->scale);
            set_selected_layout(m->tagset, *r->lt);
            wlr_output_set_transform(output, r->rr);
            break;
        }
    }
    /* Set up event listeners */
    m->frame.notify = renderFrame;
    wl_signal_add(&output->events.frame, &m->frame);
    m->destroy.notify = cleanupMonitor;
    wl_signal_add(&output->events.destroy, &m->destroy);

    wl_list_insert(&mons, &m->link);

    wlr_output_enable(output, 1);
    if (!wlr_output_commit(output))
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
    wlr_output_layout_add_auto(output_layout, output);
}

void set_monitor(struct monitor *m)
{
    selected_monitor = m;
}


void cleanupMonitor(struct wl_listener *listener, void *data)
{
    struct wlr_output *wlr_output = data;
    struct monitor *m = wlr_output->data;

    wl_list_remove(&m->destroy.link);
    free(m);
}

struct monitor *xytomon(double x, double y)
{
    struct wlr_output *o = wlr_output_layout_output_at(output_layout, x, y);
    return o ? o->data : NULL;
}
