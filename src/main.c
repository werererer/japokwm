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
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_idle_inhibit_v1.h>
#include <wlr/types/wlr_input_inhibitor.h>
#include <wlr/util/log.h>
#include <signal.h>

#include "server.h"
#include "stringop.h"

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

    /* If nobody is reading the status output, don't terminate */
    struct sigaction sigint_action = {.sa_handler = SIG_IGN};
    sigaction(SIGPIPE, &sigint_action, NULL);

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
            case 'c':
                server.config_file = optarg;
                break;
            case 'p':
                server.custom_path = strdup(optarg);
                expand_path(&server.custom_path);
                g_ptr_array_insert(server.config_paths, 0, server.custom_path);
                g_ptr_array_insert(server.layout_paths, 0, server.custom_path);
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
    int status = start_server(startup_cmd);
    if (status == 0) {
        stop_server();
    }
    finalize_server();
    return status;
}
