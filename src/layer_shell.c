#include "layer_shell.h"

#include "monitor.h"
#include "popup.h"
#include "stdlib.h"

void create_notify_layer_shell(struct wl_listener *listener, void *data)
{
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
     * client, either a toplevel (application window) or popup. */
    struct wlr_layer_surface_v1 *layer_surface = data;
    struct client *c;

    /* Allocate a Client for this surface */
    union surface_t surface;
    surface.layer = layer_surface;
    c = layer_surface->data = create_client(LAYER_SHELL, surface);
    c->bw = 0;

    if (!c->surface.layer->output) {
        c->surface.layer->output = selected_monitor->wlr_output;
    }
    struct monitor *m = output_to_monitor(c->surface.layer->output);
    wlr_layer_surface_v1_configure(c->surface.layer, m->geom.width, m->geom.height);

    /* Listen to the various events it can emit */
    c->map.notify = maprequest;
    wl_signal_add(&layer_surface->events.map, &c->map);
    c->unmap.notify = unmap_notify;
    wl_signal_add(&layer_surface->events.unmap, &c->unmap);
    c->destroy.notify = destroy_notify;
    wl_signal_add(&layer_surface->events.destroy, &c->destroy);
    c->destroy.notify = destroy_notify;

    /* wl_signal_add(&layer_surface->surface->role->commit,  .destroy, &c->destroy); */

    /* popups */
    c->new_popup.notify = popup_handle_new_popup;
    wl_signal_add(&layer_surface->events.new_popup, &c->new_popup);
}
