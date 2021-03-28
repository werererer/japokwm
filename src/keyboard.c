#include "keyboard.h"

#include "server.h"
#include "keybinding.h"

static bool handle_VT_keys(struct keyboard *kb, uint32_t keycode)
{
    const xkb_keysym_t *syms;
    int nsyms =
        xkb_state_key_get_syms(kb->device->keyboard->xkb_state, keycode, &syms);
    bool handled = false;

    for (int i = 0; i < nsyms; i++) {
        if (syms[i] < XKB_KEY_XF86Switch_VT_1 || syms[i] > XKB_KEY_XF86Switch_VT_12)
            continue;
        if (!wlr_backend_is_multi(server.backend))
            break;

        /* if required switch to different virtual terminal */
        struct wlr_session *session =
            wlr_backend_get_session(server.backend);
        if (!session)
            break;

        int vt = syms[i] - XKB_KEY_XF86Switch_VT_1 + 1;
        wlr_session_change_vt(session, vt);
        handled = true;
    }
    return handled;
}

void buttonpress(struct wl_listener *listener, void *data)
{
    struct wlr_event_pointer_button *event = data;

    switch (event->state) {
        case WLR_BUTTON_PRESSED:
            {
                /* Translate libinput to xkbcommon code */
                unsigned sym = event->button + 64985;

                /* get modifiers */
                struct wlr_keyboard *kb = wlr_seat_get_keyboard(server.seat);
                int mods = wlr_keyboard_get_modifiers(kb);

                handle_keybinding(mods, sym);
                break;
            }
        case WLR_BUTTON_RELEASED:
            /* If you released any buttons, we exit interactive move/resize
             * mode. */
            /* XXX should reset to the pointer focus's current setcursor */
            if (server.cursor.cursor_mode != CURSOR_NORMAL) {
                wlr_xcursor_manager_set_cursor_image(server.cursor_mgr, "left_ptr",
                        server.cursor.wlr_cursor);
                server.cursor.cursor_mode = CURSOR_NORMAL;
                /* Drop the window off on its new monitor */
                struct monitor *m = xy_to_monitor(server.cursor.wlr_cursor->x,
                        server.cursor.wlr_cursor->y);
                set_selected_monitor(m);
                return;
            }
            break;
    }
    /* If the event wasn't handled by the compositor, notify the client with
     * pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(server.seat, event->time_msec, event->button,
            event->state);
}

void cleanupkeyboard(struct wl_listener *listener, void *data)
{
    struct wlr_input_device *device = data;
    struct keyboard *kb = device->data;

    wl_list_remove(&kb->destroy.link);
    wl_list_remove(&kb->link);
    free(kb);
}

void create_keyboard(struct wlr_input_device *device)
{
    struct xkb_context *context;
    struct xkb_keymap *keymap;
    struct keyboard *kb;

    kb = device->data = calloc(1, sizeof(*kb));
    kb->device = device;

    /* Prepare an XKB keymap and assign it to the keyboard. */
    context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    keymap = xkb_map_new_from_names(context, NULL,
        XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(device->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(device->keyboard, server.default_layout.options.repeat_rate,
            server.default_layout.options.repeat_delay);

    /* Here we set up listeners for keyboard events. */
    kb->modifiers.notify = keypressmod;
    wl_signal_add(&device->keyboard->events.modifiers, &kb->modifiers);
    kb->key.notify = keypress;
    wl_signal_add(&device->keyboard->events.key, &kb->key);
    kb->destroy.notify = cleanupkeyboard;
    wl_signal_add(&device->events.destroy, &kb->destroy);

    wlr_seat_set_keyboard(server.seat, device);

    /* And add the keyboard to our list of server.keyboards */
    wl_list_insert(&server.keyboards, &kb->link);
}

void keypress(struct wl_listener *listener, void *data)
{
    /* This event is raised when a key is pressed or released. */
    struct keyboard *kb = wl_container_of(listener, kb, key);
    struct wlr_event_keyboard_key *event = data;
    int i;

    /* Translate libinput keycode -> xkbcommon */
    uint32_t keycode = event->keycode + 8;
    /* Get a list of keysyms based on the keymap for this keyboard */
    const xkb_keysym_t *syms;
    struct xkb_state *state;
    xkb_state_key_get_one_sym(kb->device->keyboard->xkb_state, keycode);

    /* create new state to clear the shift modifier to get a instead of A */
    state = xkb_state_new(kb->device->keyboard->keymap);
    int nsyms = xkb_state_key_get_syms(state, keycode, &syms);
    uint32_t mods = wlr_keyboard_get_modifiers(kb->device->keyboard);

    bool handled = false;
    /* On _press_, attempt to process a compositor keybinding. */

    if (handle_VT_keys(kb, keycode))
        return;

    if (event->state == WLR_KEY_PRESSED) {
        for (i = 0; i < nsyms; i++) {
            handled = handle_keybinding(mods, syms[i]);
        }
    }

    if (!handled) {
        /* Pass unhandled keycodes along to the client. */
        wlr_seat_set_keyboard(server.seat, kb->device);
        wlr_seat_keyboard_notify_key(server.seat, event->time_msec,
            event->keycode, event->state);
    }
}

void keypressmod(struct wl_listener *listener, void *data)
{
    /* This event is raised when a modifier key, such as shift or alt, is
     * pressed. We simply communicate this to the client. */
    struct keyboard *kb = wl_container_of(listener, kb, modifiers);
    /*
     * A seat can only have one keyboard, but this is a limitation of the
     * Wayland protocol - not wlroots. We assign all connected server.keyboards 
     * to the same seat. You can swap out the underlying wlr_keyboard like this
     * and wlr_seat handles this transparently.
     */
    wlr_seat_set_keyboard(server.seat, kb->device);
    /* Send modifiers to the client. */
    wlr_seat_keyboard_notify_modifiers(server.seat,
        &kb->device->keyboard->modifiers);
}
