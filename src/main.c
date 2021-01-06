/*
 * See LICENSE file for copyright and license details.
 */

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <assert.h>
#include <getopt.h>
#include <linux/input-event-codes.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <systemd/sd-bus.h>
#include <tgmath.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/backend/multi.h>
#include <wlr/backend/session.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_primary_selection_v1.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include <wlr/util/region.h>
#include <wlr/xwayland.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon.h>

#include "client.h"
#include "container.h"
#include "ipc-server.h"
#include "keybinding.h"
#include "lib/actions/actions.h"
#include "monitor.h"
#include "parseConfig.h"
#include "popup.h"
#include "render/render.h"
#include "root.h"
#include "server.h"
#include "tile/tile.h"
#include "tile/tileTexture.h"
#include "tile/tileUtils.h"
#include "workspace.h"

typedef struct {
    struct wl_listener request_mode;
    struct wl_listener destroy;
} Decoration;

/* Used to move all of the data necessary to render a surface from the top-level
 * frame handler to the per-surface render function. */
/* function declarations */
void axisnotify(struct wl_listener *listener, void *data);
void buttonpress(struct wl_listener *listener, void *data);
void chvt(unsigned int ui);
void cleanup();
void cleanupkeyboard(struct wl_listener *listener, void *data);
void commitnotify(struct wl_listener *listener, void *data);
void createkeyboard(struct wlr_input_device *device);
void createnotify(struct wl_listener *listener, void *data);
void createnotify_layer_shell(struct wl_listener *listener, void *data);
void createpointer(struct wlr_input_device *device);
void createxdeco(struct wl_listener *listener, void *data);
void cursorframe(struct wl_listener *listener, void *data);
void destroynotify(struct wl_listener *listener, void *data);
void destroyxdeco(struct wl_listener *listener, void *data);
void getxdecomode(struct wl_listener *listener, void *data);
void incnmaster(int i);
void inputdevice(struct wl_listener *listener, void *data);
void keypress(struct wl_listener *listener, void *data);
void keypressmod(struct wl_listener *listener, void *data);
void maprequest(struct wl_listener *listener, void *data);
void maprequestx11(struct wl_listener *listener, void *data);
void motionabsolute(struct wl_listener *listener, void *data);
void motionrelative(struct wl_listener *listener, void *data);
void run(char *startup_cmd);
void set_cursor(struct wl_listener *listener, void *data);
void setpsel(struct wl_listener *listener, void *data);
void setsel(struct wl_listener *listener, void *data);
void setmfact(float factor);
int setup();
void sigchld(int unused);
void tagmon(int i);
void unmapnotify(struct wl_listener *listener, void *data);

/* global variables */

/* global event handlers */
static struct wl_listener cursor_axis = {.notify = axisnotify};
static struct wl_listener cursor_button = {.notify = buttonpress};
static struct wl_listener cursor_frame = {.notify = cursorframe};
static struct wl_listener cursor_motion = {.notify = motionrelative};
static struct wl_listener cursor_motion_absolute = {.notify = motionabsolute};
static struct wl_listener new_input = {.notify = inputdevice};
static struct wl_listener new_output = {.notify = create_monitor};
static struct wl_listener new_xdeco = {.notify = createxdeco};
static struct wl_listener new_xdg_surface = {.notify = createnotify};
static struct wl_listener new_layer_shell_surface = {.notify = createnotify_layer_shell};
static struct wl_listener request_cursor = {.notify = set_cursor};
static struct wl_listener request_set_psel = {.notify = setpsel};
static struct wl_listener request_set_sel = {.notify = setsel};

static void activatex11(struct wl_listener *listener, void *data);
static void createnotifyx11(struct wl_listener *listener, void *data);
static struct wl_listener new_xwayland_surface = {.notify = createnotifyx11};

void axisnotify(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an axis event,
     * for example when you move the scroll wheel. */
    struct wlr_event_pointer_axis *event = data;
    /* Notify the client with pointer focus of the axis event. */
    wlr_seat_pointer_notify_axis(server.seat,
            event->time_msec, event->orientation, event->delta,
            event->delta_discrete, event->source);
}

