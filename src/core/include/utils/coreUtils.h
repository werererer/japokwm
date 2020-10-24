#ifndef COREUTILS
#define COREUTILS

#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wayland-server-protocol.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/render/wlr_texture.h>
#include <julia.h>

/* macros */
#define BARF(fmt, ...)      do { fprintf(stderr, fmt "\n", ##__VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define EBARF(fmt, ...)     BARF(fmt ": %s", ##__VA_ARGS__, strerror(errno))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define END(A)                  ((A) + LENGTH(A))
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define NUM_DIGITS 9
#define ARR_STRING_LENGTH(X) strlen(X) + 2*(strlen("[]") + NUM_DIGITS)

/* enums */
enum { CurNormal, CurMove, CurResize }; /* cursor */

typedef struct Monitor Monitor;
typedef struct wlr_fbox Container;

struct layout {
    char *symbol;
    jl_function_t *arrange;
};

struct monitor {
    struct wl_list link;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener destroy;
    struct wlr_box m;      /* monitor area, layout-relative */
    struct wlr_box w;      /* window area, layout-relative */
    struct layout lt;
    unsigned int seltags;
    unsigned int tagset[2];
    double mfact;
    int nmaster;
};

struct keyboard {
    struct wl_list link;
    struct wlr_input_device *device;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

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
struct rule {
    const char *id;
    const char *title;
    int tags;
    int isfloating;
    int monitor;
};

struct monRule {
    const char *name;
    float mfact;
    int nmaster;
    float scale;
    struct layout *lt;
    enum wl_output_transform rr;
};
extern struct wlr_seat *seat;

// breaking codestyle to abide by the wlroots style
void wlr_list_clear(struct wlr_list *list);
void joinPath(char *base, char *file);
char lastChar(char *str);
#endif
