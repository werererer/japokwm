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
#include <glib.h>

#include "cursor.h"
#include "layout.h"
#include "options.h"
#include "xwayland.h"
#include "input_manager.h"
#include "utils/coreUtils.h"
#include "bitset/bitset.h"

struct server {
    struct wl_display *wl_display;
    struct wl_event_loop *wl_event_loop;
    struct wlr_backend *backend;
    struct wlr_compositor *compositor;

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

    GPtrArray *config_paths;
    char *config_file;
    char *config_dir;

    int previous_workspace;
    BitSet *previous_bitset;

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

    GPtrArray2D *layer_visual_stack_lists;

    GPtrArray *layer_visual_stack_background;
    GPtrArray *layer_visual_stack_bottom;
    GPtrArray *layer_visual_stack_top;
    GPtrArray *layer_visual_stack_overlay;

    GPtrArray *floating_stack;
    GPtrArray *floating_containers;

    /* global event handlers */
    struct wl_listener new_output;
    struct wl_listener new_xdeco;
    struct wl_listener new_xdg_surface;
    struct wl_listener new_layer_shell_surface;
    struct wl_listener new_pointer_constraint;

    // TODO: give them a more sensible name they are here to fix a bug for
    // sloppy focus
    struct container *old_xy_container;
    bool xy_container_is_locked;

#if JAPOKWM_HAS_XWAYLAND
    struct xwayland xwayland;
    struct wl_listener xwayland_ready;

    struct wl_listener new_xwayland_surface;
#endif
};

extern struct server server;

void init_server();
#endif /* SERVER_H */
