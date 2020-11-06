#include "utils/coreUtils.h"
#include <string.h>

struct wlr_seat *seat;
struct lua_State *L;

void wlr_list_clear(struct wlr_list *list)
{
    wlr_list_finish(list);
    wlr_list_init(list);
}

char lastChar(char *str)
{
    return str[strlen(str)-1];
}

void joinPath(char *base, char *file)
{
    if (lastChar(file) != '/')
        strcat(base, "/");
    strcat(base, file);
}
