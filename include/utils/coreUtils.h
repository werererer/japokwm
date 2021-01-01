#ifndef COREUTILS
#define COREUTILS

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_list.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <lua.h>

/* macros */
#define BARF(fmt, ...)      do { fprintf(stderr, fmt "\n", ##__VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define EBARF(fmt, ...)     BARF(fmt ": %s", ##__VA_ARGS__, strerror(errno))
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define END(A)                  ((A) + LENGTH(A))
/* number of chars a string should contain */
#define ARR_STRING_LENGTH(X) strlen(X) + 2*(strlen("[]") + NUM_DIGITS)
#define MAXLEN 15
#define NUM_CHARS 64
#define NUM_DIGITS 9


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

typedef uint32_t xkb_keysym_t;

/* rules */
struct rule {
    char *id;
    char *title;
    int tags;
    int floating;
    int monitor;
};

struct mon_rule {
    char *name;
    float mfact;
    int nmaster;
    float scale;
    struct layout *lt;
    enum wl_output_transform rr;
};
extern struct lua_State *L;

bool file_exists(const char *path);
char last_char(const char *str);
// returns exactly the same values as strcmp
int path_compare(const char *path1, const char *path2);
void join_path(char *base, const char *file);

void wlr_list_clear(struct wlr_list *list);
#endif
