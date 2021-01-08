#include "layout.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct layout default_layout;
struct layout prev_layout;

void create_layout(struct layout *lt, const char *symbol, int funcId)
{
    lt = malloc(sizeof(struct layout));
    lt->nmaster = 1;
    lt->lua_func_index = funcId;
    lt->symbol = symbol;
}

void destroy_layout(struct layout *lt)
{
    free(lt);
}

bool is_same_layout(struct layout layout, struct layout layout2)
{
    // same string means same layout
    return strcmp(layout.symbol, layout2.symbol) != 0;
}
