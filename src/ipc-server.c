// See https://i3wm.org/docs/ipc.html for protocol information
#include "json_object.h"
#include "json_types.h"
#include <linux/input-event-codes.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <json.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_list.h>
#include "ipc-json.h"
#include "ipc-server.h"
#include "server.h"
#include "workspaceset.h"
#include "client.h"
#include "command.h"
#include "monitor.h"
#include "root.h"

static int ipc_socket = -1;
static struct sockaddr_un *ipc_sockaddr = NULL;
static struct wlr_list ipc_client_list;

static const char ipc_magic[] = {'i', '3', '-', 'i', 'p', 'c'};

#define IPC_HEADER_SIZE (sizeof(ipc_magic) + 8)

#define event_mask(ev) (1 << (ev & 0x7F))
enum ipc_command_type {
    // i3 command types - see i3's I3_REPLY_TYPE constants
    IPC_COMMAND = 0,
    IPC_GET_WORKSPACES = 1,
    IPC_SUBSCRIBE = 2,
    IPC_GET_OUTPUTS = 3,
    IPC_GET_TREE = 4,
    IPC_GET_MARKS = 5,
    IPC_GET_BAR_CONFIG = 6,
    IPC_GET_VERSION = 7,
    IPC_GET_BINDING_MODES = 8,
    IPC_GET_CONFIG = 9,
    IPC_SEND_TICK = 10,
    IPC_SYNC = 11,
    IPC_GET_BINDING_STATE = 12,

    // sway-specific command types
    IPC_GET_INPUTS = 100,
    IPC_GET_SEATS = 101,

    // Events sent from sway to clients. Events have the highest bits set.
    IPC_EVENT_WORKSPACE = ((1<<31) | 0),
    IPC_EVENT_OUTPUT = ((1<<31) | 1),
    IPC_EVENT_MODE = ((1<<31) | 2),
    IPC_EVENT_WINDOW = ((1<<31) | 3),
    IPC_EVENT_BARCONFIG_UPDATE = ((1<<31) | 4),
    IPC_EVENT_BINDING = ((1<<31) | 5),
    IPC_EVENT_SHUTDOWN = ((1<<31) | 6),
    IPC_EVENT_TICK = ((1<<31) | 7),

    // sway-specific event types
    IPC_EVENT_BAR_STATE_UPDATE = ((1<<31) | 20),
    IPC_EVENT_INPUT = ((1<<31) | 21),
};

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

struct sockaddr_un *ipc_user_sockaddr(void);
int ipc_handle_connection(int fd, uint32_t mask, void *data);
int ipc_client_handle_readable(int client_fd, uint32_t mask, void *data);
int ipc_client_handle_writable(int client_fd, uint32_t mask, void *data);
void ipc_client_disconnect(struct ipc_client *client);
void ipc_client_handle_command(struct ipc_client *client, uint32_t payload_length,
    enum ipc_command_type payload_type);
bool ipc_send_reply(struct ipc_client *client, enum ipc_command_type payload_type,
    const char *payload, uint32_t payload_length);

void ipc_init(struct wl_event_loop *wl_event_loop) {
    ipc_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ipc_socket == -1) {
        printf("Unable to create IPC socket\n");
    }
    if (fcntl(ipc_socket, F_SETFD, FD_CLOEXEC) == -1) {
        printf("Unable to set CLOEXEC on IPC socket\n");
    }
    if (fcntl(ipc_socket, F_SETFL, O_NONBLOCK) == -1) {
        printf("Unable to set NONBLOCK on IPC socket\n");
    }

    ipc_sockaddr = ipc_user_sockaddr();

    /* // We want to use socket name set by user, not existing socket from another sway instance. */
    /* if (getenv("SWAYSOCK") != NULL && access(getenv("SWAYSOCK"), F_OK) == -1) { */
    /*  strncpy(ipc_sockaddr->sun_path, getenv("SWAYSOCK"), sizeof(ipc_sockaddr->sun_path) - 1); */
    /*  ipc_sockaddr->sun_path[sizeof(ipc_sockaddr->sun_path) - 1] = 0; */
    /* } */

    unlink(ipc_sockaddr->sun_path);
    if (bind(ipc_socket, (struct sockaddr *)ipc_sockaddr, sizeof(*ipc_sockaddr)) == -1) {
        printf("Unable to bind IPC socket\n");
    }

    if (listen(ipc_socket, 3) == -1) {
        printf("Unable to listen on IPC socket\n");
    }

    // Set i3 IPC socket path so that i3-msg works out of the box
    /* setenv("I3SOCK", ipc_sockaddr->sun_path, 1); */
    setenv("SWAYSOCK", ipc_sockaddr->sun_path, 1);

    wlr_list_init(&ipc_client_list);

    wl_event_loop_add_fd(wl_event_loop, ipc_socket,
            WL_EVENT_READABLE, ipc_handle_connection, wl_event_loop);
}

