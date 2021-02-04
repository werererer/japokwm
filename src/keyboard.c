#include "keyboard.h"

#include "server.h"
#include "keybinding.h"

void buttonpress(struct wl_listener *listener, void *data)
{
    struct wlr_event_pointer_button *event = data;
    /* struct wlr_keyboard *keyboard; */
    /* uint32_t mods; */

    switch (event->state) {
    case WLR_BUTTON_PRESSED:
        {
            /* Change focus if the button was _pressed_ over a client */

            /* Translate libinput to xkbcommon code */
            unsigned sym = event->button + 64985;

            /* get modifiers */
            struct wlr_keyboard *kb = wlr_seat_get_keyboard(server.seat);
            int mods = wlr_keyboard_get_modifiers(kb);

            button_pressed(mods, sym);
            break;
        }
    case WLR_BUTTON_RELEASED:
        /* If you released any buttons, we exit interactive move/resize mode. */
        /* XXX should reset to the pointer focus's current setcursor */
        if (server.cursor.cursor_mode != CURSOR_NORMAL) {
            wlr_xcursor_manager_set_cursor_image(server.cursor_mgr,
                    "left_ptr", server.cursor.wlr_cursor);
            server.cursor.cursor_mode = CURSOR_NORMAL;
            /* Drop the window off on its new monitor */
            struct monitor *m = xytomon(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y);
            set_selected_monitor(m);
            return;
        }
        break;
    }
    /* If the event wasn't handled by the compositor, notify the client with
     * pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(server.seat,
            event->time_msec, event->button, event->state);
}

void cleanupkeyboard(struct wl_listener *listener, void *data)
{
    struct wlr_input_device *device = data;
    struct keyboard *kb = device->data;

    wl_list_remove(&kb->destroy.link);
    wl_list_remove(&kb->link);
    free(kb);
}
