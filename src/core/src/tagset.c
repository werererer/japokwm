#include "tagset.h"
#include <stdlib.h>

void tagsetCreate(struct tagset *tagset)
{
    tagset->tagNames = calloc(NUM_CHARS * MAXLEN, sizeof(char));
}

void tagsetDestroy(struct tagset *tagset)
{
    free(tagset->tagNames);
}

unsigned int tagPositionToFlag(enum tagPosition pos)
{
    return (1 << pos);
}

void setSelTags(struct tagset *tagset, unsigned int selTags)
{
    tagset->selTags[0] = selTags;
}

void toggleAddTag(struct tagset *tagset, unsigned int selTags)
{
    setSelTags(tagset, tagset->selTags[0] | selTags);
}

void pushSelTags(struct tagset *tagset, unsigned int selTags)
{
    tagset->selTags[1] = tagset->selTags[0];
    setSelTags(tagset, selTags);
}

void invertTagset(struct tagset *tagset)
{
    pushSelTags(tagset, tagset->selTags[0] ^ 1);
}

void toggleTagset(struct tagset *tagset)
{
    unsigned int ts = tagset->selTags[1];
    tagset->selTags[1] = tagset->selTags[0];
    tagset->selTags[0] = ts;
}