void buttonpress(struct wl_listener *listener, void *data)
{
    struct wlr_event_pointer_button *event = data;
    /* struct wlr_keyboard *keyboard; */
    /* uint32_t mods; */

    switch (event->state) {
    case WLR_BUTTON_PRESSED:
        {
            /* Change focus if the button was _pressed_ over a client */
            struct container *con;
            if ((con = xytocontainer(server.cursor->x, server.cursor->y)))
                focus_container(con, selected_monitor, FOCUS_NOOP);

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
        if (server.cursorMode != CurNormal) {
            wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
                    "left_ptr", server.cursor);
            server.cursorMode = CurNormal;
            /* Drop the window off on its new monitor */
            set_selected_monitor(xytomon(server.cursor->x, server.cursor->y));
            return;
        }
        break;
    }
    /* If the event wasn't handled by the compositor, notify the client with
     * pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(server.seat,
            event->time_msec, event->button, event->state);
}

void chvt(unsigned int ui)
{
    wlr_session_change_vt(wlr_backend_get_session(server.backend), ui);
}

void cleanup()
{
    wlr_xwayland_destroy(server.xwayland.wlr_xwayland);
    wl_display_destroy_clients(server.display);

    wlr_xcursor_manager_destroy(server.cursorMgr);
    wlr_cursor_destroy(server.cursor);
    wlr_output_layout_destroy(server.output_layout);
}

void cleanupkeyboard(struct wl_listener *listener, void *data)
{
    struct wlr_input_device *device = data;
    struct keyboard *kb = device->data;

    wl_list_remove(&kb->destroy.link);
    free(kb);
}

void commitnotify(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, commit);
    if (!c->con)
        return;

    switch (c->type) {
        case XDG_SHELL:
            /* mark a pending resize as completed */
            if (c->resize && c->resize <= c->surface.xdg->configure_serial) {
                c->resize = 0;
            }
            break;
        case LAYER_SHELL:
            /* mark a pending resize as completed */
            if (c->resize && c->resize <= c->surface.layer->configure_serial)
                c->resize = 0;
            break;
        default:
            break;
    }

    struct container *con;
    wl_list_for_each(con, &stack, slink) {
        container_damage_part(con);
    }
}

void createkeyboard(struct wlr_input_device *device)
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
    wlr_keyboard_set_repeat_info(device->keyboard, repeat_rate, repeat_delay);

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

void createnotify(struct wl_listener *listener, void *data)
{
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
     * client, either a toplevel (application window) or popup. */
    struct wlr_xdg_surface *xdg_surface = data;

    if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL)
        return;

    /* Allocate a Client for this surface */
    struct client *c = xdg_surface->data = calloc(1, sizeof(struct client));
    c->surface.xdg = xdg_surface;
    c->bw = border_px;
    c->type = XDG_SHELL;

    /* Tell the client not to try anything fancy */
    wlr_xdg_toplevel_set_tiled(c->surface.xdg, WLR_EDGE_TOP |
            WLR_EDGE_BOTTOM | WLR_EDGE_LEFT | WLR_EDGE_RIGHT);

    /* Listen to the various events it can emit */
    c->commit.notify = commitnotify;
    wl_signal_add(&xdg_surface->surface->events.commit, &c->commit);
    c->map.notify = maprequest;
    wl_signal_add(&xdg_surface->events.map, &c->map);
    c->unmap.notify = unmapnotify;
    wl_signal_add(&xdg_surface->events.unmap, &c->unmap);
    c->destroy.notify = destroynotify;
    wl_signal_add(&xdg_surface->events.destroy, &c->destroy);
    /* popups */
    c->new_popup.notify = popup_handle_new_popup;
    wl_signal_add(&xdg_surface->events.new_popup, &c->new_popup);
}