struct sockaddr_un *ipc_user_sockaddr(void) {
    struct sockaddr_un *ipc_sockaddr = malloc(sizeof(struct sockaddr_un));
    if (ipc_sockaddr == NULL) {
        printf("Can't allocate ipc_sockaddr\n");
    }

    ipc_sockaddr->sun_family = AF_UNIX;
    int path_size = sizeof(ipc_sockaddr->sun_path);

    // Env var typically set by logind, e.g. "/run/user/<user-id>"
    const char *dir = getenv("XDG_RUNTIME_DIR");
    if (!dir) {
        dir = "/tmp";
    }
    if (path_size <= snprintf(ipc_sockaddr->sun_path, path_size,
            "%s/sway-ipc.%u.%i.sock", dir, getuid(), getpid())) {
        printf("Socket path won't fit into ipc_sockaddr->sun_path\n");
    }

    return ipc_sockaddr;
}

int ipc_handle_connection(int fd, uint32_t mask, void *data) {
    struct wl_event_loop *wl_event_loop = data;
    (void) fd;
    int client_fd = accept(ipc_socket, NULL, NULL);
    if (client_fd == -1) {
        printf("Unable to accept IPC client connection\n");
        return 0;
    }

    if (fcntl(client_fd, F_SETFD, FD_CLOEXEC) == -1) {
        printf("Unable to set CLOEXEC on IPC client socket\n");
        close(client_fd);
        return 0;
    }
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
        printf("Unable to set NONBLOCK on IPC client socket\n");
        close(client_fd);
        return 0;
    }

    struct ipc_client *client = malloc(sizeof(struct ipc_client));
    if (!client) {
        printf("Unable to allocate ipc client\n");
        close(client_fd);
        return 0;
    }
    client->pending_length = 0;
    client->fd = client_fd;
    client->subscribed_events = 0;
    client->event_source = wl_event_loop_add_fd(wl_event_loop,
            client_fd, WL_EVENT_READABLE, ipc_client_handle_readable, client);
    client->writable_event_source = NULL;

    client->write_buffer_size = 128;
    client->write_buffer_len = 0;
    client->write_buffer = malloc(client->write_buffer_size);
    if (!client->write_buffer) {
        printf("Unable to allocate ipc client write buffer\n");
        close(client_fd);
        return 0;
    }

    wlr_list_push(&ipc_client_list, client);
    return 0;
}

int ipc_client_handle_readable(int client_fd, uint32_t mask, void *data) {
    struct ipc_client *client = data;

    if (mask & WL_EVENT_ERROR) {
        printf("IPC Client socket error, removing client\n");
        ipc_client_disconnect(client);
        return 0;
    }

    if (mask & WL_EVENT_HANGUP) {
        printf("Client %d hung up\n", client->fd);
        ipc_client_disconnect(client);
        return 0;
    }

    int read_available;
    if (ioctl(client_fd, FIONREAD, &read_available) == -1) {
        printf("Unable to read IPC socket buffer size\n");
        ipc_client_disconnect(client);
        return 0;
    }

    // Wait for the rest of the command payload in case the header has already been read
    if (client->pending_length > 0) {
        if ((uint32_t)read_available >= client->pending_length) {
            // Reset pending values.
            uint32_t pending_length = client->pending_length;
            enum ipc_command_type pending_type = client->pending_type;
            client->pending_length = 0;
            ipc_client_handle_command(client, pending_length, pending_type);
        }
        return 0;
    }

    if (read_available < (int) IPC_HEADER_SIZE) {
        return 0;
    }

    uint8_t buf[IPC_HEADER_SIZE];
    // Should be fully available, because read_available >= IPC_HEADER_SIZE
    ssize_t received = recv(client_fd, buf, IPC_HEADER_SIZE, 0);
    if (received == -1) {
        printf("Unable to receive header from IPC client\n");
        ipc_client_disconnect(client);
        return 0;
    }

    if (memcmp(buf, ipc_magic, sizeof(ipc_magic)) != 0) {
        printf("IPC header check failed\n");
        ipc_client_disconnect(client);
        return 0;
    }

    memcpy(&client->pending_length, buf + sizeof(ipc_magic), sizeof(uint32_t));
    memcpy(&client->pending_type, buf + sizeof(ipc_magic) + sizeof(uint32_t), sizeof(uint32_t));

    if (read_available - received >= (long)client->pending_length) {
        // Reset pending values.
        uint32_t pending_length = client->pending_length;
        enum ipc_command_type pending_type = client->pending_type;
        client->pending_length = 0;
        ipc_client_handle_command(client, pending_length, pending_type);
    }

    return 0;
}

