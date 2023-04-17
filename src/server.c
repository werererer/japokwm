#include "server.h"

#include "list_sets/container_stack_set.h"
#include "list_sets/focus_stack_set.h"
#include "monitor.h"
#include "stringop.h"
#include "tag.h"

#include <assert.h>
#include <poll.h>
#include <unistd.h>
#include <uv.h>
#include <wait.h>
#include <wayland-client.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_data_control_v1.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_idle.h>
#include <wlr/types/wlr_idle_inhibit_v1.h>
#include <wlr/types/wlr_input_inhibitor.h>
#include <wlr/types/wlr_primary_selection_v1.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_output_v1.h>

#include "ipc-server.h"
#include "keybinding.h"
#include "layer_shell.h"
#include "monitor.h"
#include "ring_buffer.h"
#include "translationLayer.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "xdg_shell.h"
#include "container.h"

#define XDG_SHELL_VERSION 2

struct server server;

static void init_event_handlers(struct server *server);
static void init_lists(struct server *server);
static void init_timers(struct server *server);
static void init_lua_api(struct server *server);

static void finalize_event_handlers(struct server *server);
static void finalize_lists(struct server *server);
static void finalize_timers(struct server *server);
static void finalize_lua_api(struct server *server);

static struct tag *handle_too_few_tags(uint32_t tag_id);

static struct tag *handle_too_few_tags(uint32_t tag_id) {
  // no number has more than 11 digits when int is 32 bit long
  char name[12];
  // TODO explain why +1
  snprintf(name, 12, "%d:%d", tag_id, c_idx_to_lua_idx(tag_id));

  struct tag *new_tag = create_tag(name, tag_id, server.default_layout);
  int *tag_id_ptr = malloc(sizeof(*tag_id_ptr));
  *tag_id_ptr = tag_id;
  g_hash_table_insert(server.tags, tag_id_ptr, new_tag);
  struct tag *tag = get_tag(0);
  wlr_list_cat(new_tag->con_set->tiled_containers,
               tag->con_set->tiled_containers);

  wlr_list_cat(new_tag->focus_set->focus_stack_layer_background,
               tag->focus_set->focus_stack_layer_background);
  wlr_list_cat(new_tag->focus_set->focus_stack_layer_bottom,
               tag->focus_set->focus_stack_layer_bottom);
  wlr_list_cat(new_tag->focus_set->focus_stack_layer_top,
               tag->focus_set->focus_stack_layer_top);
  wlr_list_cat(new_tag->focus_set->focus_stack_layer_overlay,
               tag->focus_set->focus_stack_layer_overlay);
  wlr_list_cat(new_tag->focus_set->focus_stack_on_top,
               tag->focus_set->focus_stack_on_top);
  wlr_list_cat(new_tag->focus_set->focus_stack_normal,
               tag->focus_set->focus_stack_normal);
  wlr_list_cat(new_tag->focus_set->focus_stack_not_focusable,
               tag->focus_set->focus_stack_not_focusable);

  wlr_list_cat(new_tag->visible_con_set->tiled_containers,
               tag->visible_con_set->tiled_containers);

  wlr_list_cat(new_tag->visible_focus_set->focus_stack_layer_background,
               tag->visible_focus_set->focus_stack_layer_background);
  wlr_list_cat(new_tag->visible_focus_set->focus_stack_layer_bottom,
               tag->visible_focus_set->focus_stack_layer_bottom);
  wlr_list_cat(new_tag->visible_focus_set->focus_stack_layer_top,
               tag->visible_focus_set->focus_stack_layer_top);
  wlr_list_cat(new_tag->visible_focus_set->focus_stack_layer_overlay,
               tag->visible_focus_set->focus_stack_layer_overlay);
  wlr_list_cat(new_tag->visible_focus_set->focus_stack_on_top,
               tag->visible_focus_set->focus_stack_on_top);
  wlr_list_cat(new_tag->visible_focus_set->focus_stack_normal,
               tag->visible_focus_set->focus_stack_normal);
  wlr_list_cat(new_tag->visible_focus_set->focus_stack_not_focusable,
               tag->visible_focus_set->focus_stack_not_focusable);

  return new_tag;
}