void createnotify_layer_shell(struct wl_listener *listener, void *data)
{
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
     * client, either a toplevel (application window) or popup. */
    struct wlr_layer_surface_v1 *layer_surface = data;
    struct client *c;

    /* Allocate a Client for this surface */
    c = layer_surface->data = calloc(1, sizeof(struct client));
    c->surface.layer = layer_surface;
    c->bw = 0;
    c->type = LAYER_SHELL;

    /* Listen to the various events it can emit */
    c->commit.notify = commitnotify;
    wl_signal_add(&layer_surface->surface->events.commit, &c->commit);
    c->map.notify = maprequest;
    wl_signal_add(&layer_surface->events.map, &c->map);
    c->unmap.notify = unmapnotify;
    wl_signal_add(&layer_surface->events.unmap, &c->unmap);
    c->destroy.notify = destroynotify;
    wl_signal_add(&layer_surface->events.destroy, &c->destroy);
    // TODO: remove this line
    wlr_layer_surface_v1_configure(c->surface.layer,
            selected_monitor->geom.width, selected_monitor->geom.height);

    /* popups */
    c->new_popup.notify = popup_handle_new_popup;
    wl_signal_add(&layer_surface->events.new_popup, &c->new_popup);
}

void createpointer(struct wlr_input_device *device)
{
    /* We don't do anything special with pointers. All of our pointer handling
     * is proxied through wlr_cursor. On another compositor, you might take this
     * opportunity to do libinput configuration on the device to set
     * acceleration, etc. */
    wlr_cursor_attach_input_device(server.cursor, device);
}

void createxdeco(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_toplevel_decoration_v1 *wlr_deco = data;
    Decoration *d = wlr_deco->data = calloc(1, sizeof(*d));

    wl_signal_add(&wlr_deco->events.request_mode, &d->request_mode);
    d->request_mode.notify = getxdecomode;
    wl_signal_add(&wlr_deco->events.destroy, &d->destroy);
    d->destroy.notify = destroyxdeco;

    getxdecomode(&d->request_mode, wlr_deco);
}

void cursorframe(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an frame
     * event. Frame events are sent after regular pointer events to group
     * multiple events together. For instance, two axis events may happen at the
     * same time, in which case a frame event won't be sent in between. */
    /* Notify the client with pointer focus of the frame event. */
    wlr_seat_pointer_notify_frame(server.seat);
}

void destroynotify(struct wl_listener *listener, void *data)
{
    /* Called when the surface is destroyed and should never be shown again. */
    struct client *c = wl_container_of(listener, c, destroy);
    wl_list_remove(&c->map.link);
    wl_list_remove(&c->unmap.link);
    wl_list_remove(&c->destroy.link);

    switch (c->type) {
        case XDG_SHELL:
            wl_list_remove(&c->commit.link);
            break;
        case X11_MANAGED:
            wl_list_remove(&c->activate.link);
            break;
        default:
            break;
    }

    free(c);
    c = NULL;

    arrange(false);
    focus_top_container(selected_monitor, FOCUS_NOOP);
}

void destroyxdeco(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_toplevel_decoration_v1 *wlr_deco = data;
    Decoration *d = wlr_deco->data;

    wl_list_remove(&d->destroy.link);
    wl_list_remove(&d->request_mode.link);
    free(d);

}

void getxdecomode(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_toplevel_decoration_v1 *wlr_deco = data;
    wlr_xdg_toplevel_decoration_v1_set_mode(wlr_deco,
            WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}

void inputdevice(struct wl_listener *listener, void *data)
{
    /* This event is raised by the backend when a new input device becomes
     * available. */
    struct wlr_input_device *device = data;
    uint32_t caps;
    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        createkeyboard(device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        createpointer(device);
        break;
    default:
        /* XXX handle other input device types */
        break;
    }
    /* We need to let the wlr_seat know what our capabilities are, which is
     * communiciated to the client. In dwl we always have a server.cursor, even 
     * if there are no pointer devices, so we always include that capability. */
    /* XXX do we actually require a cursor? */
    caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server.keyboards))
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    wlr_seat_set_capabilities(server.seat, caps);
}

