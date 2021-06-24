#include "server.h"
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

    wlr_list_init(&server.mons);
    wlr_list_init(&server.popups);
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
    wlr_list_init(&server.non_tiled_clients);
    wlr_list_init(&server.independent_clients);

    wlr_list_push(&server.client_lists, &server.normal_clients);
    wlr_list_push(&server.client_lists, &server.non_tiled_clients);
    wlr_list_push(&server.client_lists, &server.independent_clients);

    wlr_list_init(&server.tagsets);

    wlr_list_init(&server.messages);
}
