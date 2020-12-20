#include "utils/coreUtils.h"
#include <string.h>
#include <unistd.h>
#include <wordexp.h>
#include <stdlib.h>

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