static bool handle_VT_keys(struct keyboard *kb, uint32_t keycode)
{
    const xkb_keysym_t *syms;
    int nsyms =
        xkb_state_key_get_syms(kb->device->keyboard->xkb_state, keycode, &syms);
    bool handled = false;

    for (int i = 0; i < nsyms; i++) {
        if (syms[i] >= XKB_KEY_XF86Switch_VT_1
                && syms[i] <= XKB_KEY_XF86Switch_VT_12) {
            /* if required switch to different virtual terminal */
            if (wlr_backend_is_multi(server.backend)) {
                struct wlr_session *session =
                    wlr_backend_get_session(server.backend);
                if (session) {
                    int vt = syms[i] - XKB_KEY_XF86Switch_VT_1 + 1;
                    wlr_session_change_vt(session, vt);
                    handled = true;
                    break;
                }
            }
        }
    }
    return handled;
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
    /* uint32_t mods = wlr_keyboard_get_modifiers(kb->device->keyboard); */
    /* On _press_, attempt to process a compositor keybinding. */

    if (handle_VT_keys(kb, keycode))
        return;

    if (event->state == WLR_KEY_PRESSED) {
        for (i = 0; i < nsyms; i++) {
            handled = key_pressed(mods, syms[i]);
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

static bool wants_floating(struct client *c) {
    if (c->type != X11_MANAGED && c->type != X11_UNMANAGED) {
        return false;
    }
    struct wlr_xwayland_surface *surface = c->surface.xwayland;
    struct xwayland xwayland = server.xwayland;

    if (surface->modal) {
        return true;
    }

    for (size_t i = 0; i < surface->window_type_len; ++i) {
        xcb_atom_t type = surface->window_type[i];
        if (type == xwayland.atoms[NET_WM_WINDOW_TYPE_DIALOG] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_UTILITY] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_TOOLBAR] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_POPUP_MENU] ||
                type == xwayland.atoms[NET_WM_WINDOW_TYPE_SPLASH]) {
            return true;
        }
    }

    struct wlr_xwayland_surface_size_hints *size_hints = surface->size_hints;
    if (size_hints != NULL &&
            size_hints->min_width > 0 && size_hints->min_height > 0 &&
            (size_hints->max_width == size_hints->min_width ||
            size_hints->max_height == size_hints->min_height)) {
        return true;
    }

    return false;
}

static bool is_popup_menu(struct client *c)
{
    struct wlr_xwayland_surface *surface = c->surface.xwayland;
    struct xwayland xwayland = server.xwayland;
    for (size_t i = 0; i < surface->window_type_len; ++i) {
        xcb_atom_t type = surface->window_type[i];
        if (type == xwayland.atoms[NET_WM_WINDOW_TYPE_POPUP_MENU]) {
            return true;
        }
    }
    return false;
}

void maprequest(struct wl_listener *listener, void *data)
{
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct client *c = wl_container_of(listener, c, map);

    struct monitor *m = selected_monitor;
    c->ws = m->ws;

    switch (c->type) {
        case XDG_SHELL:
            {
                wl_list_insert(&clients, &c->link);
                create_container(c, m);
                break;
            }
        case LAYER_SHELL:
            {
                struct monitor *m = outputtomon(c->surface.layer->output);
                wl_list_insert(&clients, &c->link);
                wlr_layer_surface_v1_configure(c->surface.layer, m->geom.width, m->geom.height);
                create_container(c, m);
                break;
            }
        default:
            break;
    }
    arrange(false);
    focus_top_container(selected_monitor, FOCUS_NOOP);

    container_damage_part(c->con);
}

