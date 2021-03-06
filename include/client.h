#ifndef CLIENT
#define CLIENT
#include <X11/Xlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/xwayland.h>

enum shell { XDG_SHELL, X11_MANAGED, X11_UNMANAGED, LAYER_SHELL }; /* client types */
union surface_t {
    struct wlr_xdg_surface *xdg;
    struct wlr_layer_surface_v1 *layer;
    struct wlr_xwayland_surface *xwayland;
};

struct client {
    float ratio;
    /* containers containing this client */
    struct container *con;
    union surface_t surface;

    struct wl_listener set_title;
    struct wl_listener set_app_id;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener new_popup;
    int bw;

    enum shell type;
    const char *title;
    const char *app_id;
    bool sticky;
    // workspace id
    int ws_id;

    // used to determine what to damage
    bool resized;
    bool moved_workspace;
};

struct client *create_client(enum shell shell_type, union surface_t surface);
void destroy_client(struct client *c);

void focus_client(struct client *old, struct client *c);
void client_setsticky(struct client *c, bool sticky);
void reset_tiled_client_borders(int border_bx);
void reset_floating_client_borders(int border_px);
void kill_client(struct client *c);

bool wants_floating(struct client *c);
bool is_popup_menu(struct client *c);

float calc_ratio(float width, float height);

void destroy_notify(struct wl_listener *listener, void *data);
void maprequest(struct wl_listener *listener, void *data);
void unmap_notify(struct wl_listener *listener, void *data);
void client_handle_set_title(struct wl_listener *listener, void *data);
void client_handle_set_app_id(struct wl_listener *listener, void *data);

struct wlr_surface *get_base_wlrsurface(struct client *c);
struct wlr_surface *get_wlrsurface(struct client *c);
#endif
