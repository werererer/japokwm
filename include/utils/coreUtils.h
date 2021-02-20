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
#include <lauxlib.h>
#include "options.h"

/* macros */
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define END(A)                  ((A) + LENGTH(A))
/* number of chars a string should contain */
#define ARR_STRING_LENGTH(X) strlen(X) + 2*(strlen("[]") + NUM_DIGITS)
#define MAXLEN 15
#define NUM_CHARS 64
#define NUM_DIGITS 9

/* rules */
struct rule {
    char *id;
    char *title;
    int lua_func_ref;
};

extern struct lua_State *L;

bool file_exists(const char *path);
char last_char(const char *str);
// returns exactly the same values as strcmp
int path_compare(const char *path1, const char *path2);
void join_path(char *base, const char *file);

/*
 * create a lua table that looks like this:
 * {
 *   {
 *     {0, 0, 1, 1}
 *   }
 * }
 */
void lua_get_default_layout_data();

/*
 * create a lua table that looks like this:
 * {
 *   {
 *     {0, 0, 1, 1}
 *   },
 *   {
 *     {0, 0, 1, 0.5},
 *     {0, 0.5, 1, 0.5},
 *   },
 *   {
 *     {0, 0, 1, 0.333},
 *     {0, 0.333, 1, 0.333},
 *     {0, 0.5, 1, 0.333},
 *   },
 * }
 */
void lua_get_default_master_layout_data();
/*
 * create a lua table that looks like this:
 * {
 *   {2, 3, 4, 5, 6, 7, 8, 9}
 * }
 */
void lua_get_default_resize_data();

void wlr_list_clear(struct wlr_list *list);
void lua_tocolor(float dest_color[static 4]);
void lua_ref_safe(lua_State *L, int t, int *ref);
void copy_options(struct options *dest_option, struct options *src_option);
void print_trace();
#endif
