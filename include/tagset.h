#ifndef TAGSET_H
#define TAGSET_H

#include "layout.h"
#include "tag.h"
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_list.h>

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

/* *
 * A tagset contains a list of tags
 * */
struct tagset {
    /* *
     * selTags are flags that should that represent the selected Tags in binary.
     * At position 0 the current selTags are stored.
     * At position 1 the previous selTags are stored.
     * */
    unsigned int selTags[2];
    /* position of current selected tag count starts at 0 */
    unsigned int focusedTag;
    /* list of tags */
    struct wlr_list tags;
};

struct tagset *create_tagset(struct wlr_list *tagNames, unsigned int focusedTag, unsigned int selTags);
void destroy_tagset(struct tagset *tagset);

void tagsetAddTag(struct tagset *tagset, struct tag *tag);
struct tag *get_tag_from_tagset(struct tagset *tagset, size_t i);
struct tag *focused_tag_from_tagset(struct tagset *tagset);

/* sets the value of selTag[0] */
void set_selelected_Tags(struct tagset *tagset, unsigned int selTags);
/* *
 * selTag[1] = selTag[0] then
 * selTag[0] = new value
 * */
void push_seleceted_tags(struct tagset *tagset, unsigned int selTags);
/*
 * bit xor to add selTags
 */
void toggle_add_tag(struct tagset *tagset, unsigned int selTags);

void invert_tagset(struct tagset *tagset);
void toggle_tagset(struct tagset *tagset);

bool tags_overlap(unsigned int tags, unsigned int tags2);

enum tagPosition position_to_flag(unsigned int pos);
unsigned int flag_to_position(enum tagPosition flag);
struct layout selected_layout(struct tagset *tagset);
void set_selected_layout(struct tagset *tagset, struct layout layout);
#endif /* TAGSET_H */
