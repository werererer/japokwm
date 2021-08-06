#ifndef SERVER_H
#define SERVER_H
#include <wayland-server.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_virtual_pointer_v1.h>
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/xcursor.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_relative_pointer_v1.h>

#include "cursor.h"
#include "layout.h"
#include "options.h"
#include "xwayland.h"
#include "input_manager.h"

struct server {
    struct wl_display *wl_display;
    struct wl_event_loop *wl_event_loop;
    struct wlr_backend *backend;
    struct wlr_compositor *compositor;

    struct xwayland xwayland;
    struct wl_listener xwayland_ready;

    struct wlr_xdg_shell *xdg_shell;
    struct wlr_layer_shell_v1 *layer_shell;
    struct wlr_xdg_decoration_manager_v1 *xdeco_mgr;

    struct input_manager *input_manager;

    struct wlr_virtual_pointer_manager_v1 *virtual_pointer_mgr;
    struct wlr_virtual_keyboard_manager_v1 *virtual_keyboard_mgr;
    struct wlr_relative_pointer_manager_v1 *relative_pointer_mgr;
    struct wlr_input_inhibit_manager *input_inhibitor_mgr;
    struct wlr_pointer_constraints_v1 *pointer_constraints;

    struct layout *default_layout;
    struct layout_set layout_set;

    struct wlr_output_layout *output_layout;
    GPtrArray *keyboards;

    GPtrArray *workspaces;

    GPtrArray *scratchpad;

    const char *config_file;
    const char *config_dir;

    struct tagset *previous_tagset;

    GPtrArray *client_lists;
    GPtrArray *normal_clients;
    GPtrArray *non_tiled_clients;
    GPtrArray *independent_clients;

    GPtrArray *mons;
    GPtrArray *popups;
    // X11 popups are handled as containers
    GPtrArray *xwayland_popups;

    GPtrArray *tagsets;

    struct wlr_surface *old_surface;
};

extern struct server server;

void init_server();
#endif /* SERVER_H */
