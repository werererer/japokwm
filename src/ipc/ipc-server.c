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
#include <wlr/util/log.h>

#include "ipc/ipc-json.h"
#include "ipc/ipc-server.h"
#include "server.h"
#include "tag.h"
#include "client.h"
#include "command.h"
#include "monitor.h"

static int ipc_socket = -1;
static struct sockaddr_un *ipc_sockaddr = NULL;
static GPtrArray *ipc_client_list;

// Macro to generate a bitmask for an event. 
// It shifts 1 to the left by the lower 7 bits of the event value.

struct sockaddr_un *ipc_user_sockaddr(void);
int ipc_handle_connection(int fd, uint32_t mask, void *data);
int ipc_client_handle_writable(int client_fd, uint32_t mask, void *data);
void ipc_client_disconnect(struct ipc_client *client);
void ipc_client_handle_command(struct ipc_client *client, uint32_t payload_length, enum ipc_command_type payload_type);

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

    g_ptr_array_add(ipc_client_list, client);
    return 0;
}

// Forward declaration of helper functions
static int read_client_header(int client_fd, struct ipc_client *client);
static int get_available_read_data(int client_fd, struct ipc_client *client);


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

int handle_client_payload(struct ipc_client *client) {
    if (client->pending_length <= 0) {
        return 0;
    }

    // Process the pending command
    uint32_t pending_length = client->pending_length;
    enum ipc_command_type pending_type = client->pending_type;
    client->pending_length = 0;
    ipc_client_handle_command(client, pending_length, pending_type);

    return 0;
}

