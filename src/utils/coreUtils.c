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

struct lua_State *L;

bool dir_exists(const char *path)
{
    DIR *dir = opendir(path);
    bool exists = errno == ENOENT;
    closedir(dir);
    return exists;
}

bool file_exists(const char *path)
{
    return access(path, R_OK) != -1;
}

void wlr_list_clear(struct wlr_list *list)
{
    wlr_list_finish(list);
    wlr_list_init(list);
}

bool wlr_list_empty(struct wlr_list *list)
{
    return list->length <= 0;
}

int wlr_list_remove(struct wlr_list *list,
        int (*compare)(const void *, const void *), const void *cmp_to)
{
    for (int i = 0; i < list->length; i++) {
        void *item = list->items[i];
        if (compare(item, cmp_to) == 0) {
            wlr_list_del(list, i);
            return 0;
        }
    }
    return 1;
}

int wlr_list_remove_in_composed_list(struct wlr_list *lists,
        int (*compare)(const void *, const void *), const void *cmp_to)
{
    for (int i = 0; i < lists->length; i++) {
        struct wlr_list *list = lists->items[i];
        if (wlr_list_remove(list, compare, cmp_to) == 0)
            return 0;
    }
    return 1;
}

int cmp_ptr(const void *ptr1, const void *ptr2)
{
    return ptr1 == ptr2 ? 0 : 1;
}

int wlr_list_find_in_composed_list(struct wlr_list *lists,
        int (*compare)(const void *, const void *), const void *cmp_to)
{
    int position = 0;
    for (int i = 0; i < lists->length; i++) {
        struct wlr_list *list = lists->items[i];
        for (int j = 0; j < list->length; j++) {
            void *item = list->items[j];
            if (compare(item, cmp_to) == 0) {
                return position;
            }
            position++;
        }
    }
    return -1;
}

struct wlr_list *wlr_list_find_list_in_composed_list(struct wlr_list *lists,
        int (*compare)(const void *, const void *), const void *cmp_to)
{
    for (int i = 0; i < lists->length; i++) {
        struct wlr_list *list = lists->items[i];
        for (int j = 0; j < list->length; j++) {
            void *item = list->items[j];
            if (compare(item, cmp_to) == 0) {
                return list;
            }
        }
    }
    return NULL;
}

char last_char(const char *str)
{
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

void join_path(char *base, const char *file)
{
    if (last_char(base) != '/' && file[0] != '/') {
        strcat(base, "/");
    } else if (last_char(base) == '/' && file[0] == ' ') {
        base[strlen(base)-1] = '\0';
    }
    strcat(base, file);
}

void lua_ref_safe(lua_State *L, int t, int *ref)
{
    if (*ref > 0) {
        luaL_unref(L, t, *ref);
    }

    *ref = luaL_ref(L, t);
}

static void lua_create_container(struct wlr_fbox con)
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

void lua_get_default_layout_data()
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
            lua_create_container(con);
            lua_rawseti(L, -2, 1);
        }
        lua_rawseti(L, -2, 1);
    }
}

void lua_get_default_master_layout_data()
{
    struct wlr_fbox con;
    lua_createtable(L, 1, 0);
    int i = 1;
    {
        lua_createtable(L, 1, 0);
        int j = 1;
        {
            con = (struct wlr_fbox) {
                .x = 0,
                .y = 0,
                .width = 1,
                .height = 1,
            };
            lua_create_container(con);
            lua_rawseti(L, -2, j++);
        }
        lua_rawseti(L, -2, i++);
    }
    {
        lua_createtable(L, 1, 0);
        int j = 1;
        {
            con = (struct wlr_fbox) {
                .x = 0,
                .y = 0,
                .width = 1,
                .height = 0.5,
            };
            lua_create_container(con);
            lua_rawseti(L, -2, j++);
        }
        {
            con = (struct wlr_fbox) {
                .x = 0,
                .y = 0.5,
                .width = 1,
                .height = 0.5,
            };
            lua_create_container(con);
            lua_rawseti(L, -2, j++);
        }
        lua_rawseti(L, -2, i++);
    }
    {
        lua_createtable(L, 1, 0);
        int j = 1;
        {
            con = (struct wlr_fbox) {
                .x = 0,
                .y = 0,
                .width = 1,
                .height = 0.333,
            };
            lua_create_container(con);
            lua_rawseti(L, -2, j++);
        }
        {
            con = (struct wlr_fbox) {
                .x = 0,
                .y = 0.333,
                .width = 1,
                .height = 0.333,
            };
            lua_create_container(con);
            lua_rawseti(L, -2, j++);
        }
        {
            con = (struct wlr_fbox) {
                .x = 0,
                .y = 0.666,
                .width = 1,
                .height = 0.333,
            };
            lua_create_container(con);
            lua_rawseti(L, -2, j++);
        }
        lua_rawseti(L, -2, i++);
    }
}

void lua_get_default_resize_data()
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

void *get_on_list(struct wlr_list *list, int i)
{
    if (i >= list->length)
        return NULL;
    if (i == INVALID_POSITION)
        return NULL;

    return list->items[i];
}

void *get_in_composed_list(struct wlr_list *lists, int i)
{
    for (int j = 0; j < lists->length; j++) {
        struct wlr_list *list = lists->items[j];
        if (i >= 0 && i < list->length) {
            void *item = list->items[i];
            return item;
        }
        i -= list->length;

        if (i < 0)
            break;
    }

    // no item found
    return NULL;
}

struct wlr_list *get_list_at_i_in_composed_list(struct wlr_list *lists, int i)
{
    for (int j = 0; j < lists->length; j++) {
        struct wlr_list *list = lists->items[j];
        if (i >= 0 && i < list->length) {
            return list;
        }
        i -= list->length;

        if (i < 0)
            break;
    }

    // no item found
    return NULL;
}

int relative_index_to_absolute_index(int i, int j, int length)
{
    int new_position = (i + j) % length;
    while (new_position < 0)
        new_position += length;

    return new_position;
}

static void *get_relative_item(struct wlr_list *list, 
        void *(*get_item)(struct wlr_list *, int i),
        int (*length_of)(struct wlr_list *), int i, int j)
{
    int new_position = relative_index_to_absolute_index(i, j, length_of(list));
    return get_item(list, new_position);
}

void *get_relative_item_in_list(struct wlr_list *list, int i, int j)
{
    return get_relative_item(list, get_on_list, length_of_list, i, j);
}

void *get_relative_item_in_composed_list(struct wlr_list *lists, int i, int j)
{
    return get_relative_item(lists, get_in_composed_list, length_of_composed_list,
            i, j);
}

void remove_from_composed_list(struct wlr_list *lists, int i)
{
    for (int j = 0; j < lists->length; j++) {
        struct wlr_list *list = lists->items[j];
        if (i >= 0 && i < list->length) {
            wlr_list_del(list, i);
            return;
        }
        i -= list->length;

        if (i < 0)
            break;
    }

    // no item found
    return;
}

int length_of_list(struct wlr_list *list)
{
    return list->length;
}

int length_of_composed_list(struct wlr_list *lists)
{
    int length = 0;
    for (int i = 0; i < lists->length; i++) {
        struct wlr_list *list = lists->items[i];
        length += list->length;
    }
    return length;
}
