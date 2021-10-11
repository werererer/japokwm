#include "keyboard.h"

#include "seat.h"
#include "server.h"
#include "keybinding.h"
#include "utils/coreUtils.h"

static bool handle_VT_keys(struct keyboard *kb, uint32_t keycode)
{
    const xkb_keysym_t *syms;
    struct wlr_input_device *wlr_device = kb->seat_device->input_device->wlr_device;
    int nsyms =
        xkb_state_key_get_syms(wlr_device->keyboard->xkb_state, keycode, &syms);
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

static int handle_keyboard_repeat(void *data)
{
    debug_print("repeat\n");
    struct keyboard *keyboard = (struct keyboard *)data;
    struct wlr_keyboard *wlr_device =
            keyboard->seat_device->input_device->wlr_device->keyboard;
    if (keyboard->repeat_binding) {
        if (wlr_device->repeat_info.rate > 0) {
            // We queue the next event first, as the command might cancel it
            if (wl_event_source_timer_update(keyboard->key_repeat_source,
                    1000 / wlr_device->repeat_info.rate) < 0) {
                printf("failed to update key repeat timer\n");
            }
        }

        debug_print("repeat_binding: %s\n", keyboard->repeat_binding);
        handle_keyboard_key(keyboard->repeat_binding);
    }
    return 0;
}

void cleanupkeyboard(struct wl_listener *listener, void *data)
{
    struct wlr_input_device *device = data;
    struct keyboard *kb = device->data;

    destroy_keyboard(kb);
}

void create_keyboard(struct seat *seat, struct seat_device *seat_device)
{
    struct wlr_input_device *wlr_device = seat_device->input_device->wlr_device;
    struct keyboard *kb = wlr_device->data = calloc(1, sizeof(*kb));
    kb->seat_device = seat_device;
    kb->seat = seat;

    /* Prepare an XKB keymap and assign it to the keyboard. */
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_map_new_from_names(context, NULL,
        XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(wlr_device->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(wlr_device->keyboard,
            server.default_layout->options->repeat_rate,
            server.default_layout->options->repeat_delay);

    kb->key_repeat_source = wl_event_loop_add_timer(
            server.wl_event_loop,
            handle_keyboard_repeat
            , kb);

    kb->repeat_binding = "";

    /* Here we set up listeners for keyboard events. */
    LISTEN(&wlr_device->keyboard->events.modifiers, &kb->modifiers, keypressmod);
    LISTEN(&wlr_device->keyboard->events.key, &kb->key, handle_key_event);
    LISTEN(&wlr_device->events.destroy, &kb->destroy, cleanupkeyboard);

    wlr_seat_set_keyboard(seat->wlr_seat, wlr_device);

    /* And add the keyboard to our list of server.keyboards */
    g_ptr_array_add(server.keyboards, kb);
}

void destroy_keyboard(struct keyboard *kb)
{
    if (!kb)
        return;
    wl_list_remove(&kb->modifiers.link);
    wl_list_remove(&kb->key.link);
    wl_list_remove(&kb->destroy.link);
    g_ptr_array_remove(server.keyboards, kb);

/*     xkb_keymap_unref(kb->seat_device->input_device->wlr_device->keyboard->keymap); */

    wl_event_source_remove(kb->key_repeat_source);

    free(kb);
}

void keyboard_disarm_key_repeat(struct keyboard *kb) {
    if (!kb) {
        return;
    }
    free(kb->repeat_binding);
    if (wl_event_source_timer_update(kb->key_repeat_source, 0) < 0) {
        printf("failed to disarm key repeat timer \n");
    }
}


void handle_key_event(struct wl_listener *listener, void *data)
{
    /* This event is raised when a key is pressed or released. */
    struct wlr_event_keyboard_key *event = data;

    struct keyboard *kb = wl_container_of(listener, kb, key);
    /* Get a list of keysyms based on the keymap for this keyboard */
    struct wlr_input_device *wlr_device = kb->seat_device->input_device->wlr_device;

    /* Translate libinput keycode -> xkbcommon */
    uint32_t keycode = event->keycode + 8;
    xkb_state_key_get_one_sym(wlr_device->keyboard->xkb_state, keycode);

    /* create new state to clear the shift modifier to get a instead of A */
    struct xkb_state *state = xkb_state_new(wlr_device->keyboard->keymap);
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(state, keycode, &syms);
    uint32_t mods = wlr_keyboard_get_modifiers(wlr_device->keyboard);

    bool handled = false;
    /* On _press_, attempt to process a compositor keybinding. */

    if (handle_VT_keys(kb, keycode))
        return;

    char *bind = mod_to_keybinding(mods, syms[nsyms-1]);

    switch (event->state) {
        case WL_KEYBOARD_KEY_STATE_PRESSED:
            debug_print("pressed\n");
            handled = handle_keyboard_key(bind);
            server.prev_mods = mods;
            break;
        case WL_KEYBOARD_KEY_STATE_RELEASED:
            debug_print("released\n");
            // TODO: we can optimize this ;). It is called multiple times
            // because multiple keys are released. good luck!
            if (server.registered_key_combos->len <= 0) {
                list_clear(server.named_key_combos, free);
            }
            break;
    }

    if (!handled) {
        struct wlr_seat *wlr_seat = kb->seat_device->seat->wlr_seat;
        /* Pass unhandled keycodes along to the client. */
        wlr_seat_set_keyboard(wlr_seat, wlr_device);
        wlr_seat_keyboard_notify_key(wlr_seat, event->time_msec,
            event->keycode, event->state);
    }

    // Set up (or clear) keyboard repeat for a pressed binding. Since the
    // binding may remove the keyboard, the timer needs to be updated first
    if (bind && nsyms > 0 && wlr_device->keyboard->repeat_info.delay > 0) {
        kb->repeat_binding = strdup(bind);
        if (wl_event_source_timer_update(kb->key_repeat_source,
                wlr_device->keyboard->repeat_info.delay) < 0) {
            printf("failed to set key repeat timer\n");
        }
    } else if (kb->repeat_binding) {
        printf("keyboard disarm\n");
        keyboard_disarm_key_repeat(kb);
    }

    free(bind);
    xkb_state_unref(state);
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
    struct wlr_seat *wlr_seat = kb->seat->wlr_seat;
    wlr_seat_set_keyboard(wlr_seat, kb->seat_device->input_device->wlr_device);
    /* Send modifiers to the client. */
    wlr_seat_keyboard_notify_modifiers(wlr_seat,
        &kb->seat_device->input_device->wlr_device->keyboard->modifiers);
}
