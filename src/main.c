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
#include <wlr/util/log.h>

#include "clipboard.h"
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

/* global event handlers */
static struct wl_listener cursor_axis = {.notify = axisnotify};
static struct wl_listener cursor_button = {.notify = buttonpress};
static struct wl_listener cursor_frame = {.notify = cursorframe};
static struct wl_listener cursor_motion = {.notify = motion_relative};
static struct wl_listener cursor_motion_absolute = {.notify = motion_absolute};
static struct wl_listener new_input = {.notify = handle_new_inputdevice};
static struct wl_listener new_output = {.notify = create_monitor};
static struct wl_listener new_xdeco = {.notify = createxdeco};
static struct wl_listener new_xdg_surface = {.notify = create_notify};
static struct wl_listener new_layer_shell_surface = {.notify = create_notify_layer_shell};
static struct wl_listener request_set_psel = {.notify = set_primary_selection};
static struct wl_listener request_set_sel = {.notify = set_selection};

static struct wl_listener new_xwayland_surface = {.notify = create_notifyx11};

static void cleanup()
{
    destroy_workspaces(&server.workspaces);

    close_error_file();
    wlr_xwayland_destroy(server.xwayland.wlr_xwayland);
    wl_display_destroy_clients(server.wl_display);

    wlr_xcursor_manager_destroy(server.cursor_mgr);
    wlr_cursor_destroy(server.cursor.wlr_cursor);
    wlr_output_layout_destroy(server.output_layout);
}

