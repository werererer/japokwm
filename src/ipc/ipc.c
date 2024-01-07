#include <assert.h>
#include <linux/input-event-codes.h>
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
#include <wlr/util/log.h>

#include "ipc/ipc.h"
#include "ipc/ipc-server.h"

static struct sockaddr_un *ipc_sockaddr = NULL;
static GPtrArray *ipc_client_list;

static int read_client_header(int client_fd, struct ipc_client *client);
static void ipc_client_disconnect(struct ipc_client *client);
static int check_socket_errors(uint32_t mask, struct ipc_client *client);
static int get_available_read_data(int client_fd, struct ipc_client *client);

struct sockaddr_un *ipc_user_sockaddr(void);

int ipc_handle_connection(int fd, uint32_t mask, void *data);

void ipc_init(struct wl_event_loop *wl_event_loop) {
    int ipc_socket = socket(AF_UNIX, SOCK_STREAM, 0);
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

    unlink(ipc_sockaddr->sun_path);
    if (bind(ipc_socket, (struct sockaddr *)ipc_sockaddr, sizeof(*ipc_sockaddr)) == -1) {
        printf("Unable to bind IPC socket\n");
    }

    if (listen(ipc_socket, 3) == -1) {
        printf("Unable to listen on IPC socket\n");
    }

    // Set SWAY IPC socket path so that waybar automatically shows
    // tags(tags)
    setenv("SWAYSOCK", ipc_sockaddr->sun_path, 1);
    setenv("JAPOKWMSOCK", ipc_sockaddr->sun_path, 1);

    ipc_client_list = g_ptr_array_new();

    wl_event_loop_add_fd(wl_event_loop, ipc_socket,
            WL_EVENT_READABLE, ipc_handle_connection, wl_event_loop);
}

int ipc_handle_connection(int fd, uint32_t mask, void *data) {
    struct wl_event_loop *wl_event_loop = data;
    (void) fd;
    int ipc_socket = server.ipc_socket;
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

    g_ptr_array_add(ipc_client_list, client);
    return 0;
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
            "%s/japokwm-ipc.%u.%i.sock", dir, getuid(), getpid())) {
        printf("Socket path won't fit into ipc_sockaddr->sun_path\n");
    }

    return ipc_sockaddr;
}

static void ipc_client_disconnect(struct ipc_client *client) {
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
    while (i < ipc_client_list->len && g_ptr_array_index(ipc_client_list, i) != client) {
        i++;
    }
    g_ptr_array_remove_index(ipc_client_list, i);
    free(client->write_buffer);
    close(client->fd);
    free(client);
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

static int check_socket_errors(uint32_t mask, struct ipc_client *client) {
    if (mask & (WL_EVENT_ERROR | WL_EVENT_HANGUP)) {
        printf("Client %d disconnected%s\n", client->fd, 
               (mask & WL_EVENT_ERROR) ? " with error" : "");
        ipc_client_disconnect(client);
        return 1; // Return 1 to indicate an error occurred
    }
    return 0; // Return 0 to indicate no error
}

int ipc_client_handle_readable(int client_fd, uint32_t mask, void *data) {
    struct ipc_client *client = data;

    if (check_socket_errors(mask, client)) {
        return 0;
    }

    if (get_available_read_data(client_fd, client)) {
        return 0;
    }

    return (client->pending_length > 0) ? 
            handle_client_payload(client) : 
            read_client_header(client_fd, client);
}

void ipc_send_event(const char *json_string, enum ipc_command_type event) {
    struct ipc_client *client;
    for (size_t i = 0; i < ipc_client_list->len; i++) {
        client = g_ptr_array_index(ipc_client_list, i);
        if ((client->subscribed_events & CREATE_EVENT_BITMASK(event)) == 0) {
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

static int get_available_read_data(int client_fd, struct ipc_client *client) {
    int read_available;
    if (ioctl(client_fd, FIONREAD, &read_available) == -1) {
        printf("Unable to read IPC socket buffer size\n");
        ipc_client_disconnect(client);
        return 1; // Return 1 to indicate an error occurred
    }
    return 0; // Return 0 to indicate success
}

static int read_client_header(int client_fd, struct ipc_client *client) {
    uint8_t buf[IPC_HEADER_SIZE];
    ssize_t received = recv(client_fd, buf, IPC_HEADER_SIZE, 0);
    if (received == -1) {
        printf("Unable to receive header from IPC client\n");
        ipc_client_disconnect(client);
        return 0;
    }

    // Validate the IPC header
    if (memcmp(buf, ipc_magic, sizeof(ipc_magic)) != 0) {
        printf("IPC header check failed\n");
        ipc_client_disconnect(client);
        return 0;
    }

    // Extract and store header information
    memcpy(&client->pending_length, buf + sizeof(ipc_magic), sizeof(uint32_t));
    memcpy(&client->pending_type, buf + sizeof(ipc_magic) + sizeof(uint32_t), sizeof(uint32_t));

    return handle_client_payload(client);
}
