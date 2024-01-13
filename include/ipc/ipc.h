#ifndef IPC_H
#define IPC_H

#include <sys/socket.h>

#include "server.h"

#define CREATE_EVENT_BITMASK(ev) (1 << (ev & 0x7F))

static const char ipc_magic[] = {'i', '3', '-', 'i', 'p', 'c'};

#define IPC_HEADER_SIZE (sizeof(ipc_magic) + 8)

enum ipc_command_type {
    IPC_COMMAND = 0,
    IPC_GET_TAGS = 1,
    IPC_SUBSCRIBE = 2,
    IPC_GET_TREE = 4,
    IPC_GET_BAR_CONFIG = 6,

    // Event Types
    IPC_EVENT_TAG = ((1<<31) | 0),
    IPC_EVENT_MODE = ((1<<31) | 2),
    IPC_EVENT_WINDOW = ((1<<31) | 3),
    IPC_EVENT_BARCONFIG_UPDATE = ((1<<31) | 4),
    IPC_EVENT_BINDING = ((1<<31) | 5),
    IPC_EVENT_SHUTDOWN = ((1<<31) | 6),
    IPC_EVENT_TICK = ((1<<31) | 7),
};
;


struct ipc_client {
    struct wl_event_source *event_source;
    struct wl_event_source *writable_event_source;
    int fd;
    enum ipc_command_type subscribed_events;
    size_t write_buffer_len;
    size_t write_buffer_size;
    char *write_buffer;
    // The following are for storing data between event_loop calls
    uint32_t pending_length;
    enum ipc_command_type pending_type;
};

void ipc_init(struct wl_event_loop *wl_event_loop);
int ipc_client_handle_readable(int client_fd, uint32_t mask, void *data);
int ipc_client_handle_writable(int client_fd, uint32_t mask, void *data);
bool ipc_send_reply(struct ipc_client *client,
                    enum ipc_command_type payload_type, const char *payload,
                    uint32_t payload_length);
void ipc_send_event(const char *json_string, enum ipc_command_type event);
int ipc_handle_connection(int fd, uint32_t mask, void *data);
void ipc_client_disconnect(struct ipc_client *client);

struct sockaddr_un *ipc_user_sockaddr(void);


#endif // IPC_H
