#include "server.h"
#include "monitor.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"

struct server server;

static void init_lists(struct server *m);

static void init_lists(struct server *server)
{
    server->visual_stack_lists = g_ptr_array_new();
    server->normal_visual_stack_lists = g_ptr_array_new();
    server->layer_visual_stack_lists = g_ptr_array_new();

    server->tiled_visual_stack = g_ptr_array_new();
    server->floating_visual_stack = g_ptr_array_new();
    server->layer_visual_stack_background = g_ptr_array_new();
    server->layer_visual_stack_bottom = g_ptr_array_new();
    server->layer_visual_stack_top = g_ptr_array_new();
    server->layer_visual_stack_overlay = g_ptr_array_new();

    g_ptr_array_add(server->visual_stack_lists, server->layer_visual_stack_overlay);
    g_ptr_array_add(server->visual_stack_lists, server->layer_visual_stack_top);
    g_ptr_array_add(server->visual_stack_lists, server->floating_visual_stack);
    g_ptr_array_add(server->visual_stack_lists, server->tiled_visual_stack);
    g_ptr_array_add(server->visual_stack_lists, server->layer_visual_stack_bottom);
    g_ptr_array_add(server->visual_stack_lists, server->layer_visual_stack_background);

    g_ptr_array_add(server->normal_visual_stack_lists, server->floating_visual_stack);
    g_ptr_array_add(server->normal_visual_stack_lists, server->tiled_visual_stack);

    g_ptr_array_add(server->layer_visual_stack_lists, server->layer_visual_stack_overlay);
    g_ptr_array_add(server->layer_visual_stack_lists, server->layer_visual_stack_top);
    g_ptr_array_add(server->layer_visual_stack_lists, server->layer_visual_stack_bottom);
    g_ptr_array_add(server->layer_visual_stack_lists, server->layer_visual_stack_background);
}

void init_server()
{
    server = (struct server) {
        .previous_tagset = NULL,
    };

    init_lists(&server);

    wl_list_init(&sticky_stack);


    server.mons = g_ptr_array_new();
    server.popups = g_ptr_array_new();
    server.xwayland_popups = g_ptr_array_new();

    server.scratchpad = g_ptr_array_new();
    server.keyboards = g_ptr_array_new();
    server.config_paths = create_default_config_paths();
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
