#include "server.h"

#include "layer_shell.h"
#include "monitor.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "xdg_shell.h"

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

static void init_event_handlers(struct server *server)
{
    server->new_output = (struct wl_listener){.notify = create_monitor};
    server->new_xdeco = (struct wl_listener){.notify = createxdeco};
    server->new_xdg_surface = (struct wl_listener){.notify = create_notify_xdg};
    server->new_layer_shell_surface = (struct wl_listener){.notify = create_notify_layer_shell};
    server->new_pointer_constraint = (struct wl_listener){.notify = handle_new_pointer_constraint};

#if JAPOKWM_HAS_XWAYLAND
    server->new_xwayland_surface = (struct wl_listener){.notify = create_notifyx11};
#endif
}

void init_server()
{
    server = (struct server) {
    };

    init_lists(&server);
    init_event_handlers(&server);

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

    server.floating_containers = g_ptr_array_new();
    server.focus_stack = g_ptr_array_new();

    g_ptr_array_add(server.client_lists, server.normal_clients);
    g_ptr_array_add(server.client_lists, server.non_tiled_clients);
    g_ptr_array_add(server.client_lists, server.independent_clients);

    server.tagsets = g_ptr_array_new();
    server.previous_workspace = 0;
}
