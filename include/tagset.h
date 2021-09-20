#ifndef TAGSET_H
#define TAGSET_H

#include <glib.h>
#include <stdlib.h>

#include "bitset/bitset.h"

struct client;
struct container;
struct workspace;

/* A tagset consists of a list_set, a struct to hold all relevant containers,
 * information on which monitor it belongs to and the list of workspaces it
 * currently shows.
 * If a tagset is created it will be added to the server's tagsets array.
 * If a tagset is destroyed it will be be removed from the server's tagsets
 * array.
 * If a tagset is loaded list_set represents the containers loaded and events
 * such as destroy container can alter this list.
 * If a tagset is not loaded the tagset wont change it's state without doing it
 * explicitly.
 * A tagset will be unloaded when destroyed.
 * loading is the action of populating the list_set.
 * unloading is the action of cleaning the list_set.
 * OVERLAPPING WORKSPACES:
 * If another workspace is added to the current tagset it is checked if
 * other loaded tagsets, which are on other monitors, contain the same workspace.
 * If the workspace is not attached to the other monitor then it is removed from
 * this tagset. Otherwise the workspace will be temporarily pulled over to the
 * current tagset. As soon as the workspace is unselected it moves back to its
 * prior monitor.
 * LOADING TAGSETS:
 * select the monitor of the tagset and load it.
 * SELECTING WORKSPACE:
 * create a new tagset with the given workspace and load it.
 * CREATING TAGSET:
 * All 
 * */
struct tagset {
    struct monitor *m;
    int selected_ws_id;
    BitSet *workspaces;
    BitSet *loaded_workspaces;

    struct container_set *con_set;
    struct focus_set *visible_focus_set;
    struct focus_set *local_focus_set;
    struct visual_set *visible_visual_set;

    bool applied_action;
};

/* this creates a tagset with reference count of 1. Calling focus_tagset
 * afterwards also adds a ref counter of 1 therefore use focus_tagset_no_ref
 * instead.  */
struct tagset *create_tagset(struct monitor *m, int selected_ws_id, BitSet *workspaces);
void destroy_tagset(struct tagset *tagset);

void focus_tagset(struct tagset *tagset);
void tagset_write_to_workspaces(struct tagset *tagset);
void tagset_focus_workspace(int ws_id);
void tagset_toggle_add(struct tagset *tagset, BitSet *bitset);
void tagset_focus_tags(int ws_id, struct BitSet *bitset);
void tagset_reload(struct tagset *tagset);
void tagset_move_sticky_containers(struct tagset *old_tagset, struct tagset *tagset);

void tagset_write_to_focus_stacks(struct tagset *tagset);
void update_sub_focus_stack(struct tagset *tagset);
bool is_reduced_focus_stack(struct workspace *ws, struct container *con);
void update_reduced_focus_stack(struct tagset *tagset);
void update_local_focus_stack(struct tagset *tagset);

void update_visual_visible_stack(struct tagset *tagset);

bool container_intersects_with_monitor(struct container *con, struct monitor *m);

// get with server floating containers instead
GPtrArray *server_update_floating_containers();
GPtrArray *tagset_get_visible_lists(struct tagset *tagset);
GPtrArray *tagset_get_global_floating_copy(struct tagset *tagset);
GPtrArray *tagset_get_tiled_list_copy(struct tagset *tagset);
GPtrArray *tagset_get_tiled_list(struct tagset *tagset);
GPtrArray *tagset_get_floating_list_copy(struct tagset *tagset);
GPtrArray *tagset_get_visible_list_copy(struct tagset *tagset);
GPtrArray *tagset_get_hidden_list_copy(struct tagset *tagset);

void tagset_list_remove(GPtrArray *list, struct container *con);
void tagset_list_remove_index(GPtrArray *list, int i);
void tagset_list_add(GPtrArray *list, struct container *con);
void tagset_list_insert(GPtrArray *list, int i, struct container *con);
struct container *tagset_list_steal_index(GPtrArray *list, int i);

void workspace_id_to_tag(BitSet *dest, int ws_id);

bool container_viewable_on_monitor(struct monitor *m,
        struct container *con);
bool container_potentially_viewable_on_monitor(struct monitor *m,
        struct container *con);
bool exist_on(struct monitor *m, BitSet *workspaces, struct container *con);
bool tagset_contains_client(BitSet *workspaces, struct client *c);
bool visible_on(struct monitor *m, BitSet *workspaces, struct container *con);
bool tagset_is_visible(struct tagset *tagset);

bool tagset_exist_on(struct tagset *tagset, struct container *con);
bool tagset_visible_on(struct tagset *tagset, struct container *con);

// adds a refcount of 1 to tagset
void focus_tagset(struct tagset *tagset);
void focus_tagset(struct tagset *tagset);
// adds a refcount of 1 to tagset
void push_tagset(struct tagset *tagset);
void push_tagset(struct tagset *tagset);

void tagset_workspaces_disconnect(struct tagset *tagset);
void tagset_workspaces_connect(struct tagset *tagset);

void tagset_unload_workspaces(struct tagset *tagset);
void tagset_load_workspaces(struct tagset *tagset, BitSet *workspaces);

struct layout *tagset_get_layout(struct tagset *tagset);

struct workspace *tagset_get_workspace(struct tagset *tagset);

#endif /* TAGSET_H */
