/*
 * See LICENSE file for copyright and license details.
 */

#include <getopt.h>
#include <systemd/sd-bus.h>
#include <unistd.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_primary_selection_v1.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_idle.h>
#include <wlr/types/wlr_idle_inhibit_v1.h>
#include <wlr/types/wlr_input_inhibitor.h>
#include <wlr/util/log.h>

#include "ipc-server.h"
#include "keyboard.h"
#include "layer_shell.h"
#include "render/render.h"
#include "scratchpad.h"
#include "server.h"
#include "translationLayer.h"
#include "utils/parseConfigUtils.h"
#include "xdg_shell.h"

/* Used to move all of the data necessary to render a surface from the top-level
 * frame handler to the per-surface render function. */
/* function declarations */
static void cleanup();
static void handle_new_inputdevice(struct wl_listener *listener, void *data);
static void run(char *startup_cmd);
static int setup();

static void cleanup()
{
    close_error_file();
#if JAPOKWM_HAS_XWAYLAND
    wlr_xwayland_destroy(server.xwayland.wlr_xwayland);
#endif
    wl_display_destroy_clients(server.wl_display);

    wlr_output_layout_destroy(server.output_layout);
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
    if (!wlr_backend_start(server.backend))
        printf("startup: backend_start");

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

static int setup()
{
    L = luaL_newstate();
    load_lua_api(L);
    init_error_file();

    server.layout_set = get_default_layout_set();

    init_utils(L);

    /* The Wayland display is managed by libwayland. It handles accepting
     * clients from the Unix socket, manging Wayland globals, and so on. */
    server.wl_display = wl_display_create();
    server.wl_event_loop = wl_display_get_event_loop(server.wl_display);
    ipc_init(server.wl_event_loop);
    server.default_layout = create_layout(L);
    load_config(L);

    /* The backend is a wlroots feature which abstracts the underlying input and
     * output hardware. The autocreate option will choose the most suitable
     * backend based on the current environment, such as opening an X11 window
     * if an X11 server is running. The NULL argument here optionally allows you
     * to pass in a custom renderer if wlr_renderer doesnt). */
    if (!(server.backend = wlr_backend_autocreate(server.wl_display))) {
        printf("couldn't create backend\n");
        return EXIT_FAILURE;
    }

    /* If we don't provide a renderer, autocreate makes a GLES2 renderer for us.
     * The renderer is responsible for defining the various pixel formats it
     * supports for shared memory, this configures that for clients. */
    drw = wlr_backend_get_renderer(server.backend);
    wlr_renderer_init_wl_display(drw, server.wl_display);

    /* This creates some hands-off wlroots interfaces. The compositor is
     * necessary for clients to allocate surfaces and the data device manager
     * handles the clipboard. Each of these wlroots interfaces has room for you
     * to dig your fingers in and play with their behavior if you want. Note that
     * the clients cannot set the selection directly without compositor approval,
     * see the setsel() function. */
    server.compositor = wlr_compositor_create(server.wl_display, drw);
    wlr_export_dmabuf_manager_v1_create(server.wl_display);
    wlr_screencopy_manager_v1_create(server.wl_display);
    wlr_data_device_manager_create(server.wl_display);
    wlr_gamma_control_manager_v1_create(server.wl_display);
    wlr_primary_selection_v1_device_manager_create(server.wl_display);
    wlr_viewporter_create(server.wl_display);
    wlr_idle_create(server.wl_display);
    wlr_idle_inhibit_v1_create(server.wl_display);

    /* Creates an output layout, which a wlroots utility for working with an
     * arrangement of screens in a physical layout. */
    server.output_layout = wlr_output_layout_create();
    wlr_xdg_output_manager_v1_create(server.wl_display, server.output_layout);

    /* Configure a listener to be notified when new outputs are available on the
     * backend. */
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    /* Set up our client lists and the xdg-shell. The xdg-shell is a
     * Wayland protocol which is used for application windows. For more
     * detail on shells, refer to the article:
     *
     * https://drewdevault.com/2018/07/29/Wayland-shells.html
     */

    server.xdg_shell = wlr_xdg_shell_create(server.wl_display);
    wl_signal_add(&server.xdg_shell->events.new_surface, &server.new_xdg_surface);
    // remove csd(client side decorations) completely from xdg based windows
    wlr_server_decoration_manager_set_default_mode(
            wlr_server_decoration_manager_create(server.wl_display),
            WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);

    server.layer_shell = wlr_layer_shell_v1_create(server.wl_display);
    wl_signal_add(&server.layer_shell->events.new_surface,
            &server.new_layer_shell_surface);

    server.input_inhibitor_mgr = wlr_input_inhibit_manager_create(server.wl_display);

    /* setup virtual pointer manager*/
    server.virtual_pointer_mgr = wlr_virtual_pointer_manager_v1_create(server.wl_display);

    /* setup virtual keyboard manager */
    server.virtual_keyboard_mgr = wlr_virtual_keyboard_manager_v1_create(server.wl_display);

    /* setup relative pointer manager */
    server.relative_pointer_mgr = wlr_relative_pointer_manager_v1_create(server.wl_display);
    /* wl_signal_add(&server.virtual_keyboard_mgr->events.new_virtual_keyboard, &new_virtual_keyboard); */

    /* Use xdg_decoration protocol to negotiate server-side decorations */
    server.xdeco_mgr = wlr_xdg_decoration_manager_v1_create(server.wl_display);
    wl_signal_add(&server.xdeco_mgr->events.new_toplevel_decoration, &server.new_xdeco);

    server.pointer_constraints = wlr_pointer_constraints_v1_create(server.wl_display);
    wl_signal_add(&server.pointer_constraints->events.new_constraint, &server.new_pointer_constraint);

    /*
     * Configures a seat, which is a single "seat" at which a user sits and
     * operates the computer. This conceptually includes up to one keyboard,
     * pointer, touch, and drawing tablet device. We also rig up a listener to
     * let us know when new input devices are available on the backend.
     */
    server.input_manager = create_input_manager();
    struct seat *seat = create_seat("seat0");

#ifdef JAPOKWM_HAS_XWAYLAND
    init_xwayland(server.wl_display, seat);
#endif
    return 0;
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

void print_help()
{
    printf("  -h, --help             Show help message and quit.\n"
            "  -c, --config <config>  Specify a config file.\n"
            "  -s, --startup          Specify the program which is executed on startup\n"
            "\n");
}

void print_version()
{
    printf("japokwm "JAPOKWM_VERSION"\n");
}

void print_usage()
{
    printf("Usage: japokwm [options] [command]\n\n");
    print_help();
}

int main(int argc, char *argv[])
{
#if DEBUG
    setbuf(stdout, NULL);
#endif

    init_server();

    char *startup_cmd = "";

    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"config", required_argument, NULL, 'c'},
        {"path", required_argument, NULL, 'p'},
        {"startup", no_argument, NULL, 's'},
        {"version", no_argument, NULL, 'v'},
        {0, 0, 0, 0}
    };

    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "h:c:p:s:v", long_options, &option_index)) != -1) {
        switch (c) {
            case 's':
                startup_cmd = optarg;
                break;
            case 'c':
                server.config_file = optarg;
                break;
            case 'p':
                g_ptr_array_insert(server.config_paths, 0, optarg);
                break;
            case 'h':
                print_help();
                return EXIT_SUCCESS;
                break;
            case 'v':
                print_version();
                return EXIT_SUCCESS;
            default:
                print_usage();
                return EXIT_SUCCESS;
        }
    }
    if (optind < argc) {
        print_usage();
        return EXIT_SUCCESS;
    }

    // Wayland requires XDG_RUNTIME_DIR for creating its communications
    // socket
    if (!getenv("XDG_RUNTIME_DIR")) {
        fprintf(stderr,
                "XDG_RUNTIME_DIR is not set in the environment. Aborting.\n");
        return EXIT_FAILURE;
    }
    if (setup()) {
        printf("failed to setup japokwm\n");
        return EXIT_FAILURE;
    }

    run(startup_cmd);
    cleanup();
    return EXIT_SUCCESS;
}
