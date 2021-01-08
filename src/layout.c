#include "layout.h"
#include <stdlib.h>
#include <string.h>

struct layout default_layout;
struct layout prev_layout;

void create_layout(struct layout *lt, const char *symbol, int funcId)
{
    lt = malloc(sizeof(struct layout));
    lt->nmaster = 1;
    lt->lua_func_index = funcId;
    lt->symbol = symbol;
    lt->name = "";
}

void destroy_layout(struct layout *lt)
{
    free(lt);
}

bool is_same_layout(struct layout layout, struct layout layout2)
{
    // same string means same layout
    return strcmp(layout.name, layout2.name) != 0;
}
