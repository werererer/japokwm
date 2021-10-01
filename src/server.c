#include "server.h"

#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_primary_selection_v1.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_idle.h>
#include <wlr/types/wlr_idle_inhibit_v1.h>
#include <wlr/types/wlr_input_inhibitor.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_viewporter.h>
#include <unistd.h>
#include <wait.h>
#include <wayland-client.h>

#include "layer_shell.h"
#include "monitor.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "xdg_shell.h"
#include "translationLayer.h"
#include "ipc-server.h"
#include "render/render.h"

struct server server;

static void init_event_handlers(struct server *server);
static void init_lists(struct server *server);
static void init_timers(struct server *server);
static void init_lua_api(struct server *server);

static void finalize_event_handlers(struct server *server);
static void finalize_lists(struct server *server);
static void finalize_timers(struct server *server);
static void finalize_lua_api(struct server *server);

static void init_lists(struct server *server)
{
    server->layer_visual_stack_lists = g_ptr_array_new();

    server->layer_visual_stack_background = g_ptr_array_new();
    server->layer_visual_stack_bottom = g_ptr_array_new();
    server->layer_visual_stack_top = g_ptr_array_new();
    server->layer_visual_stack_overlay = g_ptr_array_new();

    g_ptr_array_add(server->layer_visual_stack_lists, server->layer_visual_stack_overlay);
    g_ptr_array_add(server->layer_visual_stack_lists, server->layer_visual_stack_top);
    g_ptr_array_add(server->layer_visual_stack_lists, server->layer_visual_stack_bottom);
    g_ptr_array_add(server->layer_visual_stack_lists, server->layer_visual_stack_background);
}

static void finalize_lists(struct server *server)
{
    g_ptr_array_unref(server->layer_visual_stack_background);
    g_ptr_array_unref(server->layer_visual_stack_bottom);
    g_ptr_array_unref(server->layer_visual_stack_top);
    g_ptr_array_unref(server->layer_visual_stack_overlay);

    g_ptr_array_unref(server->layer_visual_stack_lists);
}


static void clear_key_combo_timer_callback(union sigval sev) {
    list_clear(server.registered_key_combos, free);
}

static void init_timers(struct server *server)
{
    server->combo_sig_event.sigev_notify = SIGEV_THREAD;
    server->combo_sig_event.sigev_notify_function = &clear_key_combo_timer_callback;

    timer_create(CLOCK_REALTIME, &server->combo_sig_event, &server->combo_timer);
}

static void finalize_timers(struct server *server)
{
    timer_delete(&server->combo_sig_event);
}

static int init_backend(struct server *server)
{
    /* The Wayland display is managed by libwayland. It handles accepting
     * clients from the Unix socket, manging Wayland globals, and so on. */
    server->wl_display = wl_display_create();
    server->wl_event_loop = wl_display_get_event_loop(server->wl_display);

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

static void init_event_handlers(struct server *server)
{
    LISTEN(&server->backend->events.new_output, &server->new_output, create_monitor);
    /* Use xdg_decoration protocol to negotiate server-side decorations */
    server->xdeco_mgr = wlr_xdg_decoration_manager_v1_create(server->wl_display);
    LISTEN(&server->xdeco_mgr->events.new_toplevel_decoration, &server->new_xdeco, createxdeco);

    server->xdg_shell = wlr_xdg_shell_create(server->wl_display);
    // remove csd(client side decorations) completely from xdg based windows
    wlr_server_decoration_manager_set_default_mode(
            wlr_server_decoration_manager_create(server->wl_display),
            WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);
    LISTEN(&server->xdg_shell->events.new_surface, &server->new_xdg_surface, create_notify_xdg);

    server->layer_shell = wlr_layer_shell_v1_create(server->wl_display);
    LISTEN(&server->layer_shell->events.new_surface, &server->new_layer_shell_surface, create_notify_layer_shell);

    server->pointer_constraints = wlr_pointer_constraints_v1_create(server->wl_display);
    LISTEN(&server->pointer_constraints->events.new_constraint, &server->new_pointer_constraint, handle_new_pointer_constraint);
}

static void finalize_event_handlers(struct server *server)
{
    // TODO: write that one
}

void init_server()
{
    server = (struct server) {
        .xwayland = {},
    };

    server.registered_key_combos = g_ptr_array_new();
    server.named_key_combos = g_ptr_array_new();

    init_lists(&server);
    init_timers(&server);

    server.mons = g_ptr_array_new();
    server.popups = g_ptr_array_new();
    server.xwayland_popups = g_ptr_array_new();

    server.scratchpad = g_ptr_array_new();
    server.keyboards = g_ptr_array_new();
    server.config_paths = create_default_config_paths();
    server.workspaces = g_ptr_array_new();

    server.container_stack = g_ptr_array_new();


    server.tagsets = g_ptr_array_new();
    server.previous_workspace = 0;
}

void finalize_server()
{
    g_ptr_array_unref(server.registered_key_combos);
    g_ptr_array_unref(server.named_key_combos);

    finalize_lists(&server);
    finalize_timers(&server);

    g_ptr_array_unref(server.mons);
    g_ptr_array_unref(server.popups);
    g_ptr_array_unref(server.xwayland_popups);

    g_ptr_array_unref(server.scratchpad);
    g_ptr_array_unref(server.keyboards);
    g_ptr_array_unref(server.config_paths);

    g_ptr_array_unref(server.container_stack);

    g_ptr_array_unref(server.tagsets);
}

static void run(char *startup_cmd)
{
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
    struct monitor *m = xy_to_monitor(cursor->wlr_cursor->x, cursor->wlr_cursor->y);
    focus_monitor(m);

    /* XXX hack to get cursor to display in its initial location (100, 100)
     * instead of (0, 0) and then jumping.  still may not be fully
     * initialized, as the image/coordinates are not transformed for the
     * monitor when displayed here */
    wlr_cursor_warp_closest(seat->cursor->wlr_cursor, NULL, cursor->wlr_cursor->x, cursor->wlr_cursor->y);
    wlr_xcursor_manager_set_cursor_image(seat->cursor->xcursor_mgr, "left_ptr", cursor->wlr_cursor);

    if (startup_cmd) {
        startup_pid = fork();
        if (startup_pid == 0) {
            execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void *)NULL);
        }
    }
    /* Run the Wayland event loop. This does not return until you exit the
     * compositor. Starting the backend rigged up all of the necessary event
     * loop configuration to listen to libinput events, DRM events, generate
     * frame events at the refresh rate, and so on. */
    wl_display_run(server.wl_display);

    if (startup_cmd) {
        kill(startup_pid, SIGTERM);
        waitpid(startup_pid, NULL, 0);
    }
}

