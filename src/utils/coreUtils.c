#include "utils/coreUtils.h"
#include <string.h>

struct lua_State *L;

void wlr_list_clear(struct wlr_list *list)
{
    wlr_list_finish(list);
    wlr_list_init(list);
}

char last_char(const char *str)
{
    return str[strlen(str)-1];
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