static void handle_new_inputdevice(struct wl_listener *listener, void *data)
{
    /* This event is raised by the backend when a new input device becomes
     * available. */
    struct wlr_input_device *device = data;
    uint32_t caps;
    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        create_keyboard(device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        create_pointer(device);
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

static void run(char *startup_cmd)
{
    pid_t startup_pid = -1;

    /* Add a Unix socket to the Wayland display. */
    const char *socket = wl_display_add_socket_auto(server.wl_display);

    if (!socket)
        wlr_log(WLR_INFO, "startup: display_add_socket_auto");

    /* Set the WAYLAND_DISPLAY environment variable to our socket and run the
     * startup command if requested. */
    setenv("WAYLAND_DISPLAY", socket, 1);

    /* Start the backend. This will enumerate outputs and inputs, become the DRM
     * master, etc */
    if (!wlr_backend_start(server.backend))
        wlr_log(WLR_INFO, "startup: backend_start");

    /* Now that outputs are initialized, choose initial selMon based on
     * cursor position, and set default cursor image */
    update_monitor_geometries();
    struct monitor *m = xy_to_monitor(server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y);
    focus_monitor(m);

    /* XXX hack to get cursor to display in its initial location (100, 100)
     * instead of (0, 0) and then jumping.  still may not be fully
     * initialized, as the image/coordinates are not transformed for the
     * monitor when displayed here */
    wlr_cursor_warp_closest(server.cursor.wlr_cursor, NULL, server.cursor.wlr_cursor->x, server.cursor.wlr_cursor->y);
    wlr_xcursor_manager_set_cursor_image(server.cursor_mgr, "left_ptr", server.cursor.wlr_cursor);

    if (startup_cmd) {
        startup_pid = fork();
        if (startup_pid < 0)
            wlr_log(WLR_ERROR, "startup: fork");
        if (startup_pid == 0) {
            execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void *)NULL);
            wlr_log(WLR_ERROR, "startup: execl");
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
    wl_list_init(&mons);
    wl_list_init(&popups);
    wl_list_init(&sticky_stack);

    wlr_list_init(&server.visual_stack_lists);
    wlr_list_init(&server.normal_visual_stack_lists);
    wlr_list_init(&server.layer_visual_stack_lists);

    wlr_list_init(&server.tiled_visual_stack);
    wlr_list_init(&server.floating_visual_stack);
    wlr_list_init(&server.layer_visual_stack_background);
    wlr_list_init(&server.layer_visual_stack_bottom);
    wlr_list_init(&server.layer_visual_stack_top);
    wlr_list_init(&server.layer_visual_stack_overlay);

    wlr_list_push(&server.visual_stack_lists, &server.layer_visual_stack_overlay);
    wlr_list_push(&server.visual_stack_lists, &server.layer_visual_stack_top);
    wlr_list_push(&server.visual_stack_lists, &server.floating_visual_stack);
    wlr_list_push(&server.visual_stack_lists, &server.tiled_visual_stack);
    wlr_list_push(&server.visual_stack_lists, &server.layer_visual_stack_bottom);
    wlr_list_push(&server.visual_stack_lists, &server.layer_visual_stack_background);

    wlr_list_push(&server.normal_visual_stack_lists, &server.floating_visual_stack);
    wlr_list_push(&server.normal_visual_stack_lists, &server.tiled_visual_stack);

    wlr_list_push(&server.layer_visual_stack_lists, &server.layer_visual_stack_overlay);
    wlr_list_push(&server.layer_visual_stack_lists, &server.layer_visual_stack_top);
    wlr_list_push(&server.layer_visual_stack_lists, &server.layer_visual_stack_bottom);
    wlr_list_push(&server.layer_visual_stack_lists, &server.layer_visual_stack_background);

    wlr_list_init(&server.scratchpad);
    wlr_list_init(&server.workspaces);

    wlr_list_init(&server.client_lists);

    wlr_list_init(&server.normal_clients);
    wlr_list_init(&server.independent_clients);

    wlr_list_push(&server.client_lists, &server.normal_clients);
    wlr_list_push(&server.client_lists, &server.independent_clients);

    L = luaL_newstate();
    luaL_openlibs(L);
    load_libs(L);
    init_error_file();

    server.default_layout = get_default_layout();
    server.layout_set = get_default_layout_set();

    init_utils(L);

    /* The Wayland display is managed by libwayland. It handles accepting
     * clients from the Unix socket, manging Wayland globals, and so on. */
    server.wl_display = wl_display_create();
    server.wl_event_loop = wl_display_get_event_loop(server.wl_display);
    ipc_init(server.wl_event_loop);

    /* The backend is a wlroots feature which abstracts the underlying input and
     * output hardware. The autocreate option will choose the most suitable
     * backend based on the current environment, such as opening an X11 window
     * if an X11 server is running. The NULL argument here optionally allows you
     * to pass in a custom renderer if wlr_renderer doesn't meet your needs. The
     * backend uses the renderer, for example, to fall back to software cursors
     * if the backend does not support hardware cursors (some older GPUs
     * don't). */
    if (!(server.backend = wlr_backend_autocreate(server.wl_display, NULL))) {
        wlr_log(WLR_INFO, "couldn't create backend");
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

    /* Creates an output layout, which a wlroots utility for working with an
     * arrangement of screens in a physical layout. */
    server.output_layout = wlr_output_layout_create();
    wlr_xdg_output_manager_v1_create(server.wl_display, server.output_layout);

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

    server.xdg_shell = wlr_xdg_shell_create(server.wl_display);
    wl_signal_add(&server.xdg_shell->events.new_surface, &new_xdg_surface);
    // remove csd(client side decorations) completely from xdg based windows
    wlr_server_decoration_manager_set_default_mode(
            wlr_server_decoration_manager_create(server.wl_display),
            WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);

    server.layer_shell = wlr_layer_shell_v1_create(server.wl_display);
    wl_signal_add(&server.layer_shell->events.new_surface,
            &new_layer_shell_surface);

    /* Use xdg_decoration protocol to negotiate server-side decorations */
    server.xdeco_mgr = wlr_xdg_decoration_manager_v1_create(server.wl_display);
    wl_signal_add(&server.xdeco_mgr->events.new_toplevel_decoration, &new_xdeco);

    /*
     * Creates a server.cursor, which is a wlroots utility for tracking the cursor
     * image shown on screen.
     */
    server.cursor.wlr_cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server.cursor.wlr_cursor, server.output_layout);

    /* Creates an xcursor manager, another wlroots utility which loads up
     * Xcursor themes to source cursor images from and makes sure that cursor
     * images are available at all scale factors on the screen (necessary for
     * HiDPI support). Scaled cursors will be loaded with each output. */
    server.cursor_mgr = wlr_xcursor_manager_create(NULL, 24);

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
    wl_signal_add(&server.cursor.wlr_cursor->events.motion, &cursor_motion);
    wl_signal_add(&server.cursor.wlr_cursor->events.motion_absolute,
            &cursor_motion_absolute);
    wl_signal_add(&server.cursor.wlr_cursor->events.button, &cursor_button);
    wl_signal_add(&server.cursor.wlr_cursor->events.axis, &cursor_axis);
    wl_signal_add(&server.cursor.wlr_cursor->events.frame, &cursor_frame);

    /*
     * Configures a seat, which is a single "seat" at which a user sits and
     * operates the computer. This conceptually includes up to one keyboard,
     * pointer, touch, and drawing tablet device. We also rig up a listener to
     * let us know when new input devices are available on the backend.
     */
    wl_list_init(&server.keyboards);
    wl_signal_add(&server.backend->events.new_input, &new_input);
    server.seat = wlr_seat_create(server.wl_display, "seat0");
    wl_signal_add(&server.seat->events.request_set_cursor, &request_set_cursor);
    wl_signal_add(&server.seat->events.request_set_selection, &request_set_sel);
    wl_signal_add(&server.seat->events.request_set_primary_selection, &request_set_psel);

    /*
     * Initialise the XWayland X server.
     * It will be started when the first X client is started.
     */
    server.xwayland.wlr_xwayland = wlr_xwayland_create(server.wl_display,
            server.compositor, true);
    if (server.xwayland.wlr_xwayland) {
        server.xwayland_ready.notify = handle_xwayland_ready;
        wl_signal_add(&server.xwayland.wlr_xwayland->events.ready, &server.xwayland_ready);
        wl_signal_add(&server.xwayland.wlr_xwayland->events.new_surface, &new_xwayland_surface);

        setenv("DISPLAY", server.xwayland.wlr_xwayland->display_name, true);
    } else {
        wlr_log(WLR_ERROR, "failed to setup XWayland X server, continuing without it");
        unsetenv("DISPLAY");
    }

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

void print_usage()
{
    printf("Usage: japokwm [options] [command]\n\n");
    print_help();
}

int main(int argc, char *argv[])
{
    char *startup_cmd = "";

    init_server();

    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"config", required_argument, NULL, 'c'},
        {"path", required_argument, NULL, 'p'},
        {"startup", no_argument, NULL, 's'},
        {0, 0, 0, 0}
    };

    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "h:c:p:s", long_options, &option_index)) != -1) {
        switch (c) {
            case 's':
                startup_cmd = optarg;
                break;
            case 'c':
                server.config_file = optarg;
                break;
            case 'p':
                server.config_dir = optarg;
                break;
            case 'h':
                print_help();
                return EXIT_SUCCESS;
                break;
            default:
                print_usage();
                return EXIT_SUCCESS;
        }
    }
    if (optind < argc) {
        print_usage();
        return EXIT_SUCCESS;
    }

    // TODO delete to increase performance
    setbuf(stdout, NULL);

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
}