static void init_lua_api(struct server *server)
{
    L = luaL_newstate();
    load_lua_api(L);
}

static void finalize_lua_api(struct server *server)
{
    lua_close(L);
}

int setup(struct server *server)
{
    init_lua_api(server);
    init_error_file();

    server->layout_set = get_default_layout_set();

    init_utils(L);

    if (init_backend(server) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    ipc_init(server->wl_event_loop);

    server->default_layout = create_layout(L);
    load_config(L);

    /* If we don't provide a renderer, autocreate makes a GLES2 renderer for us.
     * The renderer is responsible for defining the various pixel formats it
     * supports for shared memory, this configures that for clients. */
    drw = wlr_backend_get_renderer(server->backend);
    wlr_renderer_init_wl_display(drw, server->wl_display);

    /* This creates some hands-off wlroots interfaces. The compositor is
     * necessary for clients to allocate surfaces and the data device manager
     * handles the clipboard. Each of these wlroots interfaces has room for you
     * to dig your fingers in and play with their behavior if you want. Note that
     * the clients cannot set the selection directly without compositor approval,
     * see the setsel() function. */
    server->compositor = wlr_compositor_create(server->wl_display, drw);
    wlr_export_dmabuf_manager_v1_create(server->wl_display);
    wlr_screencopy_manager_v1_create(server->wl_display);
    wlr_data_device_manager_create(server->wl_display);
    wlr_gamma_control_manager_v1_create(server->wl_display);
    wlr_primary_selection_v1_device_manager_create(server->wl_display);
    wlr_viewporter_create(server->wl_display);
    wlr_idle_create(server->wl_display);
    wlr_idle_inhibit_v1_create(server->wl_display);

    /* Creates an output layout, which a wlroots utility for working with an
     * arrangement of screens in a physical layout. */
    server->output_layout = wlr_output_layout_create();
    wlr_xdg_output_manager_v1_create(server->wl_display, server->output_layout);

    /* Set up the xdg-shell. The xdg-shell is a
     * Wayland protocol which is used for application windows. For more
     * detail on shells, refer to the article:
     *
     * https://drewdevault.com/2018/07/29/Wayland-shells.html
     */

    server->input_inhibitor_mgr = wlr_input_inhibit_manager_create(server->wl_display);

    /* setup virtual pointer manager*/
    server->virtual_pointer_mgr = wlr_virtual_pointer_manager_v1_create(server->wl_display);

    /* setup virtual keyboard manager */
    server->virtual_keyboard_mgr = wlr_virtual_keyboard_manager_v1_create(server->wl_display);

    /* setup relative pointer manager */
    server->relative_pointer_mgr = wlr_relative_pointer_manager_v1_create(server->wl_display);
    /* wl_signal_add(&server.virtual_keyboard_mgr->events.new_virtual_keyboard, &new_virtual_keyboard); */
    init_event_handlers(server);

    /*
     * Configures a seat, which is a single "seat" at which a user sits and
     * operates the computer. This conceptually includes up to one keyboard,
     * pointer, touch, and drawing tablet device. We also rig up a listener to
     * let us know when new input devices are available on the backend.
     */
    server->input_manager = create_input_manager();
    struct seat *seat = create_seat("seat0");

#ifdef JAPOKWM_HAS_XWAYLAND
    init_xwayland(server->wl_display, seat);
#endif
    return 0;
}

int start_server(char *startup_cmd)
{
    if (setup(&server)) {
        printf("failed to setup japokwm\n");
        return EXIT_FAILURE;
    }

    run(startup_cmd);
    return EXIT_SUCCESS;
}

int finalize(struct server *server)
{
    destroy_layout(server->default_layout);
    return 0;
}

int stop_server()
{
#if JAPOKWM_HAS_XWAYLAND
    wlr_xwayland_destroy(server.xwayland.wlr_xwayland);
#endif
    wl_display_destroy_clients(server.wl_display);
    for (int i = 0; i < server.input_manager->seats->len; i++) {
        struct seat *seat = g_ptr_array_steal_index(server.input_manager->seats, 0);
        destroy_seat(seat);
    }

    finalize_lua_api(&server);

    close_error_file();
    wlr_output_layout_destroy(server.output_layout);
    wl_display_destroy(server.wl_display);

    destroy_workspaces(server.workspaces);

    return EXIT_SUCCESS;
}

struct monitor *server_get_selected_monitor()
{
    return server.selected_monitor;
}

void server_set_selected_monitor(struct monitor *m)
{
    server.selected_monitor = m;
}