static void ipc_send_event(const char *json_string, enum ipc_command_type event) {
    struct ipc_client *client;
    for (size_t i = 0; i < ipc_client_list.length; i++) {
        client = ipc_client_list.items[i];
        if ((client->subscribed_events & event_mask(event)) == 0) {
            continue;
        }
        if (!ipc_send_reply(client, event, json_string,
                (uint32_t)strlen(json_string))) {
            printf("Unable to send reply to IPC client\n");
            /* ipc_send_reply destroys client on error, which also
             * removes it from the list, so we need to process
             * current index again */
            i--;
        }
    }
}

void ipc_event_workspace() {
  ipc_send_event("", IPC_EVENT_WORKSPACE);
}

int ipc_client_handle_writable(int client_fd, uint32_t mask, void *data) {
    struct ipc_client *client = data;

    if (mask & WL_EVENT_ERROR) {
        printf("IPC Client socket error, removing client\n");
        ipc_client_disconnect(client);
        return 0;
    }

    if (mask & WL_EVENT_HANGUP) {
        printf("Client %d hung up\n", client->fd);
        ipc_client_disconnect(client);
        return 0;
    }

    if (client->write_buffer_len <= 0) {
        return 0;
    }

    ssize_t written = write(client->fd, client->write_buffer, client->write_buffer_len);

    if (written == -1 && errno == EAGAIN) {
        return 0;
    } else if (written == -1) {
        printf("Unable to send data from queue to IPC client\n");
        ipc_client_disconnect(client);
        return 0;
    }

    memmove(client->write_buffer, client->write_buffer + written, client->write_buffer_len - written);
    client->write_buffer_len -= written;

    if (client->write_buffer_len == 0 && client->writable_event_source) {
        wl_event_source_remove(client->writable_event_source);
        client->writable_event_source = NULL;
    }

    return 0;
}

void ipc_client_disconnect(struct ipc_client *client) {
    if (!(client != NULL)) {
        return;
    }

    shutdown(client->fd, SHUT_RDWR);

    printf("IPC Client %d disconnected\n", client->fd);
    wl_event_source_remove(client->event_source);
    if (client->writable_event_source) {
        wl_event_source_remove(client->writable_event_source);
    }
    size_t i = 0;
    while (i < ipc_client_list.length && ipc_client_list.items[i] != client) {
        i++;
    }
    wlr_list_del(&ipc_client_list, i);
    free(client->write_buffer);
    close(client->fd);
    free(client);
}

/* static void ipc_get_tags_callback(struct tagset *tagset, json_object *tagJson) { */
/*     json_object *data = ipc_json_describe_tagset(tagset); */
/*     // override the default focused indicator because */
/*     // it's set differently for the get_workspaces reply */
/*     unsigned int focusedTag = selMon->tagset.focusedTag; */
/*     bool focused = tagset->focusedTag == focusedTag; */
/*     json_object_object_del(data, "focused"); */
/*     json_object_object_add(data, "focused", */
/*             json_object_new_boolean(focused)); */
/*     json_object_array_add((json_object *)tagJson, tagJson); */
/* } */

