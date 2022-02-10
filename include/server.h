#ifndef SERVER_H
#define SERVER_H
#include <uv.h>
#include <wayland-server.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_virtual_pointer_v1.h>
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/xcursor.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_relative_pointer_v1.h>
#include <wlr/types/wlr_output_management_v1.h>
#include <glib.h>
#include <pthread.h>

#include "cursor.h"
#include "layout.h"
#include "options.h"
#include "xwayland.h"
#include "input_manager.h"
#include "utils/coreUtils.h"
#include "bitset/bitset.h"

struct server {
    bool is_running;
    uv_loop_t *uv_loop;
    uv_async_t async_handler;

    struct wl_display *wl_display;
    struct wl_event_loop *wl_event_loop;
    struct wlr_backend *backend;
    struct wlr_compositor *compositor;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    struct wlr_xdg_shell *xdg_shell;
    struct wlr_layer_shell_v1 *layer_shell;
    struct wlr_xdg_decoration_manager_v1 *xdeco_mgr;

    struct input_manager *input_manager;

    struct event_handler *event_handler;

    struct wlr_virtual_pointer_manager_v1 *virtual_pointer_mgr;
    struct wlr_virtual_keyboard_manager_v1 *virtual_keyboard_mgr;
    struct wlr_relative_pointer_manager_v1 *relative_pointer_mgr;
    struct wlr_input_inhibit_manager *input_inhibitor_mgr;
    struct wlr_pointer_constraints_v1 *pointer_constraints;
    struct wlr_output_manager_v1 *output_mgr;

    struct layout *default_layout;
    struct ring_buffer *default_layout_ring;

    struct wlr_output_layout *output_layout;
    GPtrArray *keyboards;
    int prev_mods;

    GPtrArray *registered_key_combos;
    struct wl_event_source *combo_timer_source;

    // TODO: rename
    GPtrArray *named_key_combos;

    GHashTable *tags;

    GPtrArray *scratchpad;

    char *custom_path;
    char *error_path;
    GPtrArray *layout_paths;
    GPtrArray *config_paths;
    GPtrArray *user_data_paths;
    char *config_file;

    int previous_tag;
    BitSet *previous_bitset;
    // a global bitset to hold temporary values useful to improve performance
    struct BitSet *tmp_bitset;
    // this variable ought to be only used by bitset.c everything other is
    // UB so be careful.
    // We have this to avoid calling malloc/free too often thus improving
    // performance.
    struct BitSet *local_tmp_bitset;

    struct monitor *selected_monitor;

    GPtrArray *mons;
    GPtrArray *popups;
    // X11 popups are handled as containers
    GPtrArray *xwayland_popups;

    GPtrArray2D *layer_visual_stack_lists;

    GPtrArray *layer_visual_stack_background;
    GPtrArray *layer_visual_stack_bottom;
    GPtrArray *layer_visual_stack_top;
    GPtrArray *layer_visual_stack_overlay;

    GPtrArray *container_stack;

    /* global event handlers */
    struct wl_listener new_output;
    struct wl_listener new_xdeco;
    struct wl_listener new_xdg_surface;
    struct wl_listener new_layer_shell_surface;
    struct wl_listener new_pointer_constraint;
    struct wl_listener output_mgr_apply;
    struct wl_listener output_mgr_test;

    // TODO: give them a more sensible name they are here to fix a bug for
    // sloppy focus
    struct container *old_xy_container;
    bool xy_container_is_locked;

    // this breaks the reload_config load_config loop
    bool prohibit_reload_config;

    struct container *grab_c;
    enum wlr_edges grabbed_edges;
#if JAPOKWM_HAS_XWAYLAND
    struct xwayland xwayland;
    struct wl_listener xwayland_ready;

    struct wl_listener new_xwayland_surface;
#endif
};

struct function_data {
    lua_State *L;
    int lua_func_ref;
    char *cmd;
    char *output;
};

extern struct server server;

void init_server();
void finalize_server();

int start_server(char *startup_cmd);
void server_terminate(struct server *server);
int stop_server();

void server_reset_layout_ring(struct ring_buffer *layout_ring);
int server_get_tag_count();
int server_get_tag_key_count();
GList *server_get_tags();
struct tag *get_tag(int id);

void server_prohibit_reloading_config();
void server_allow_reloading_config();
bool server_is_config_reloading_prohibited();

void server_start_keycombo(const char *key_combo_name);
bool server_is_keycombo(const char *key_combo_name);

struct monitor *server_get_selected_monitor();
void server_set_selected_monitor(struct monitor *m);

void server_center_default_cursor_in_monitor(struct monitor *m);

/* This set of functions can be used everywhere except for bitset.c. If you do
 * it is UB */
BitSet *server_bitset_get_tmp();
BitSet *server_bitset_get_tmp_copy(BitSet *bitset);

/* The functions are only allowed to be called from bitset.c and offer a slight
 * performance boost */
BitSet *server_bitset_get_local_tmp();
BitSet *server_bitset_get_local_tmp_copy(BitSet *bitset);

struct tag *server_get_selected_tag();
struct layout *server_get_selected_layout();
#endif /* SERVER_H */
