#include "utils/coreUtils.h"
#include <string.h>
#include <unistd.h>
#include <wlr/util/log.h>
#include <wordexp.h>
#include <stdlib.h>
#include <execinfo.h>
#include <math.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

struct lua_State *L;

bool dir_exists(const char *path)
{
    struct stat sb;
    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        return true;
    } else {
        return false;
    }
}

bool file_exists(const char *path)
{
    return access(path, R_OK) != -1;
}

void wlr_list_clear(GPtrArray *array, void (*destroy_func)(void *))
{
    for (int i = 0; i < array->len; i++) {
        void *item = g_ptr_array_index(array, 0);
        if (destroy_func) {
            destroy_func(item);
        }
        g_ptr_array_remove(array, 0);
    }
}

bool list_remove(GPtrArray *array, int (*compare)(const void *, const void *), const void *cmp_to)
{
    for (int i = 0; i < array->len; i++) {
        void *item = g_ptr_array_index(array, i);
        if (compare(item, cmp_to) == 0) {
            g_ptr_array_remove_index(array, i);
            return true;
        }
    }
    return false;
}

bool remove_in_composed_list(GPtrArray *array, int (*compare)(const void *, const void *), void *cmp_to)
{
    for (int i = 0; i < array->len; i++) {
        GPtrArray *list = g_ptr_array_index(array, i);
        if (list_remove(list, compare, cmp_to) == 0) {
            return true;
        }
    }
    return false;
}

int cmp_ptr(const void *ptr1, const void *ptr2)
{
    return ptr1 == ptr2 ? 0 : 1;
}

int find_in_composed_list(GPtrArray *lists,
        int (*compare)(const void *, const void *), const void *cmp_to)
{
    int position = 0;
    for (int i = 0; i < lists->len; i++) {
        GPtrArray *list = g_ptr_array_index(lists, i);
        for (int j = 0; j < list->len; j++) {
            void *item = g_ptr_array_index(list, j);
            if (compare(item, cmp_to) == 0) {
                return position;
            }
            position++;
        }
    }
    return -1;
}

GPtrArray *find_list_in_composed_list(GPtrArray *arrays,
        int (*compare)(const void *, const void *), const void *cmp_to)
{
    for (int i = 0; i < arrays->len; i++) {
        GPtrArray *array = g_ptr_array_index(arrays, i);
        for (int j = 0; j < array->len; j++) {
            void *item = g_ptr_array_index(array, j);
            if (compare(item, cmp_to) == 0) {
                return array;
            }
        }
    }
    return NULL;
}

char last_char(const char *str)
{
    if (strlen(str) == 0)
        return '\0';
    return str[strlen(str)-1];
}

int path_compare(const char *path1, const char *path2)
{
    char *p1 = strdup(path1);
    char *p2 = strdup(path2);

    // replace all trailing '/'
    while (p1[strlen(p1)-1] == '/')
        p1[strlen(p1)-1] = '\0';
    while (p2[strlen(p2)-1] == '/')
        p2[strlen(p2)-1] = '\0';

    int ret = strcmp(p1, p2);

    free(p2);
    free(p1);
    return ret;
}

void join_path(char **base, const char *file)
{
    *base = realloc(*base, strlen(*base) + 1 + strlen(file) + 1);

    if (last_char(*base) != '/' && file[0] != '/') {
        strcat(*base, "/");
    } else if (last_char(*base) == '/' && file[0] == ' ') {
        *base[strlen(*base)-1] = '\0';
    }
    strcat(*base, file);
}

void lua_ref_safe(lua_State *L, int t, int *ref)
{
    if (*ref > 0) {
        luaL_unref(L, t, *ref);
    }

    *ref = luaL_ref(L, t);
}

static void lua_create_container(lua_State *L, struct wlr_fbox con)
{
    lua_createtable(L, 4, 0);
    lua_pushnumber(L, con.x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, con.y);
    lua_rawseti(L, -2, 2);
    lua_pushnumber(L, con.width);
    lua_rawseti(L, -2, 3);
    lua_pushnumber(L, con.height);
    lua_rawseti(L, -2, 4);
}

void lua_get_default_layout_data(lua_State *L)
{
    struct wlr_fbox con;
    lua_createtable(L, 1, 0);
    {
        lua_createtable(L, 1, 0);
        {
            con = (struct wlr_fbox) {
                .x = 0,
                    .y = 0,
                    .width = 1,
                    .height = 1,
            };
            lua_create_container(L, con);
            lua_rawseti(L, -2, 1);
        }
        lua_rawseti(L, -2, 1);
    }
}

