#ifndef TAG_H
#define TAG_H

#include <stdio.h>
#include <stdlib.h>
#include "utils/coreUtils.h"

#include "bitset/bitset.h"

struct layout;
struct focus_set;
struct client;
struct container;
struct server;
struct tagset;

/* when an action should change the tag and the tagsets associated with it
 * you should use this macro.
 * NOTE: use to jump to the end of the current action*/
#define DO_ACTION_LOCALLY(_tag, action) \
    do {\
        struct container_set *con_set = _tag->con_set;\
        do {\
            action\
        } while (0);\
        \
        do {\
            struct monitor *m = container_get_monitor(con);\
            struct tag *_tag = monitor_get_active_tag(m);\
            if (!_tag)\
                continue;\
            struct container_set *con_set = _tag->visible_con_set;\
            action\
        } while (0);\
    } while (0)

#define DO_ACTION_GLOBALLY(tags, action) \
    do {\
        for (GList *_iter = tags; _iter; _iter = _iter->next) {\
            struct tag *_tag = _iter->data;\
            \
            struct container_set *con_set = _tag->con_set;\
            do {\
                action\
            } while (0);\
            \
        }\
        do {\
            struct monitor *m = container_get_monitor(con);\
            struct tag *_tag = monitor_get_active_tag(m);\
            if (!_tag)\
                continue;\
            struct container_set *con_set = _tag->visible_con_set;\
            action\
        } while (0);\
    } while (0)

struct tag {
    GPtrArray *loaded_layouts;
    const char *current_layout;
    const char *previous_layout;

    size_t id;
    char *name;

    BitSet *tags;
    BitSet *prev_tags;
    // the monitor the tag is locked to
    struct monitor *m;
    // the monitor the tag is currently on
    struct monitor *current_m;

    bool is_loaded;

    struct container_set *con_set;
    struct focus_set *focus_set;

    struct container_set *visible_con_set;
    struct focus_set *visible_focus_set;

    // whether the tag needs to be reloaded
    bool damaged;

    /* should anchored layershell programs be taken into consideration */
    enum wlr_edges visible_bar_edges;
};

GHashTable *create_tags();
void destroy_tags(GHashTable *tags);

void load_tags(GList *tags, GPtrArray *tag_names);

struct tag *create_tag(const char *name, size_t id, struct layout *lt);
void destroy_tag(struct tag *tag);

void update_tags(GList *tags, GPtrArray *tag_names);
void update_tag_ids(GPtrArray *tags);

bool is_tag_occupied(struct tag *tag);
bool tag_is_visible(struct tag *tag, struct monitor *m);
bool is_tag_the_selected_one(struct tag *tag);
bool is_tag_extern(struct tag *tag);
bool tag_is_active(struct tag *tag);
bool is_tag_empty(struct tag *tag);
bool tag_has_no_visible_containers(struct tag *tag);

int get_tag_container_count(struct tag *tag);

struct container *get_container(struct tag *tag, int i);
struct container *get_container_in_stack(struct tag *tag, int i);
struct container *tag_get_focused_container(struct tag *tag);

struct tag *find_next_unoccupied_tag(GList *tags, struct tag *tag);
struct tag *get_next_empty_tag(GList *tags, size_t i);
struct tag *get_prev_empty_tag(GList *tags, size_t tag_id);
struct tag *get_nearest_empty_tag(GList *tags, int tag_id);

struct layout *tag_get_layout(struct tag *tag);
struct layout *tag_get_previous_layout(struct tag *tag);
struct root *tag_get_root(struct tag *tag);
struct wlr_box tag_get_active_geom(struct tag *tag);

struct monitor *tag_get_selected_monitor(struct tag *tag);
struct monitor *tag_get_monitor(struct tag *tag);
void tag_set_selected_monitor(struct tag *tag, struct monitor *m);
void tag_set_current_monitor(struct tag *tag, struct monitor *m);
void tag_swap(struct tag *tag1, struct tag *tag2);
void tag_swap_smart(struct tag *tag1, struct tag *tag2);

void destroy_tags(GHashTable *tags);
void push_layout(struct tag *tag, const char *layout_name);
void set_default_layout(struct tag *tag);
void focus_layout(struct tag *tag, const char *name);
int tag_load_layout(struct tag *tag, const char *layout_name);
void tag_remove_loaded_layouts(struct tag *tag);
void tags_remove_loaded_layouts(GList *tags);
void tag_rename(struct tag *tag, const char *name);
void tag_update_name(struct tag *tag);
void tag_update_names(GList *tags);
struct container *tag_get_focused_container(struct tag *tag);

void tag_focus_first_container(struct tag *tag);
void tag_this_focus_most_recent_container();
void tag_focus_most_recent_container(struct tag *tag);
void tag_this_focus_container(struct container *con);
void tag_focus_container(struct tag *tag, struct container *con);
void tag_add_container_to_containers(struct tag *tag, int i, struct container *con);
void tag_add_container_to_focus_stack(struct tag *tag, int i, struct container *con);
void remove_container_from_stack(struct tag *tag, struct container *con);
void add_container_to_stack(struct tag *tag, struct container *con);
void add_container_to_layer_stack(struct container *con);

void list_set_insert_container_to_focus_stack(struct focus_set *focus_set, int position, struct container *con);
void tag_remove_container_from_containers_locally(struct tag *tag, struct container *con);
void tag_add_container_to_containers_locally(struct tag *tag, int i, struct container *con);
void tag_remove_container_from_focus_stack_locally(struct tag *tag, struct container *con);
void tag_add_container_to_focus_stack_locally(struct tag *tag, struct container *con);
void tag_remove_container_from_floating_stack_locally(struct tag *tag, struct container *con);
void tag_add_container_to_floating_stack_locally(struct tag *tag, int i, struct container *con);

void tag_remove_container_from_visual_stack_layer(struct tag *tag, struct container *con);
void tag_add_container_to_visual_stack_layer(struct tag *tag, struct container *con);

void tag_remove_container(struct tag *tag, struct container *con);
void tag_remove_container_from_focus_stack(struct tag *tag, struct container *con);

void tag_set_tags(struct tag *tag, BitSet *tags);
void tag_set_prev_tags(struct tag *tag, BitSet *tags);

GArray *container_array2D_get_positions_array(GPtrArray2D *containers);
GArray *container_array_get_positions_array(GPtrArray *containers);
void tag_repush_containers(GPtrArray *array, int i, int abs_index);
void tag_swap_containers(GPtrArray *array, int i, int j);
void tag_repush_tag(struct tag *tag, struct container *con, int new_pos);
void tag_repush_on_focus_stack(struct tag *tag, struct container *con, int i);

bool tag_sticky_contains_client(struct tag *tag, struct client *client);

#endif /* TAG_H */
