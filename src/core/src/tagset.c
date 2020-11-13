#include "tagset.h"
#include <math.h>
#include <stdio.h>
#include <tgmath.h>
#include <stdlib.h>
#include <string.h>

struct tagset *tagsetCreate(struct wlr_list *tagNames)
{
    struct tagset *tagset = calloc(1, sizeof(struct tagset));
    wlr_list_init(&tagset->tags);

    for (int i = 0; i < tagNames->length; i++) {
        struct tag *tag = calloc(1, sizeof(struct tag));
        tag->name = tagNames->items[i];
        tag->layout = defaultLayout;
        wlr_list_push(&tagset->tags, tag);
    }

    tagset->focusedTag = 0;
    return tagset;
}

void tagsetDestroy(struct tagset *tagset)
{
    // delete all tags dynamically allocated by create
    for (int i = 0; i < tagset->tags.length; i++)
        free(wlr_list_pop(&tagset->tags));
    wlr_list_finish(&tagset->tags);
    free(tagset);
}

unsigned int flagToPosition(enum tagPosition flag)
{
    if (flag % 2 == 0 || flag < 2) {
        // inverse of (1 << (x-1)) = 2^(x-1)
        return log2(flag)+1;
    } else {
        return TAG_ONE;
    }
}

enum tagPosition positionToFlag(unsigned int pos)
{
    /* tagPosition begins at 1 */
    return (1 << (pos-1));
}

void setSelTags(struct tagset *tagset, unsigned int selTags)
{
    tagset->selTags[0] = selTags;
}

struct tag *tagsetGetTag(struct tagset *tagset, size_t i)
{
    return tagset->tags.items[i];
}

struct tag *tagsetFocusedTag(struct tagset *tagset)
{
    return tagsetGetTag(tagset, tagset->focusedTag);
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

bool tagsOverlap(unsigned int tags, unsigned int tags2)
{
    return tags & tags2;
}

struct layout selLayout(struct tagset *tagset)
{
    return tagsetFocusedTag(tagset)->layout;
}

void setSelLayout(struct tagset *tagset, struct layout layout)
{
    struct tag *tag = tagsetFocusedTag(tagset);
    if (strcmp(tag->name, "") == 0) {
        printf("ERROR: tag not initialized\n");
        return;
    }
    tag->layout = layout;
}