static void init_lists(struct server *server) {
  server->layer_visual_stack_lists = g_ptr_array_new();

  server->layer_visual_stack_background = g_ptr_array_new();
  server->layer_visual_stack_bottom = g_ptr_array_new();
  server->layer_visual_stack_top = g_ptr_array_new();
  server->layer_visual_stack_overlay = g_ptr_array_new();

  g_ptr_array_add(server->layer_visual_stack_lists,
                  server->layer_visual_stack_overlay);
  g_ptr_array_add(server->layer_visual_stack_lists,
                  server->layer_visual_stack_top);
  g_ptr_array_add(server->layer_visual_stack_lists,
                  server->layer_visual_stack_bottom);
  g_ptr_array_add(server->layer_visual_stack_lists,
                  server->layer_visual_stack_background);
}

static void finalize_lists(struct server *server) {
  g_ptr_array_unref(server->layer_visual_stack_background);
  g_ptr_array_unref(server->layer_visual_stack_bottom);
  g_ptr_array_unref(server->layer_visual_stack_top);
  g_ptr_array_unref(server->layer_visual_stack_overlay);

  g_ptr_array_unref(server->layer_visual_stack_lists);
}

static int clear_key_combo_timer_callback(void *data) {
  GPtrArray *registered_key_combos = server.registered_key_combos;
  char *bind = join_string((const char **)registered_key_combos->pdata,
                           registered_key_combos->len, " ");

  struct tag *tag = server_get_selected_tag();
  struct layout *lt = tag_get_layout(tag);

  process_binding(lt, bind);
  free(bind);

  list_clear(server.registered_key_combos, free);
  return 0;
}

static void init_timers(struct server *server) {
  server->combo_timer_source = wl_event_loop_add_timer(
      server->wl_event_loop, clear_key_combo_timer_callback,
      server->registered_key_combos);
}

static void finalize_timers(struct server *server) {
  wl_event_source_remove(server->combo_timer_source);
}

