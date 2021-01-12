#include "layout.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lua.h>

#include "utils/coreUtils.h"

struct layout default_layout;
struct layout prev_layout;

void create_layout(struct layout *lt, const char *name, const char *symbol)
{
    lt = malloc(sizeof(struct layout));

    lt->nmaster = 1;
    lt->name = name;
    lt->symbol = symbol;
}

void destroy_layout(struct layout *lt)
{
    free(lt);
}

bool is_same_layout(struct layout layout, struct layout layout2)
{
    const char *c = layout.symbol;
    const char *c2 = layout2.symbol;
    // same string means same layout
    return strcmp(c, c2) != 0;
}