void ipc_event_tag() {
    ipc_send_event("", IPC_EVENT_TAG);

    // TODO: this doesn't belong here
    // HACK: just for the time being
    // update container visibility
    for (int i = server.container_stack->len-1; i >= 0; i--) {
        struct container *con = g_ptr_array_index(server.container_stack, i);
        struct monitor *m = server_get_selected_monitor();
        bool viewable = container_viewable_on_monitor(m, con);
        struct wlr_scene_node *node = container_get_scene_node(con);
        wlr_scene_node_set_enabled(node, viewable);
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
    while (i < ipc_client_list->len && g_ptr_array_index(ipc_client_list, i) != client) {
        i++;
    }
    g_ptr_array_remove_index(ipc_client_list, i);
    free(client->write_buffer);
    close(client->fd);
    free(client);
}

void ipc_event_window() {
    ipc_send_event("", IPC_EVENT_WINDOW);
}

void handle_ipc_command(struct ipc_client *client,
        char *buf, uint32_t payload_length,
        enum ipc_command_type payload_type) {
    // Logic for IPC_COMMAND
    char *line = strtok(buf, "\n");
    while (line) {
        size_t line_length = strlen(line);
        if (line + line_length >= buf + payload_length) {
            break;
        }
        line[line_length] = ';';
        line = strtok(NULL, "\n");
    }

    struct cmd_results *results = execute_command(buf, NULL, NULL);
    /* transaction_commit_dirty(); */
    char *json = cmd_results_to_json(results);
    int length = strlen(json);
    ipc_send_reply(client, payload_type, json, (uint32_t)length);
    free(json);
    free(results);
    return;
}

void handle_ipc_get_tags(struct ipc_client *client, char *buf,
        enum ipc_command_type payload_type) {
    json_object *array;

    array = ipc_json_describe_tagsets();

    const char *json_string = json_object_get_string(array);
    ipc_send_reply(client, payload_type, json_string,
            strlen(json_string));
    json_object_put(array); // free
}

void handle_ipc_subscribe(struct ipc_client *client, char *buf, enum ipc_command_type payload_type) {
    // TODO: Check if they're permitted to use these events
    // NOTE: this will probably be fixed by sway, if so copy its
    // implementation and call it a day
    struct json_object *request = json_tokener_parse(buf);

    bool is_tick = false;
    // parse requested event types
    for (size_t i = 0; i < json_object_array_length(request); i++) {
        const char *event_type = json_object_get_string(json_object_array_get_idx(request, i));
        if (strcmp(event_type, "workspace") == 0) {
            client->subscribed_events |= CREATE_EVENT_BITMASK(IPC_EVENT_TAG);
        } else if (strcmp(event_type, "barconfig_update") == 0) {
            client->subscribed_events |= CREATE_EVENT_BITMASK(IPC_EVENT_BARCONFIG_UPDATE);
        } else if (strcmp(event_type, "mode") == 0) {
            client->subscribed_events |= CREATE_EVENT_BITMASK(IPC_EVENT_MODE);
        } else if (strcmp(event_type, "shutdown") == 0) {
            client->subscribed_events |= CREATE_EVENT_BITMASK(IPC_EVENT_SHUTDOWN);
        } else if (strcmp(event_type, "window") == 0) {
            client->subscribed_events |= CREATE_EVENT_BITMASK(IPC_EVENT_WINDOW);
        } else if (strcmp(event_type, "binding") == 0) {
            client->subscribed_events |= CREATE_EVENT_BITMASK(IPC_EVENT_BINDING);
        } else if (strcmp(event_type, "tick") == 0) {
            client->subscribed_events |= CREATE_EVENT_BITMASK(IPC_EVENT_TICK);
            is_tick = true;
        } else {
            const char msg[] = "{\"success\": false}";
            ipc_send_reply(client, payload_type, msg, strlen(msg));
            json_object_put(request);
            printf("Unsupported event type in subscribe request\n");
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
}

void handle_ipc_get_tree(struct ipc_client *client, char *buf,
        enum ipc_command_type payload_type) {
    struct monitor *m = server_get_selected_monitor();
    json_object *tree = ipc_json_describe_selected_container(m);
    const char *json_string = json_object_to_json_string(tree);

    ipc_send_reply(client, payload_type, json_string, strlen(json_string));
    json_object_put(tree);
}

void handle_ipc_get_bar_config(struct ipc_client *client, char *buf, enum ipc_command_type payload_type) {
    if (!buf[0]) {
        // Send list of configured bar IDs
        json_object *bars = json_object_new_array();
        // for (int i = 0; i < config->bars->length; ++i) {
        //     struct bar_config *bar = config->bars->items[i];
        //     json_object_array_add(bars, json_object_new_string(bar->id));
        // }
        const char *json_string = json_object_to_json_string(bars);
        ipc_send_reply(client, payload_type, json_string,
                (uint32_t)strlen(json_string));
        json_object_put(bars); // free
    } else {
        // Send particular bar's details
        json_object *json = ipc_json_describe_bar_config();
        const char *json_string = json_object_to_json_string(json);
        ipc_send_reply(client, payload_type, json_string,
                (uint32_t)strlen(json_string));
        json_object_put(json); // free
    }
}

// Function to receive payload
static char* receive_payload(struct ipc_client *client, uint32_t payload_length) {
    if (client == NULL || payload_length == 0) {
        return NULL;
    }

    char *buf = malloc(payload_length + 1);
    if (!buf) {
        printf("Unable to allocate IPC payload\n");
        return NULL;
    }

    ssize_t received = recv(client->fd, buf, payload_length, 0);
    if (received == -1) {
        printf("Unable to receive payload from IPC client\n");
        free(buf);
        return NULL;
    }

    buf[payload_length] = '\0';
    return buf;
}

// Dispatcher function
static void dispatch_ipc_command(struct ipc_client *client, char *buf,
                                 uint32_t payload_length,
                                 enum ipc_command_type payload_type) {
    if (buf == NULL) {
        return;
    }

    switch (payload_type) {
        case IPC_COMMAND:
            handle_ipc_command(client, buf, payload_length, payload_type);
            break;
        case IPC_GET_TAGS:
            handle_ipc_get_tags(client, buf, payload_type);
            break;
        case IPC_SUBSCRIBE:
            handle_ipc_subscribe(client, buf, payload_type);
            break;
        case IPC_GET_TREE:
            handle_ipc_get_tree(client, buf, payload_type);
            break;
        case IPC_GET_BAR_CONFIG:
            handle_ipc_get_bar_config(client, buf, payload_type);
            break;
        default:
            printf("Unknown IPC command type %x\n", payload_type);
            break;
    }
}

static char* receive_and_verify_payload(struct ipc_client *client, uint32_t payload_length) {
    char *buf = receive_payload(client, payload_length);
    if (buf == NULL) {
        printf("Payload reception failed\n");
        ipc_client_disconnect(client);
    }
    return buf;
}

void ipc_client_handle_command(struct ipc_client *client, uint32_t payload_length, enum ipc_command_type payload_type) {
    char *buf = receive_and_verify_payload(client, payload_length);
    if (buf == NULL) {
        return; // The error is already handled in receive_and_verify_payload
    }

    dispatch_ipc_command(client, buf, payload_length, payload_type);
    free(buf);
}