void maprequestx11(struct wl_listener *listener, void *data)
{
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct client *c = wl_container_of(listener, c, map);
    struct wlr_xwayland_surface *xwayland_surface = c->surface.xwayland;
    struct monitor *m = selected_monitor;

    c->type = xwayland_surface->override_redirect ? X11_UNMANAGED : X11_MANAGED;
    c->ws = m->ws;

    if (is_popup_menu(c)) {
        wl_list_insert(&server.independents, &c->ilink);
        return;
    }

    struct container *con = create_container(c, m);
    switch (c->type) {
        case X11_MANAGED:
            {
                wl_list_insert(&clients, &c->link);

                con->on_top = false;
                if (wants_floating(con->client)) {
                    set_container_floating(con, true);
                    resize(con, get_center_box(con->m->geom), false);
                }
                break;
            }
        case X11_UNMANAGED:
            {
                wl_list_insert(&server.independents, &c->ilink);

                con->on_top = true;
                set_container_floating(con, true);
                resize(con, get_center_box(con->m->geom), false);
                break;
            }
        default:
            break;
    }
    arrange(false);
    focus_top_container(selected_monitor, FOCUS_NOOP);
}

void motionabsolute(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an _absolute_
     * motion event, from 0..1 on Each axis. This happens, for example, when
     * wlroots is running under a Wayland window rather than KMS+DRM, and you
     * move the mouse over the Windows. You could enter the window from any edge,
     * so we have to warp the mouse there. There is also some hardware which
     * emits these events. */
    struct wlr_event_pointer_motion_absolute *event = data;
    wlr_cursor_warp_absolute(server.cursor, event->device, event->x, event->y);
    motionnotify(event->time_msec);
}

void motionrelative(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits a _relative_
     * pointer motion event (i.e. a delta) */
    struct wlr_event_pointer_motion *event = data;
    /* The cursor doesn't move unless we tell it to. The cursor automatically
     * handles constraining the motion to the output layout, as well as any
     * special configuration applied for the specific input device which
     * generated the event. You can pass NULL for the device if you want to move
     * the cursor around without any input. */
    wlr_cursor_move(server.cursor, event->device,
            event->delta_x, event->delta_y);
    motionnotify(event->time_msec);
}

void run(char *startup_cmd)
{
    pid_t startup_pid = -1;

    /* Add a Unix socket to the Wayland display. */
    const char *socket = wl_display_add_socket_auto(server.display);
    if (!socket)
        BARF("startup: display_add_socket_auto");

    /* Start the backend. This will enumerate outputs and inputs, become the DRM
     * master, etc */
    if (!wlr_backend_start(server.backend))
        BARF("startup: backend_start");

    wlr_cursor_warp_absolute(server.cursor, NULL, 0.4, 0.3);
    /* Now that outputs are initialized, choose initial selMon based on
     * cursor position, and set default cursor image */
    set_selected_monitor(xytomon(server.cursor->x, server.cursor->y));

    /* XXX hack to get cursor to display in its initial location (100, 100)
     * instead of (0, 0) and then jumping.  still may not be fully
     * initialized, as the image/coordinates are not transformed for the
     * monitor when displayed here */
    wlr_cursor_warp_closest(server.cursor, NULL, server.cursor->x, server.cursor->y);
    wlr_xcursor_manager_set_cursor_image(server.cursorMgr, "left_ptr", server.cursor);

    /* Set the WAYLAND_DISPLAY environment variable to our socket and run the
     * startup command if requested. */
    setenv("WAYLAND_DISPLAY", socket, 1);
    if (startup_cmd) {
        startup_pid = fork();
        if (startup_pid < 0)
            EBARF("startup: fork");
        if (startup_pid == 0) {
            printf("exec: %s\n", startup_cmd);
            execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void *)NULL);
            EBARF("startup: execl");
        }
    }
    /* Run the Wayland event loop. This does not return until you exit the
     * compositor. Starting the backend rigged up all of the necessary event
     * loop configuration to listen to libinput events, DRM events, generate
     * frame events at the refresh rate, and so on. */
    wl_display_run(server.display);

    if (startup_cmd) {
        kill(startup_pid, SIGTERM);
        waitpid(startup_pid, NULL, 0);
    }
}