static int init_backend(struct server *server) {
  /* The Wayland display is managed by libwayland. It handles accepting
   * clients from the Unix socket, manging Wayland globals, and so on. */
  server->wl_display = wl_display_create();
  server->wl_event_loop = wl_display_get_event_loop(server->wl_display);

  init_timers(server);

  /* The backend is a wlroots feature which abstracts the underlying input and
   * output hardware. The autocreate option will choose the most suitable
   * backend based on the current environment, such as opening an X11 window
   * if an X11 server is running. The NULL argument here optionally allows you
   * to pass in a custom renderer if wlr_renderer doesnt). */
  if (!(server->backend = wlr_backend_autocreate(server->wl_display))) {
    printf("couldn't create backend\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

static void init_event_handlers(struct server *server) {
  LISTEN(&server->backend->events.new_output, &server->new_output,
         create_monitor);
  /* Use xdg_decoration protocol to negotiate server-side decorations */
  server->xdeco_mgr = wlr_xdg_decoration_manager_v1_create(server->wl_display);
  LISTEN(&server->xdeco_mgr->events.new_toplevel_decoration, &server->new_xdeco,
         createxdeco);

  server->xdg_shell =
      wlr_xdg_shell_create(server->wl_display, XDG_SHELL_VERSION);
  // remove csd(client side decorations) completely from xdg based windows
  wlr_server_decoration_manager_set_default_mode(
      wlr_server_decoration_manager_create(server->wl_display),
      WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);
  LISTEN(&server->xdg_shell->events.new_surface, &server->new_xdg_surface,
         create_notify_xdg);

  server->layer_shell = wlr_layer_shell_v1_create(server->wl_display);
  LISTEN(&server->layer_shell->events.new_surface,
         &server->new_layer_shell_surface, create_notify_layer_shell);

  server->pointer_constraints =
      wlr_pointer_constraints_v1_create(server->wl_display);
  LISTEN(&server->pointer_constraints->events.new_constraint,
         &server->new_pointer_constraint, handle_new_pointer_constraint);

  server->output_mgr = wlr_output_manager_v1_create(server->wl_display);
  LISTEN(&server->output_mgr->events.apply, &server->output_mgr_apply,
         handle_output_mgr_apply);
  LISTEN(&server->output_mgr->events.test, &server->output_mgr_test,
         handle_output_mgr_test);

  server->tablet_mgr = wlr_tablet_v2_create(server->wl_display);
}

static void finalize_event_handlers(struct server *server) {
  destroy_event_handler(server->event_handler);
}

void init_server() {
  server = (struct server){};

  server.registered_key_combos = g_ptr_array_new();
  server.named_key_combos = g_ptr_array_new();
  server.error_path = strdup("$HOME/.config/japokwm");
  expand_path(&server.error_path);

  init_lists(&server);

  server.mons = g_ptr_array_new();
  server.popups = g_ptr_array_new();
  server.xwayland_popups = g_ptr_array_new();

  server.scratchpad = g_ptr_array_new();
  server.keyboards = g_ptr_array_new();
  server.config_paths = create_default_config_paths();
  server.user_data_paths = create_default_user_data_paths();
  server.layout_paths = create_default_layout_paths();
  server.tags = create_tags();

  server.container_stack = g_ptr_array_new();

  server.event_handler = create_event_handler();

  server.previous_tag = 0;
  server.previous_bitset = bitset_create();

  server_prohibit_reloading_config();

  init_lua_api(&server);
  init_error_file();

  server.default_layout_ring = create_ring_buffer();
  server_reset_layout_ring(server.default_layout_ring);

  server.default_layout = create_layout(L);

  load_lua_api(L);
  if (init_backend(&server) != EXIT_SUCCESS) {
    return;
  }

  ipc_init(server.wl_event_loop);

  /* Creates an output layout, which a wlroots utility for working with an
   * arrangement of screens in a physical layout. */
  server.output_layout = wlr_output_layout_create();
}

void finalize_server() {
  g_ptr_array_unref(server.registered_key_combos);
  g_ptr_array_unref(server.named_key_combos);

  finalize_lists(&server);
  finalize_event_handlers(&server);

  bitset_destroy(server.previous_bitset);

  // NOTE: these bitsets are created lazily, so they may be NULL but that is
  // ok since bitset_destroy handles this case.
  bitset_destroy(server.tmp_bitset);
  bitset_destroy(server.local_tmp_bitset);

  g_ptr_array_unref(server.mons);
  g_ptr_array_unref(server.popups);
  g_ptr_array_unref(server.xwayland_popups);

  g_ptr_array_unref(server.scratchpad);
  g_ptr_array_unref(server.keyboards);
  g_ptr_array_unref(server.config_paths);
  g_ptr_array_unref(server.user_data_paths);
  g_ptr_array_unref(server.layout_paths);

  g_ptr_array_unref(server.container_stack);
}

void server_terminate(struct server *server) {
  server->is_running = false;
  wl_display_terminate(server->wl_display);
  uv_loop_close(server->uv_loop);
}

static void run_event_loop() {
  int pfd_size = 2;
  struct pollfd pfds[pfd_size];

  pfds[0].fd = wl_event_loop_get_fd(server.wl_event_loop);
  pfds[0].events = POLLIN;

  pfds[1].fd = uv_backend_fd(server.uv_loop);
  pfds[1].events = POLLIN;

  server.is_running = 1;
  while (server.is_running) {
    wl_display_flush_clients(server.wl_display);

    /* poll waits for any event of either the wayland event loop or the
     * libuv event loop and only if one emits an event we continue */
    poll(pfds, pfd_size, -1);

    // TODO: we can probably run this more efficiently
    uv_run(server.uv_loop, UV_RUN_NOWAIT);
    wl_event_loop_dispatch(server.wl_event_loop, 0);
  }
}

static void run(char *startup_cmd) {
  pid_t startup_pid = -1;

  /* Add a Unix socket to the Wayland display. */
  const char *socket = wl_display_add_socket_auto(server.wl_display);

  if (!socket)
    printf("startup: display_add_socket_auto\n");

  /* Set the WAYLAND_DISPLAY environment variable to our socket and run the
   * startup command if requested. */
  setenv("WAYLAND_DISPLAY", socket, 1);

  /* Start the backend. This will enumerate outputs and inputs, become the DRM
   * master, etc */
  if (!wlr_backend_start(server.backend)) {
    printf("Failed to start backend");
    wlr_backend_destroy(server.backend);
    return;
  }

  /* Now that outputs are initialized, choose initial selMon based on
   * cursor position, and set default cursor image */
  update_monitor_geometries();
  struct seat *seat = input_manager_get_default_seat();
  struct cursor *cursor = seat->cursor;
  struct monitor *m =
      xy_to_monitor(cursor->wlr_cursor->x, cursor->wlr_cursor->y);
  focus_monitor(m);

  /* XXX hack to get cursor to display in its initial location (100, 100)
   * instead of (0, 0) and then jumping.  still may not be fully
   * initialized, as the image/coordinates are not transformed for the
   * monitor when displayed here */
  wlr_cursor_warp_closest(seat->cursor->wlr_cursor, NULL, cursor->wlr_cursor->x,
                          cursor->wlr_cursor->y);
  wlr_xcursor_manager_set_cursor_image(seat->cursor->xcursor_mgr, "left_ptr",
                                       cursor->wlr_cursor);

  if (startup_cmd) {
    startup_pid = fork();
    if (startup_pid == 0) {
      execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void *)NULL);
    }
  }

  run_event_loop();

  if (startup_cmd) {
    kill(startup_pid, SIGTERM);
    waitpid(startup_pid, NULL, 0);
  }
}

static void init_lua_api(struct server *server) {
  L = luaL_newstate();
  luaL_openlibs(L);
  lua_setwarnf(L, handle_warning, NULL);
}

static void finalize_lua_api(struct server *server) { lua_close(L); }

void server_reset_layout_ring(struct ring_buffer *layout_ring) {
  list_clear(layout_ring->names, NULL);
  g_ptr_array_add(layout_ring->names, strdup("tile"));
  g_ptr_array_add(layout_ring->names, strdup("monocle"));
}

static void _async_handler_function(struct uv_async_s *arg) {
  // struct function_data *func_data = data->data;
  // printf("the end\n");
  // free(func_data->output);

  struct function_data *data = arg->data;

  lua_State *L = data->L;
  int func_ref = data->lua_func_ref;
  printf("the end %d\n", func_ref);

  lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);
  lua_pushstring(L, data->output);
  lua_call_safe(L, 1, 0, 0);
  luaL_unref(L, LUA_REGISTRYINDEX, func_ref);

  free(data);
}

