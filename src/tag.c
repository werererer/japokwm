#include "tag.h"
#include "layout.h"
#include <string.h>

void tagCreate(struct tag *tag, const char *name)
{
    tag->lt = &defaultLayout;
    malloc(strlen(name)*sizeof(char));
    strcpy(tag->name, name);
}

void tagDestroy(struct tag *tag)
{
    free(tag->name);
}
