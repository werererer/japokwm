#ifndef WORKSPACE_H
#define WORKSPACE_H

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

/* when an action should change the workspace and the tagsets associated with it
 * you should use this macro.
 * NOTE: use to jump to the end of the current action*/
#define DO_ACTION_LOCALLY(_ws, action) \
    do {\
        struct container_set *con_set = _ws->con_set;\
        do {\
            action\
        } while (0);\
        \
        do {\
            struct monitor *m = container_get_monitor(con);\
            struct workspace *_ws = monitor_get_active_workspace(m);\
            if (!_ws)\
                continue;\
            struct container_set *con_set = _ws->visible_con_set;\
            action\
        } while (0);\
    } while (0)

#define DO_ACTION_GLOBALLY(workspaces, action) \
    do {\
        for (GList *_iter = workspaces; _iter; _iter = _iter->next) {\
            struct workspace *_ws = _iter->data;\
            \
            struct container_set *con_set = _ws->con_set;\
            do {\
                action\
            } while (0);\
            \
        }\
        do {\
            struct monitor *m = container_get_monitor(con);\
            struct workspace *_ws = monitor_get_active_workspace(m);\
            if (!_ws)\
                continue;\
            struct container_set *con_set = _ws->visible_con_set;\
            action\
        } while (0);\
    } while (0)

/* A tag is simply a workspace that can be focused (like a normal workspace)
 * and can selected: which just means that all clients on the selected tags
 * will be combined to be shown on the focused tag
 * using this struct requires to use tagsetCreate and later tagsetDestroy
 * */
struct workspace {
    GPtrArray *loaded_layouts;
    const char *current_layout;
    const char *previous_layout;

    size_t id;
    char *name;

    BitSet *tags;
    BitSet *prev_tags;
    // the monitor the workspace is locked to
    struct monitor *m;
    // the monitor the workspace is currently on
    struct monitor *current_m;

    bool is_loaded;

    struct container_set *con_set;
    struct focus_set *focus_set;

    struct container_set *visible_con_set;
    struct focus_set *visible_focus_set;

    // whether the workspace needs to be reloaded
    bool damaged;

    /* should anchored layershell programs be taken into consideration */
    enum wlr_edges visible_bar_edges;
};

GHashTable *create_workspaces();
void destroy_workspaces(GHashTable *workspaces);

void load_workspaces(GList *workspaces, GPtrArray *tag_names);

struct workspace *create_workspace(const char *name, size_t id, struct layout *lt);
void destroy_workspace(struct workspace *ws);

void update_workspaces(GList *workspaces, GPtrArray *tag_names);
void update_workspace_ids(GPtrArray *workspaces);

bool is_workspace_occupied(struct workspace *ws);
bool workspace_is_visible(struct workspace *ws, struct monitor *m);
bool is_workspace_the_selected_one(struct workspace *ws);
bool is_workspace_extern(struct workspace *ws);
bool workspace_is_active(struct workspace *ws);

int get_workspace_container_count(struct workspace *ws);
bool is_workspace_empty(struct workspace *ws);

void focus_most_recent_container();
struct container *get_container(struct workspace *ws, int i);
struct container *get_container_in_stack(struct workspace *ws, int i);

struct workspace *find_next_unoccupied_workspace(GList *workspaces, struct workspace *ws);
struct workspace *get_next_empty_workspace(GList *workspaces, size_t i);
struct workspace *get_prev_empty_workspace(GList *workspaces, size_t ws_id);
struct workspace *get_nearest_empty_workspace(GList *workspaces, int ws_id);

struct layout *workspace_get_layout(struct workspace *ws);
struct layout *workspace_get_previous_layout(struct workspace *ws);
struct root *workspace_get_root(struct workspace *ws);
struct wlr_box workspace_get_active_geom(struct workspace *ws);

struct monitor *workspace_get_selected_monitor(struct workspace *ws);
struct monitor *workspace_get_monitor(struct workspace *ws);
void workspace_set_selected_monitor(struct workspace *ws, struct monitor *m);
void workspace_set_current_monitor(struct workspace *ws, struct monitor *m);
void workspace_swap(struct workspace *ws1, struct workspace *ws2);
void workspace_swap_smart(struct workspace *ws1, struct workspace *ws2);

void destroy_workspaces(GHashTable *workspaces);
void push_layout(struct workspace *ws, const char *layout_name);
void set_default_layout(struct workspace *ws);
void focus_layout(struct workspace *ws, const char *name);
int workspace_load_layout(struct workspace *ws, const char *layout_name);
void workspace_remove_loaded_layouts(struct workspace *ws);
void workspaces_remove_loaded_layouts(GList *workspaces);
void workspace_rename(struct workspace *ws, const char *name);
void workspace_update_name(struct workspace *ws);
void workspace_update_names(GList *workspaces);
struct container *workspace_get_focused_container(struct workspace *ws);

void workspace_add_container_to_containers(struct workspace *ws, int i, struct container *con);
void workspace_add_container_to_focus_stack(struct workspace *ws, int i, struct container *con);
void remove_container_from_stack(struct workspace *ws, struct container *con);
void add_container_to_stack(struct workspace *ws, struct container *con);
void add_container_to_layer_stack(struct workspace *ws, struct container *con);

void list_set_insert_container_to_focus_stack(struct focus_set *focus_set, int position, struct container *con);
void workspace_remove_container_from_containers_locally(struct workspace *ws, struct container *con);
void workspace_add_container_to_containers_locally(struct workspace *ws, int i, struct container *con);
void workspace_remove_container_from_focus_stack_locally(struct workspace *ws, struct container *con);
void workspace_add_container_to_focus_stack_locally(struct workspace *ws, struct container *con);
void workspace_remove_container_from_floating_stack_locally(struct workspace *ws, struct container *con);
void workspace_add_container_to_floating_stack_locally(struct workspace *ws, int i, struct container *con);

void workspace_remove_container_from_visual_stack_layer(struct workspace *ws, struct container *con);
void workspace_add_container_to_visual_stack_layer(struct workspace *ws, struct container *con);

void workspace_remove_container(struct workspace *ws, struct container *con);
void workspace_remove_container_from_focus_stack(struct workspace *ws, struct container *con);

void workspace_set_tags(struct workspace *ws, BitSet *tags);
void workspace_set_prev_tags(struct workspace *ws, BitSet *tags);

GArray *container_array2D_get_positions_array(GPtrArray2D *containers);
GArray *container_array_get_positions_array(GPtrArray *containers);
void workspace_repush(GPtrArray *array, int i, int abs_index);
void workspace_repush_workspace(struct workspace *ws, struct container *con, int new_pos);
void workspace_repush_on_focus_stack(struct workspace *ws, struct container *con, int i);

bool workspace_sticky_contains_client(struct workspace *ws, struct client *client);

#endif /* WORKSPACE_H */
