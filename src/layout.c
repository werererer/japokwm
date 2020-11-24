#include "layout.h"
#include <stdlib.h>
#include <string.h>

struct layout defaultLayout;
struct layout prev_layout;

void layoutCreate(struct layout *lt, const char *symbol, int funcId)
{
    lt = malloc(sizeof(struct layout));
    lt->symbol = calloc(strlen(symbol), sizeof(char));
    strcpy(lt->symbol, symbol);
    lt->funcId = funcId;
}

void layoutDestroy(struct layout *lt)
{
    free(lt->symbol);
    free(lt);
}
