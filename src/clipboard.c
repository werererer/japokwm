#include "clipboard.h"
#include "server.h"
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_data_device.h>

void set_primary_selection(struct wl_listener *listener, void *data)
{
    /* This event is raised by the seat when a client wants to set the selection,
     * usually when the user copies something. wlroots allows compositors to
     * ignore such requests if they so choose, but in dwl we always honor
     */
    struct wlr_seat_request_set_primary_selection_event *event = data;
    wlr_seat_set_primary_selection(server.seat, event->source, event->serial);
}

void set_selection(struct wl_listener *listener, void *data)
{
    /* This event is raised by the seat when a client wants to set the selection,
     * usually when the user copies something. wlroots allows compositors to
     * ignore such requests if they so choose, but in dwl we always honor
     */
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(server.seat, event->source, event->serial);
}
