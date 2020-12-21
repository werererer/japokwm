#ifndef _SWAY_IPC_SERVER_H
#define _SWAY_IPC_SERVER_H
#include <sys/socket.h>
#include <wayland-server.h>

void ipc_init(struct wl_event_loop *wl_event_loop);

struct sockaddr_un *ipc_user_sockaddr(void);

void ipc_event_workspace();
#endif
