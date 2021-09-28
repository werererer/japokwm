#include "server.h"

#include "layer_shell.h"
#include "monitor.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "xdg_shell.h"

struct server server;

static void init_event_handlers(struct server *server);
static void init_lists(struct server *server);
static void init_timers(struct server *server);

static void finalize_event_handlers(struct server *server);
static void finalize_lists(struct server *server);
static void finalize_timers(struct server *server);

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
    g_ptr_array_free(server->layer_visual_stack_background, FALSE);
    g_ptr_array_free(server->layer_visual_stack_bottom, FALSE);
    g_ptr_array_free(server->layer_visual_stack_top, FALSE);
    g_ptr_array_free(server->layer_visual_stack_overlay, FALSE);

    g_ptr_array_free(server->layer_visual_stack_lists, FALSE);
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

static void finalize_event_handlers(struct server *server)
{
    // TODO: write that one
}

void init_server()
{
    server = (struct server) {
    };

    server.registered_key_combos = g_ptr_array_new();
    server.named_key_combos = g_ptr_array_new();

    init_lists(&server);
    init_timers(&server);
    init_event_handlers(&server);

    server.mons = g_ptr_array_new();
    server.popups = g_ptr_array_new();
    server.xwayland_popups = g_ptr_array_new();

    server.scratchpad = g_ptr_array_new();
    server.keyboards = g_ptr_array_new();
    server.config_paths = create_default_config_paths();
    server.workspaces = g_ptr_array_new();

    server.client_lists = g_ptr_array_new();

    server.normal_clients = g_ptr_array_new();
    server.independent_clients = g_ptr_array_new();

    server.floating_containers = g_ptr_array_new();
    server.floating_stack = g_ptr_array_new();

    g_ptr_array_add(server.client_lists, server.normal_clients);
    g_ptr_array_add(server.client_lists, server.independent_clients);

    server.tagsets = g_ptr_array_new();
    server.previous_workspace = 0;
}

void finalize_server()
{
    // TODO: fix all lines that are commented out so that they to the opposite
    // of init_server()
    g_ptr_array_free(server.registered_key_combos, TRUE);
    g_ptr_array_free(server.named_key_combos, TRUE);

    finalize_lists(&server);
    finalize_timers(&server);
    /* init_event_handlers(&server); */

    g_ptr_array_free(server.mons, TRUE);
    /* server.popups = g_ptr_array_new(); */
    /* server.xwayland_popups = g_ptr_array_new(); */

    /* server.scratchpad = g_ptr_array_new(); */
    /* server.keyboards = g_ptr_array_new(); */
    /* server.config_paths = create_default_config_paths(); */
    /* server.workspaces = g_ptr_array_new(); */

    g_ptr_array_free(server.client_lists, FALSE);

    g_ptr_array_free(server.normal_clients, FALSE);
    g_ptr_array_free(server.independent_clients, FALSE);

    /* server.floating_containers = g_ptr_array_new(); */
    /* server.floating_stack = g_ptr_array_new(); */

    /* g_ptr_array_add(server.client_lists, server.normal_clients); */
    /* g_ptr_array_add(server.client_lists, server.non_tiled_clients); */
    /* g_ptr_array_add(server.client_lists, server.independent_clients); */

    g_ptr_array_free(server.tagsets, TRUE);
}