void set_cursor(struct wl_listener *listener, void *data)
{
    /* This event is raised by the seat when a client provides a cursor image */
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    /* If we're "grabbing" the server.cursor, don't use the client's image */
    /* XXX still need to save the provided surface to restore later */
    if (server.cursorMode != CurNormal)
        return;
    /* This can be sent by any client, so we check to make sure this one is
     * actually has pointer focus first. If so, we can tell the cursor to
     * use the provided surface as the cursor image. It will set the
     * hardware cursor on the output that it's currently on and continue to
     * do so as the cursor moves between outputs. */
    if (event->seat_client == server.seat->pointer_state.focused_client)
        wlr_cursor_set_surface(server.cursor, event->surface,
                event->hotspot_x, event->hotspot_y);
}

/* arg > 1.0 will set mfact absolutely */
void setmfact(float factor)
{
    if (!selected_monitor->ws->layout.funcId)
        return;
    factor = factor < 1.0 ? factor + selected_monitor->mfact : factor - 1.0;
    if (factor < 0.1 || factor > 0.9)
        return;
    selected_monitor->mfact = factor;
    arrange(false);
}

void setpsel(struct wl_listener *listener, void *data)
{
    /* This event is raised by the seat when a client wants to set the selection,
     * usually when the user copies something. wlroots allows compositors to
     * ignore such requests if they so choose, but in dwl we always honor
     */
    struct wlr_seat_request_set_primary_selection_event *event = data;
    wlr_seat_set_primary_selection(server.seat, event->source, event->serial);
}

void setsel(struct wl_listener *listener, void *data)
{
    /* This event is raised by the seat when a client wants to set the selection,
     * usually when the user copies something. wlroots allows compositors to
     * ignore such requests if they so choose, but in dwl we always honor
     */
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(server.seat, event->source, event->serial);
}

