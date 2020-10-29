#ifndef TAGSET_H
#define TAGSET_H

#include "layout.h"
#include <wlr/types/wlr_box.h>

// TODO: support 32 tags if possible
enum tagPosition {
    TAG_ONE = (1 << 0),
    TAG_TWO = (1 << 1),
    TAG_THREE = (1 << 2),
    TAG_FOUR = (1 << 3),
    TAG_FIVE = (1 << 4),
    TAG_SIX = (1 << 5),
    TAG_SEVEN = (1 << 6),
    TAG_EIGHT = (1 << 7),
    TAG_NINE = (1 << 8),
    TAG_TEN = (1 << 9),
};

#define MAXLEN 15
#define NUM_CHARS 64
#define NUM_DIGITS 9

/* A tag is simply a workspace that can be focused (like a normal workspace)
 * and can selected: which just means that all clients on the selected tags
 * will be combined to be shown on the focused tag
 * using this struct requires to use tagsetCreate and later tagsetDestroy
 * */
struct tagset {
    /* position of current selected tag count starts at 0 */
    unsigned int focusedTag;
    char **tagNames;
    struct layout *lt;
    /* window area(area where windows can tile) */
    struct wlr_box w;
    /* *
     * selTags are flags that should that represent the selected Tags in binary.
     * At position 0 the current selTags are stored.
     * At position 1 the previous selTags are stored.
     * */
    unsigned int selTags[2];
};

void tagsetCreate(struct tagset *tagset);
void tagsetDestroy(struct tagset *tagset);

/* sets the value of selTag[0] */
void setSelTags(struct tagset *tagset, unsigned int selTags);
/* *
 * selTag[1] = selTag[0] then
 * selTag[0] = new value
 * */
void pushSelTags(struct tagset *tagset, unsigned int selTags);
/*
 * bit xor to add selTags
 */
void toggleAddTag(struct tagset *tagset, unsigned int selTags);

void invertTagset(struct tagset *tagset);
void toggleTagset(struct tagset *tagset);

bool tagsOverlap(unsigned int tags, unsigned int tags2);

enum tagPosition positionToFlag(unsigned int pos);
unsigned int flagToPosition(enum tagPosition flag);
struct layout selLayout(struct tagset *tagset);
struct layout setSelLayout(struct tagset *tagset, struct layout layout);
#endif /* TAGSET_H */
