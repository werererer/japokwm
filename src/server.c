#include "server.h"
#include "utils/coreUtils.h"

struct server server;

void init_server()
{
    server = (struct server) {
        .config_file = "",
        .config_dir = "",
        .previous_workspace_id = INVALID_POSITION,
    };
}