int setup()
{
    wl_list_init(&mons);
    wl_list_init(&focus_stack);
    wl_list_init(&stack);
    wl_list_init(&containers);
    wl_list_init(&layer_stack);
    wl_list_init(&popups);

    L = luaL_newstate();
    luaL_openlibs(L);
    if(update_config(L)) {
        wlr_log(WLR_ERROR, "failed updating config");
        return 1;
    }
    init_overlay();
    create_workspaces(tag_names);
    /* The Wayland display is managed by libwayland. It handles accepting
     * clients from the Unix socket, manging Wayland globals, and so on. */
    server.display = wl_display_create();
    server.wl_event_loop = wl_display_get_event_loop(server.display);
    ipc_init(server.wl_event_loop);

    /* clean up child processes immediately */
    sigchld(0);

    /* The backend is a wlroots feature which abstracts the underlying input and
     * output hardware. The autocreate option will choose the most suitable
     * backend based on the current environment, such as opening an X11 window
     * if an X11 server is running. The NULL argument here optionally allows you
     * to pass in a custom renderer if wlr_renderer doesn't meet your needs. The
     * backend uses the renderer, for example, to fall back to software cursors
     * if the backend does not support hardware cursors (some older GPUs
     * don't). */
    if (!(server.backend = wlr_backend_autocreate(server.display, NULL)))
        BARF("couldn't create backend");

    /* If we don't provide a renderer, autocreate makes a GLES2 renderer for us.
     * The renderer is responsible for defining the various pixel formats it
     * supports for shared memory, this configures that for clients. */
    drw = wlr_backend_get_renderer(server.backend);
    wlr_renderer_init_wl_display(drw, server.display);

    /* This creates some hands-off wlroots interfaces. The compositor is
     * necessary for clients to allocate surfaces and the data device manager
     * handles the clipboard. Each of these wlroots interfaces has room for you
     * to dig your fingers in and play with their behavior if you want. Note that
     * the clients cannot set the selection directly without compositor approval,
     * see the setsel() function. */
    server.compositor = wlr_compositor_create(server.display, drw);
    wlr_export_dmabuf_manager_v1_create(server.display);
    wlr_screencopy_manager_v1_create(server.display);
    wlr_data_device_manager_create(server.display);
    wlr_gamma_control_manager_v1_create(server.display);
    wlr_primary_selection_v1_device_manager_create(server.display);
    wlr_viewporter_create(server.display);

    /* Creates an output layout, which a wlroots utility for working with an
     * arrangement of screens in a physical layout. */
    server.output_layout = wlr_output_layout_create();
    wlr_xdg_output_manager_v1_create(server.display, server.output_layout);

    /* Configure textures */
    wlr_list_init(&render_data.textures);
    /* Configure a listener to be notified when new outputs are available on the
     * backend. */
    wl_signal_add(&server.backend->events.new_output, &new_output);

    /* Set up our client lists and the xdg-shell. The xdg-shell is a
     * Wayland protocol which is used for application windows. For more
     * detail on shells, refer to the article:
     *
     * https://drewdevault.com/2018/07/29/Wayland-shells.html
     */
    wl_list_init(&clients);
    wl_list_init(&server.independents);

    server.xdgShell = wlr_xdg_shell_create(server.display);
    wl_signal_add(&server.xdgShell->events.new_surface, &new_xdg_surface);
    // remove csd(client side decorations) completely from xdg based windows
    wlr_server_decoration_manager_set_default_mode(
            wlr_server_decoration_manager_create(server.display),
            WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);

    server.layerShell = wlr_layer_shell_v1_create(server.display);
    wl_signal_add(&server.layerShell->events.new_surface,
            &new_layer_shell_surface);

    /* Use xdg_decoration protocol to negotiate server-side decorations */
    server.xdecoMgr = wlr_xdg_decoration_manager_v1_create(server.display);
    wl_signal_add(&server.xdecoMgr->events.new_toplevel_decoration, &new_xdeco);

    /*
     * Creates a server.cursor, which is a wlroots utility for tracking the cursor
     * image shown on screen.
     */
    server.cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server.cursor, server.output_layout);

    /* Creates an xcursor manager, another wlroots utility which loads up
     * Xcursor themes to source cursor images from and makes sure that cursor
     * images are available at all scale factors on the screen (necessary for
     * HiDPI support). Scaled cursors will be loaded with each output. */
    server.cursorMgr = wlr_xcursor_manager_create(NULL, 24);

    /*
     * wlr_cursor *only* displays an image on screen. It does not move around
     * when the pointer moves. However, we can attach input devices to it, and
     * it will generate aggregate events for all of them. In these events, we
     * can choose how we want to process them, forwarding them to clients and
     * moving the cursor around. More detail on this process is described in my
     * input handling blog post:
     *
     * https://drewdevault.com/2018/07/17/Input-handling-in-wlroots.html
     *
     * And more comments are sprinkled throughout the notify functions above.
     */
    wl_signal_add(&server.cursor->events.motion, &cursor_motion);
    wl_signal_add(&server.cursor->events.motion_absolute,
            &cursor_motion_absolute);
    wl_signal_add(&server.cursor->events.button, &cursor_button);
    wl_signal_add(&server.cursor->events.axis, &cursor_axis);
    wl_signal_add(&server.cursor->events.frame, &cursor_frame);

    /*
     * Configures a seat, which is a single "seat" at which a user sits and
     * operates the computer. This conceptually includes up to one keyboard,
     * pointer, touch, and drawing tablet device. We also rig up a listener to
     * let us know when new input devices are available on the backend.
     */
    wl_list_init(&server.keyboards);
    wl_signal_add(&server.backend->events.new_input, &new_input);
    server.seat = wlr_seat_create(server.display, "seat0");
    wl_signal_add(&server.seat->events.request_set_cursor,
            &request_cursor);
    wl_signal_add(&server.seat->events.request_set_selection,
            &request_set_sel);
    wl_signal_add(&server.seat->events.request_set_primary_selection,
            &request_set_psel);

    /*
     * Initialise the XWayland X server.
     * It will be started when the first X client is started.
     */
    server.xwayland.wlr_xwayland = wlr_xwayland_create(server.display, server.compositor, true);
    if (server.xwayland.wlr_xwayland) {
        server.xwayland_ready.notify = handle_xwayland_ready;
        wl_signal_add(&server.xwayland.wlr_xwayland->events.ready, &server.xwayland_ready);
        wl_signal_add(&server.xwayland.wlr_xwayland->events.new_surface, &new_xwayland_surface);

        setenv("DISPLAY", server.xwayland.wlr_xwayland->display_name, true);
    } else {
        wlr_log(WLR_ERROR, "failed to setup XWayland X server, continuing without itn");
    }
    return 0;
}

