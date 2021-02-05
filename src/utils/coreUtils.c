#include "utils/coreUtils.h"
#include <string.h>
#include <unistd.h>
#include <wordexp.h>
#include <stdlib.h>
#include <execinfo.h>

struct lua_State *L;

bool file_exists(const char *path) {
    return path && access(path, R_OK) != -1;
}

void wlr_list_clear(struct wlr_list *list)
{
    wlr_list_finish(list);
    wlr_list_init(list);
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

void lua_get_basic_layout()
{
    lua_createtable(L, 1, 0);

    lua_createtable(L, 1, 0);

    lua_createtable(L, 4, 0);
    lua_pushinteger(L, 0);

    lua_rawseti(L, -2, 1);
    lua_pushinteger(L, 0);
    lua_rawseti(L, -2, 2);
    lua_pushinteger(L, 1);
    lua_rawseti(L, -2, 3);
    lua_pushinteger(L, 1);
    lua_rawseti(L, -2, 4);

    lua_rawseti(L, -2, 1);

    lua_rawseti(L, -2, 1);
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
