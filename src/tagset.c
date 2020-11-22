#include "tagset.h"
#include <math.h>
#include <stdio.h>
#include <tgmath.h>
#include <stdlib.h>
#include <string.h>
#include "ipc-server.h"

struct tagset *create_tagset(struct wlr_list *tagNames, 
        unsigned int focusedTag, unsigned int selTags)
{
    struct tagset *tagset = calloc(1, sizeof(struct tagset));
    wlr_list_init(&tagset->tags);

    for (int i = 0; i < tagNames->length; i++) {
        struct tag *tag = calloc(1, sizeof(struct tag));
        tag->name = tagNames->items[i];
        tag->layout = defaultLayout;
        wlr_list_push(&tagset->tags, tag);
    }

    tagset->focusedTag = focusedTag;
    set_selelected_Tags(tagset, selTags);
    return tagset;
}

void destroy_tagset(struct tagset *tagset)
{
    // delete all tags dynamically allocated by create
    for (int i = 0; i < tagset->tags.length; i++)
        free(wlr_list_pop(&tagset->tags));
    wlr_list_finish(&tagset->tags);
    free(tagset);
}

unsigned int flag_to_position(enum tagPosition flag)
{
    if (flag % 2 == 0 || flag < 2) {
        // inverse of (1 << x)) = 2^x)
        return log2(flag);
    } else {
        return TAG_ONE;
    }
}

enum tagPosition position_to_flag(unsigned int pos)
{
    return 1 << pos;
}

void set_selelected_Tags(struct tagset *tagset, unsigned int selTags)
{
    tagset->selTags[0] = selTags;
    ipc_event_workspace();
}

struct tag *get_tag_from_tagset(struct tagset *tagset, size_t i)
{
    return tagset->tags.items[i];
}

struct tag *focused_tag_from_tagset(struct tagset *tagset)
{
    return get_tag_from_tagset(tagset, tagset->focusedTag);
}

void toggle_add_tag(struct tagset *tagset, unsigned int selTags)
{
    set_selelected_Tags(tagset, tagset->selTags[0] ^ selTags);
}

void push_seleceted_tags(struct tagset *tagset, unsigned int selTags)
{
    tagset->selTags[1] = tagset->selTags[0];
    set_selelected_Tags(tagset, selTags);
}

void invert_tagset(struct tagset *tagset)
{
    push_seleceted_tags(tagset, tagset->selTags[0] ^ 1);
}

void toggle_tagset(struct tagset *tagset)
{
    unsigned int ts = tagset->selTags[1];
    tagset->selTags[1] = tagset->selTags[0];
    tagset->selTags[0] = ts;
}

bool tags_overlap(unsigned int tags, unsigned int tags2)
{
    return tags & tags2;
}

struct layout selected_layout(struct tagset *tagset)
{
    return focused_tag_from_tagset(tagset)->layout;
}

void set_selected_layout(struct tagset *tagset, struct layout layout)
{
    struct tag *tag = focused_tag_from_tagset(tagset);
    if (strcmp(tag->name, "") == 0) {
        printf("ERROR: tag not initialized\n");
        return;
    }
    tag->layout = layout;
}