void sigchld(int unused)
{
    if (signal(SIGCHLD, sigchld) == SIG_ERR)
        EBARF("can't install SIGCHLD handler");
    while (0 < waitpid(-1, NULL, WNOHANG));
}

void unmapnotify(struct wl_listener *listener, void *data)
{
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct client *c = wl_container_of(listener, c, unmap);

    container_damage_whole(c->con);
    destroy_container(c->con);

    switch (c->type) {
        case LAYER_SHELL:
            wl_list_remove(&c->link);
            break;
        case XDG_SHELL:
            wl_list_remove(&c->link);
            break;
        case X11_MANAGED:
            wl_list_remove(&c->link);
            break;
        case X11_UNMANAGED:
            wl_list_remove(&c->ilink);
            break;
    }
}

void activatex11(struct wl_listener *listener, void *data)
{
       struct client *c = wl_container_of(listener, c, activate);

       /* Only "managed" windows can be activated */
       if (c->type == X11_MANAGED)
               wlr_xwayland_surface_activate(c->surface.xwayland, 1);
}

void createnotifyx11(struct wl_listener *listener, void *data)
{
    struct wlr_xwayland_surface *xwayland_surface = data;
    struct client *c;
    /* Allocate a Client for this surface */
    c = xwayland_surface->data = calloc(1, sizeof(struct client));
    c->surface.xwayland = xwayland_surface;
    // set default value will be overriden on maprequest
    c->type = X11_MANAGED;
    c->bw = border_px;

    /* Listen to the various events it can emit */
    c->map.notify = maprequestx11;
    wl_signal_add(&xwayland_surface->events.map, &c->map);
    c->unmap.notify = unmapnotify;
    wl_signal_add(&xwayland_surface->events.unmap, &c->unmap);
    c->activate.notify = activatex11;
    wl_signal_add(&xwayland_surface->events.request_activate, &c->activate);
    c->destroy.notify = destroynotify;
    wl_signal_add(&xwayland_surface->events.destroy, &c->destroy);
}

Atom getatom(xcb_connection_t *xc, const char *name)
{
    Atom atom = 0;
    xcb_intern_atom_cookie_t cookie;
    xcb_intern_atom_reply_t *reply;

    cookie = xcb_intern_atom(xc, 0, strlen(name), name);
    if ((reply = xcb_intern_atom_reply(xc, cookie, NULL)))
        atom = reply->atom;
    free(reply);

    return atom;
}

int main(int argc, char *argv[])
{
    char *startup_cmd = NULL;
    int c;

    while ((c = getopt(argc, argv, "s:h")) != -1) {
        if (c == 's')
            startup_cmd = optarg;
        else
            goto usage;
    }
    if (optind < argc)
        goto usage;

    // Wayland requires XDG_RUNTIME_DIR for creating its communications
    // socket
    if (!getenv("XDG_RUNTIME_DIR")) {
        fprintf(stderr,
                "XDG_RUNTIME_DIR is not set in the environment. Aborting.\n");
        return EXIT_FAILURE;
    }
    if (setup()) {
        wlr_log(WLR_ERROR, "didn't find file");
        return EXIT_FAILURE;
    }
    run(startup_cmd);
    cleanup();
    return EXIT_SUCCESS;

usage:
    BARF("Usage: %s [-s startup command]", argv[0]);
}