void ipc_client_handle_command(struct ipc_client *client, uint32_t payload_length,
        enum ipc_command_type payload_type) {
    if (client == NULL) {
        return;
    }

    char *buf = malloc(payload_length + 1);
    if (!buf) {
        printf("Unable to allocate IPC payload\n");
        ipc_client_disconnect(client);
        return;
    }
    if (payload_length > 0) {
        // Payload should be fully available
        ssize_t received = recv(client->fd, buf, payload_length, 0);
        if (received == -1)
        {
            printf("Unable to receive payload from IPC client\n");
            ipc_client_disconnect(client);
            free(buf);
            return;
        }
    }
    buf[payload_length] = '\0';

    switch (payload_type) {
        case IPC_COMMAND:
            execute_command(buf);
            ipc_send_reply(client, payload_type, "", strlen(""));
            goto exit_cleanup;
            break;
        case IPC_GET_WORKSPACES:
            {
                json_object *array = json_object_new_array();

                struct monitor *m;
                wl_list_for_each(m, &mons, link) {
                    struct workspaceset *ws_set = m->ws_set;
                    for (int i = 0; i < ws_set->workspaces.length; i++) {
                        json_object *tag = ipc_json_describe_workspace(
                                m,
                                get_workspace(ws_set, i),
                                ws_set->focused_workspace[0] == i);
                        json_object_array_add(array, tag);
                    }
                }

                const char *json_string = json_object_get_string(array);
                ipc_send_reply(client, payload_type, json_string,
                        strlen(json_string));
                json_object_put(array); // free
                goto exit_cleanup;
            }

        case IPC_SUBSCRIBE:
            {
                // TODO: Check if they're permitted to use these events
                struct json_object *request = json_tokener_parse(buf);

                bool is_tick = false;
                // parse requested event types
                for (size_t i = 0; i < json_object_array_length(request); i++) {
                    const char *event_type = json_object_get_string(json_object_array_get_idx(request, i));
                    if (strcmp(event_type, "workspace") == 0) {
                        client->subscribed_events |= event_mask(IPC_EVENT_WORKSPACE);
                    }
                }

                json_object_put(request);
                const char msg[] = "{\"success\": true}";
                ipc_send_reply(client, payload_type, msg, strlen(msg));
                if (is_tick) {
                    const char tickmsg[] = "{\"first\": true, \"payload\": \"\"}";
                    ipc_send_reply(client, IPC_EVENT_TICK, tickmsg,
                            strlen(tickmsg));
                }
                goto exit_cleanup;
            }

        case IPC_GET_TREE:
            {
                // TODO which clinet?
                ipc_send_reply(client, payload_type, "", strlen(""));
                goto exit_cleanup;
            }

        default:
            printf("Unknown IPC command type %x\n", payload_type);
            goto exit_cleanup;
    }

exit_cleanup:
    free(buf);
    return;
}

bool ipc_send_reply(struct ipc_client *client, enum ipc_command_type payload_type,
        const char *payload, uint32_t payload_length) {
    assert(payload);

    char data[IPC_HEADER_SIZE];

    memcpy(data, ipc_magic, sizeof(ipc_magic));
    memcpy(data + sizeof(ipc_magic), &payload_length, sizeof(payload_length));
    memcpy(data + sizeof(ipc_magic) + sizeof(payload_length), &payload_type, sizeof(payload_type));

    while (client->write_buffer_len + IPC_HEADER_SIZE + payload_length >=
                 client->write_buffer_size) {
        client->write_buffer_size *= 2;
    }

    if (client->write_buffer_size > 4e6) { // 4 MB
        printf("Client write buffer too big (%zu), disconnecting client\n",
                client->write_buffer_size);
        ipc_client_disconnect(client);
        return false;
    }

    char *new_buffer = realloc(client->write_buffer, client->write_buffer_size);
    if (!new_buffer) {
        printf("Unable to reallocate ipc client write buffer\n");
        ipc_client_disconnect(client);
        return false;
    }
    client->write_buffer = new_buffer;

    memcpy(client->write_buffer + client->write_buffer_len, data, IPC_HEADER_SIZE);
    client->write_buffer_len += IPC_HEADER_SIZE;
    memcpy(client->write_buffer + client->write_buffer_len, payload, payload_length);
    client->write_buffer_len += payload_length;

    if (!client->writable_event_source) {
        client->writable_event_source = wl_event_loop_add_fd(
                server.wl_event_loop, client->fd, WL_EVENT_WRITABLE,
                ipc_client_handle_writable, client);
    }

    return true;
}
