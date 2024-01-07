#ifndef SWAY_IPC_SERVER_H
#define SWAY_IPC_SERVER_H
#include <sys/socket.h>
#include <wayland-server.h>

#include "ipc/ipc.h"

void ipc_init(struct wl_event_loop *wl_event_loop);
bool ipc_send_reply(struct ipc_client *client, enum ipc_command_type payload_type,
    const char *payload, uint32_t payload_length);

struct sockaddr_un *ipc_user_sockaddr(void);

void ipc_event_tag();
void ipc_event_window();
int handle_client_payload(struct ipc_client *client);

#endif //SWAY_IPC_SERVER_H
