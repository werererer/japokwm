#include "server.h"

struct server server;

void init_server()
{
    server = (struct server) {
        .config_file = "",
    };
}
