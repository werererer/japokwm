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
#include <lua.h>

#include "tagset.h"

/* macros */
#define BARF(fmt, ...)      do { fprintf(stderr, fmt "\n", ##__VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define EBARF(fmt, ...)     BARF(fmt ": %s", ##__VA_ARGS__, strerror(errno))
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define END(A)                  ((A) + LENGTH(A))
/* number of chars a string should contain */
#define ARR_STRING_LENGTH(X) strlen(X) + 2*(strlen("[]") + NUM_DIGITS)

typedef struct Monitor Monitor;

struct keyboard {
    struct wl_list link;
    struct wlr_input_device *device;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

typedef struct layout Key;
typedef struct layout Button;

struct containers_info {
    // count
    int n;
    int id;
};

typedef uint32_t xkb_keysym_t;

/* rules */
struct rule {
    char *id;
    char *title;
    int tags;
    int floating;
    int monitor;
};

struct monRule {
    char *name;
    float mfact;
    int nmaster;
    float scale;
    struct layout *lt;
    enum wl_output_transform rr;
};
extern struct lua_State *L;

// breaking codestyle to abide by the wlroots style
void wlr_list_clear(struct wlr_list *list);
void join_path(char *base, const char *file);
char last_char(const char *str);
#endif
