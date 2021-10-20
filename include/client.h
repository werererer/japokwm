#ifndef CLIENT
#define CLIENT
#include <X11/Xlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/xwayland.h>

#include "bitset/bitset.h"
#include "seat.h"

enum shell { XDG_SHELL, X11_MANAGED, X11_UNMANAGED, LAYER_SHELL }; /* client types */
union surface_t {
    struct wlr_xdg_surface *xdg;
    struct wlr_layer_surface_v1 *layer;
    struct wlr_xwayland_surface *xwayland;
};

struct client {
    /* containers containing this client */
    struct container *con;
    union surface_t surface;

    struct monitor *m;
    struct wl_listener activate;
    struct wl_listener commit;
    struct wl_listener set_title;
    struct wl_listener set_app_id;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener new_popup;

    enum shell type;
    const char *title;
    const char *app_id;
    BitSet *sticky_workspaces;

    // used to determine what to damage
    bool resized;
    bool moved_workspace;
    bool is_independent;
};

struct client *create_client(enum shell shell_type, union surface_t surface);
void destroy_client(struct client *c);

void container_move_sticky_containers_current_ws(struct container *con);
void container_move_sticky_containers(struct container *con, int ws_id);

void focus_client(struct seat *seat, struct client *old, struct client *c);
void focus_surface(struct seat *seat, struct wlr_surface *surface);
void client_setsticky(struct client *c, BitSet *workspaces);
void reset_floating_client_borders(int border_px);
void kill_client(struct client *c);

float calc_ratio(float width, float height);

void client_handle_new_popup(struct wl_listener *listener, void *data);
void client_handle_set_title(struct wl_listener *listener, void *data);
void client_handle_set_app_id(struct wl_listener *listener, void *data);

struct wlr_surface *get_base_wlrsurface(struct client *c);
struct wlr_surface *get_wlrsurface(struct client *c);
#endif