int setup_server(struct server *server)
{
  server->uv_loop = uv_default_loop();

  uv_async_init(uv_default_loop(), &server->async_handler,
                _async_handler_function);

  /* If we don't provide a renderer, autocreate makes a GLES2 renderer for us.
   * The renderer is responsible for defining the various pixel formats it
   * supports for shared memory, this configures that for clients. */
  server->renderer = wlr_renderer_autocreate(server->backend);

  wlr_renderer_init_wl_display(server->renderer, server->wl_display);

  server->allocator =
      wlr_allocator_autocreate(server->backend, server->renderer);

  /* This creates some hands-off wlroots interfaces. The compositor is
   * necessary for clients to allocate surfaces and the data device manager
   * handles the clipboard. Each of these wlroots interfaces has room for you
   * to dig your fingers in and play with their behavior if you want. Note that
   * the clients cannot set the selection directly without compositor approval,
   * see the setsel() function. */
  server->compositor =
      wlr_compositor_create(server->wl_display, server->renderer);

  wlr_subcompositor_create(server->wl_display);

  wlr_export_dmabuf_manager_v1_create(server->wl_display);
  wlr_screencopy_manager_v1_create(server->wl_display);
  wlr_data_control_manager_v1_create(server->wl_display);
  wlr_data_device_manager_create(server->wl_display);
  wlr_gamma_control_manager_v1_create(server->wl_display);
  wlr_primary_selection_v1_device_manager_create(server->wl_display);
  wlr_viewporter_create(server->wl_display);
  wlr_idle_create(server->wl_display);
  wlr_idle_inhibit_v1_create(server->wl_display);

  wlr_xdg_output_manager_v1_create(server->wl_display, server->output_layout);

  /* Set up the xdg-shell. The xdg-shell is a
   * Wayland protocol which is used for application windows. For more
   * detail on shells, refer to the article:
   *
   * https://drewdevault.com/2018/07/29/Wayland-shells.html
   */

  server->input_inhibitor_mgr =
      wlr_input_inhibit_manager_create(server->wl_display);

  /* setup virtual pointer manager*/
  server->virtual_pointer_mgr =
      wlr_virtual_pointer_manager_v1_create(server->wl_display);

  /* setup virtual keyboard manager */
  server->virtual_keyboard_mgr =
      wlr_virtual_keyboard_manager_v1_create(server->wl_display);

  /* setup relative pointer manager */
  server->relative_pointer_mgr =
      wlr_relative_pointer_manager_v1_create(server->wl_display);
  /* wl_signal_add(&server.virtual_keyboard_mgr->events.new_virtual_keyboard,
   * &new_virtual_keyboard); */
  init_event_handlers(server);

  /*
   * Configures a seat, which is a single "seat" at which a user sits and
   * operates the computer. This conceptually includes up to one keyboard,
   * pointer, touch, and drawing tablet device. We also rig up a listener to
   * let us know when new input devices are available on the backend.
   */
  server->input_manager = create_input_manager();
  struct seat *seat = create_seat("seat0");
  g_ptr_array_add(server->input_manager->seats, seat);

  server->output_mgr = wlr_output_manager_v1_create(server->wl_display);
  wl_signal_add(&server->output_mgr->events.apply, &server->output_mgr_apply);
  wl_signal_add(&server->output_mgr->events.test, &server->output_mgr_test);

  server->tablet_v2 = wlr_tablet_v2_create(server->wl_display);

  server->scene = wlr_scene_create();
  struct wlr_scene *server_scene = server->scene;
  server->scene_background = wlr_scene_tree_create(&server_scene->tree);
  server->scene_tiled = wlr_scene_tree_create(&server_scene->tree);
  server->scene_floating = wlr_scene_tree_create(&server_scene->tree);
  server->scene_overlay = wlr_scene_tree_create(&server_scene->tree);

  server->new_surface.notify = server_handle_new_surface;
  wl_signal_add(&server->compositor->events.new_surface, &server->new_surface);

#ifdef JAPOKWM_HAS_XWAYLAND
  init_xwayland(server->wl_display, seat);
#endif

  return 0;
}

