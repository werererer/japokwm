#include "server.h"
#include "monitor.h"
#include "utils/coreUtils.h"

struct server server;

void init_server()
{
    server = (struct server) {
        .config_file = "",
        .config_dir = "",
        .previous_tagset = NULL,
    };

    wl_list_init(&sticky_stack);

    server.mons = g_ptr_array_new();
    server.popups = g_ptr_array_new();
    server.xwayland_popups = g_ptr_array_new();

    server.scratchpad = g_ptr_array_new();
    server.workspaces = g_ptr_array_new();

    server.client_lists = g_ptr_array_new();

    server.normal_clients = g_ptr_array_new();
    server.non_tiled_clients = g_ptr_array_new();
    server.independent_clients = g_ptr_array_new();

    g_ptr_array_add(server.client_lists, server.normal_clients);
    g_ptr_array_add(server.client_lists, server.non_tiled_clients);
    g_ptr_array_add(server.client_lists, server.independent_clients);

    server.tagsets = g_ptr_array_new();
}
