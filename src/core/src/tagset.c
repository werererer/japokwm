#include "tagset.h"
#include <math.h>
#include <tgmath.h>
#include <stdlib.h>

void tagsetCreate(struct tagset *tagset)
{
    tagset->tagNames = calloc(NUM_CHARS * MAXLEN, sizeof(char));
    tagset->lt = calloc(MAXLEN, sizeof(*tagset->lt));
}

void tagsetDestroy(struct tagset *tagset)
{
    free(tagset->tagNames);
    free(tagset->lt);
}

enum tagPosition flagToTagPosition(unsigned int flag)
{
    if (flag % 2 == 0 || flag < 2) {
        // inverse of 2^(x-1)
        return log2(flag)+1;
    } else {
        return TAG_ONE;
    }
}

unsigned int tagPositionToFlag(enum tagPosition pos)
{
    /* tagPosition begins at 1 */
    return (1 << (pos-1));
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

struct layout *selLayout(struct tagset *tagset)
{
    return tagset->lt[tagset->focusedTag];
}

struct layout *setSelLayout(struct tagset *tagset, struct layout *layout)
{
    return tagset->lt[tagset->focusedTag] = layout;
}
