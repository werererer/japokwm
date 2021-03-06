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

// those constants are arbitrary and are bound to change
#define MAXLEN 15
#define NUM_CHARS 64
#define NUM_DIGITS 9
#define INVALID_POSITION -1
#define INVALID_WORKSPACE_ID -1

#define MIN_CONTAINER_WIDTH 30
#define MIN_CONTAINER_HEIGHT 30

/* rules */
struct rule {
    char *id;
    char *title;
    int lua_func_ref;
};

extern struct lua_State *L;

bool dir_exists(const char *path);
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
void lua_get_default_layout_data(lua_State *L);

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
void lua_get_default_master_layout_data(lua_State *L);
/*
 * create a lua table that looks like this:
 * {
 *   {2, 3, 4, 5, 6, 7, 8, 9}
 * }
 */
void lua_get_default_resize_data(lua_State *L);

void wlr_list_clear(struct wlr_list *list, void (*destroy_func)(void *));
bool wlr_list_empty(struct wlr_list *list);

typedef int (*cmp_func_t)(const void*, const void*);
/* return 0 on success and 1 on failure */
int wlr_list_remove(struct wlr_list *list,
        int (*compare)(const void *, const void *), const void *cmp_to);
/* return 0 on success and 1 on failure */
int remove_in_composed_list(struct wlr_list *lists,
        int (*compare)(const void *, const void *), const void *cmp_to);
int find_in_composed_list(struct wlr_list *lists,
        int (*compare)(const void *, const void *), const void *cmp_to);
struct wlr_list *find_list_in_composed_list(struct wlr_list *lists,
        int (*compare)(const void *, const void *), const void *cmp_to);

struct wlr_list *list_insert_into_relative_position(struct wlr_list *lists,
        int index, void *item);

// compare pointers and return 0 if they are equal and 1 otherwise
int cmp_ptr(const void *ptr1, const void *ptr2);

void lua_tocolor(float dest_color[static 4]);
// like lua_ref but override the old value if *ref > 0
void lua_ref_safe(lua_State *L, int t, int *ref);
void copy_options(struct options *dest_option, struct options *src_option);
void print_trace();

/* a composed list is just a list consisting of lists so that if an index i is
 * given it returns the same value as if all the lists where concatenated */
void *get_in_composed_list(struct wlr_list *lists, int i);
struct wlr_list *get_list_at_i_in_composed_list(struct wlr_list *lists, int i);
void delete_from_composed_list(struct wlr_list *lists, int i);

int length_of_list(struct wlr_list *list);
int length_of_composed_list(struct wlr_list *lists);
int relative_index_to_absolute_index(int i, int j, int length);

void *get_relative_item_in_list(struct wlr_list *list, int i, int j);
void *get_relative_item_in_composed_list(struct wlr_list *lists, int i, int j);

int exec(const char *cmd);
bool is_approx_equal(double a, double b, double error_range);
#endif