int start_server(char *startup_cmd) {
  if (setup_server(&server)) {
    printf("failed to setup japokwm\n");
    return EXIT_FAILURE;
  }

  run(startup_cmd);
  return EXIT_SUCCESS;
}

int finalize(struct server *server) {
  finalize_timers(server);
  destroy_layout(server->default_layout);
  destroy_ring_buffer(server->default_layout_ring);
  return 0;
}

int stop_server() {
#if JAPOKWM_HAS_XWAYLAND
  wlr_xwayland_destroy(server.xwayland.wlr_xwayland);
#endif
  wl_display_destroy_clients(server.wl_display);

  finalize(&server);

  close_error_file();
  wlr_output_layout_destroy(server.output_layout);
  wl_display_destroy(server.wl_display);
  destroy_input_manager(server.input_manager);
  destroy_tags(server.tags);

  finalize_lua_api(&server);
  return EXIT_SUCCESS;
}

int server_get_tag_count() {
  size_t len = server.default_layout->options->tag_names->len;
  return len;
}

int server_get_tag_key_count() {
  size_t count = g_hash_table_size(server.tags);
  return count;
}

GList *server_get_tags() {
  GList *values = g_hash_table_get_values(server.tags);
  return values;
}

struct tag *get_tag(int id) {
  if (id < 0)
    return NULL;
  struct tag *tag = g_hash_table_lookup(server.tags, &id);
  if (!tag) {
    tag = handle_too_few_tags(id);
    assert(tag != NULL);
  }

  return tag;
}

struct monitor *server_get_selected_monitor() {
  return server.selected_monitor;
}

void server_set_selected_monitor(struct monitor *m) {
  server.selected_monitor = m;
}

void server_center_default_cursor_in_monitor(struct monitor *m) {
  struct seat *seat = input_manager_get_default_seat();
  struct cursor *cursor = seat->cursor;
  center_cursor_in_monitor(cursor, m);
}

BitSet *server_bitset_get_tmp() {
  if (!server.tmp_bitset) {
    server.tmp_bitset = bitset_create();
  }
  return server.tmp_bitset;
}

BitSet *server_bitset_get_tmp_copy(BitSet *bitset) {
  BitSet *tmp = server_bitset_get_tmp();
  bitset_assign_bitset(&tmp, bitset);
  return tmp;
}

BitSet *server_bitset_get_local_tmp() {
  if (!server.local_tmp_bitset) {
    server.local_tmp_bitset = bitset_create();
  }

  return server.local_tmp_bitset;
}

BitSet *server_bitset_get_local_tmp_copy(BitSet *bitset) {
  BitSet *tmp = server_bitset_get_local_tmp();
  bitset_assign_bitset(&tmp, bitset);
  return tmp;
}

struct tag *server_get_selected_tag() {
    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    return tag;
}

struct layout *server_get_selected_layout() {
    struct tag *tag = server_get_selected_tag();
    struct layout *lt = tag_get_layout(tag);
    return lt;
}

void server_prohibit_reloading_config() {
    server.prohibit_reload_config = true;
}

void server_allow_reloading_config() { server.prohibit_reload_config = false; }

bool server_is_config_reloading_prohibited() {
    return server.prohibit_reload_config;
}

int cmp_str_bool(const void *s1, const void *s2) { return strcmp(s1, s2) == 0; }

void server_start_keycombo(const char *key_combo_name) {
    g_ptr_array_add(server.named_key_combos, strdup(key_combo_name));
}

bool server_is_keycombo(const char *key_combo_name) {
    guint pos = 0;
    bool found = g_ptr_array_find_with_equal_func(
            server.named_key_combos, key_combo_name, cmp_str_bool, &pos);
    return found;
}