void lua_get_default_master_layout_data(lua_State *L)
{
    lua_createtable(L, 1, 0);
    int i = 1;
    {
        lua_createtable(L, 1, 0);
        int j = 1;
        {
            struct wlr_fbox con = (struct wlr_fbox) {
                .x = 0,
                .y = 0,
                .width = 1,
                .height = 1,
            };
            lua_create_container(L, con);
            lua_rawseti(L, -2, j++);
        }
        lua_rawseti(L, -2, i++);
    }
    {
        lua_createtable(L, 1, 0);
        int j = 1;
        {
            struct wlr_fbox con = (struct wlr_fbox) {
                .x = 0,
                .y = 0,
                .width = 1,
                .height = 0.5,
            };
            lua_create_container(L, con);
            lua_rawseti(L, -2, j++);
        }
        {
            struct wlr_fbox con = (struct wlr_fbox) {
                .x = 0,
                .y = 0.5,
                .width = 1,
                .height = 0.5,
            };
            lua_create_container(L, con);
            lua_rawseti(L, -2, j++);
        }
        lua_rawseti(L, -2, i++);
    }
    {
        lua_createtable(L, 1, 0);
        int j = 1;
        {
            struct wlr_fbox con = (struct wlr_fbox) {
                .x = 0,
                .y = 0,
                .width = 1,
                .height = 0.333,
            };
            lua_create_container(L, con);
            lua_rawseti(L, -2, j++);
        }
        {
            struct wlr_fbox con = (struct wlr_fbox) {
                .x = 0,
                .y = 0.333,
                .width = 1,
                .height = 0.333,
            };
            lua_create_container(L, con);
            lua_rawseti(L, -2, j++);
        }
        {
            struct wlr_fbox con = (struct wlr_fbox) {
                .x = 0,
                .y = 0.666,
                .width = 1,
                .height = 0.333,
            };
            lua_create_container(L, con);
            lua_rawseti(L, -2, j++);
        }
        lua_rawseti(L, -2, i++);
    }
}

void lua_get_default_resize_data(lua_State *L)
{
    lua_createtable(L, 1, 0);
    {
        lua_createtable(L, 1, 0);
        {
            for (int i = 1; i < 10; i++) {
                lua_pushinteger(L, i+1);
                lua_rawseti(L, -2, i);
            }
        }
        lua_rawseti(L, -2, 1);
    }
}

void lua_tocolor(float dest_color[static 4])
{
    for (int i = 0; i < 4; i++) {
        lua_rawgeti(L, -1, i+1);
        dest_color[i] = luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
}

void print_trace()
{
    void* callstack[128];
    int i, frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i) {
        printf("%s\n", strs[i]);
    }
    free(strs);
}

int exec(const char *cmd)
{
    int ret_val = 0;

    if (fork() == 0) {
        setsid();
        ret_val = execl("/bin/sh", "/bin/sh", "-c", cmd, (void *)NULL);
    }

    return ret_val;
}

bool is_approx_equal(double a, double b, double error_range)
{
    return fabs(a - b) < error_range;
}

void *get_on_list(GPtrArray *list, int i)
{
    if (i >= list->len)
        return NULL;
    if (i == INVALID_POSITION)
        return NULL;

    return g_ptr_array_index(list, i);
}

void *get_in_composed_list(GPtrArray *arrays, int i)
{
    for (int j = 0; j < arrays->len; j++) {
        GPtrArray *array = g_ptr_array_index(arrays, j);
        if (i >= 0 && i < array->len) {
            void *item = g_ptr_array_index(array, i);
            return item;
        }
        i -= array->len;

        if (i < 0)
            break;
    }

    // no item found
    return NULL;
}

GPtrArray *get_list_at_i_in_composed_list(GPtrArray *arrays, int i)
{
    for (int j = 0; j < arrays->len; j++) {
        GPtrArray *list = g_ptr_array_index(arrays, j);
        if (i >= 0 && i < list->len) {
            return list;
        }
        i -= list->len;

        if (i < 0)
            break;
    }

    // no item found
    return NULL;
}

int relative_index_to_absolute_index(int i, int j, int length)
{
    if (length <= 0)
        return INVALID_POSITION;

    int new_position = (i + j) % length;
    while (new_position < 0)
        new_position += length;

    return new_position;
}

static void *get_relative_item(GPtrArray *list, 
        void *(*get_item)(GPtrArray *, int i),
        int (*length_of)(GPtrArray *), int i, int j)
{
    int new_position = relative_index_to_absolute_index(i, j, length_of(list));
    return get_item(list, new_position);
}

void *get_relative_item_in_list(GPtrArray *array, int i, int j)
{
    return get_relative_item(array, get_on_list, length_of_list, i, j);
}

void *get_relative_item_in_composed_list(GPtrArray *arrays, int i, int j)
{
    return get_relative_item(arrays, get_in_composed_list, length_of_composed_list,
            i, j);
}

void delete_from_composed_list(GPtrArray *arrays, int i)
{
    for (int j = 0; j < arrays->len; j++) {
        GPtrArray *list = g_ptr_array_index(arrays, j);
        if (i >= 0 && i < list->len) {
            g_ptr_array_remove_index(list, i);
            return;
        }
        i -= list->len;

        if (i < 0)
            break;
    }

    // no item found
    return;
}

int length_of_list(GPtrArray *array)
{
    return array->len;
}

int length_of_composed_list(GPtrArray *array)
{
    int length = 0;
    for (int i = 0; i < array->len; i++) {
        GPtrArray *list = g_ptr_array_index(array, i);
        length += list->len;
    }
    return length;
}
