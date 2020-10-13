#ifndef COREUTILS
#define COREUTILS

#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wayland-server-protocol.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include <julia.h>

/* macros */
#define BARF(fmt, ...)      do { fprintf(stderr, fmt "\n", ##__VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define EBARF(fmt, ...)     BARF(fmt ": %s", ##__VA_ARGS__, strerror(errno))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define END(A)                  ((A) + LENGTH(A))
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#ifdef XWAYLAND
#define WLR_SURFACE(C)          ((C)->type != XDGShell ? (C)->surface.xwayland->surface : (C)->surface.xdg->surface)
#else
#define WLR_SURFACE(C)          ((C)->surface.xdg->surface)
#endif

typedef struct Monitor Monitor;
typedef struct wlr_fbox Container;

typedef union {
    int i;
    unsigned int ui;
    float f;
    const void *v;
} Arg;


typedef struct {
    char *symbol;
    jl_function_t *arrange;
} Layout;

struct Monitor {
    struct wl_list link;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener destroy;
    struct wlr_box m;      /* monitor area, layout-relative */
    struct wlr_box w;      /* window area, layout-relative */
    Layout lt;
    unsigned int seltags;
    unsigned int tagset[2];
    double mfact;
    int nmaster;
};

typedef struct {
    struct wl_list link;
    struct wlr_input_device *device;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
} Keyboard;

/* datastructures for parsing julia */
typedef struct {
    const char *symbol;
    jl_function_t *func;
} Key;

/* datastructures for parsing julia */
typedef struct {
    const char *symbol;
    jl_function_t *func;
} Button;

typedef uint32_t xkb_keysym_t;

/* rules */
typedef struct {
    const char *id;
    const char *title;
    int tags;
    int isfloating;
    int monitor;
} Rule;

typedef struct {
    const char *name;
    float mfact;
    int nmaster;
    float scale;
    Layout *lt;
    enum wl_output_transform rr;
} MonitorRule;
extern struct wlr_seat *seat;
#endif
