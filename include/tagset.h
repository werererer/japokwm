#ifndef TAGSET_H
#define TAGSET_H

#include <glib.h>
#include <stdlib.h>

#include "bitset/bitset.h"

struct client;
struct container;
struct tag;
struct monitor;

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

/* this creates a tagset with reference count of 1. Calling focus_tagset
 * afterwards also adds a ref counter of 1 therefore use focus_tagset_no_ref
 * instead.  */
void monitor_focus_tags(struct monitor *m, struct tag *tag, BitSet *workspaces);

void workspace_write_to_workspaces(struct tag *tag);

void tagset_focus_workspace(struct tag *tag);
void tagset_toggle_add(struct monitor *m, BitSet *bitset);
void tagset_set_tags(struct tag *sel_ws, BitSet *workspaces);
void tagset_focus_tags(struct tag *tag, struct BitSet *bitset);
void tagset_reload(struct tag *sel_ws);
void tagset_move_sticky_containers(struct tag *tag);

void workspace_write_to_focus_stacks(struct tag *tag);
bool is_reduced_focus_stack(struct tag *tag, struct container *con);
bool _is_reduced_focus_stack(
        void *workspace_ptr,
        GPtrArray *src_list,
        struct container *con
        );
void update_reduced_focus_stack(struct tag *tag);
bool is_local_focus_stack(struct tag *tag, struct container *con);
bool _is_local_focus_stack(
        void *workspace_ptr,
        GPtrArray *src_list,
        struct container *con
        );

bool is_visual_visible_stack(struct tag *tag, struct container *con);

bool container_intersects_with_monitor(struct container *con, struct monitor *m);

GPtrArray *tagset_get_floating_list_copy(struct tag *tag);
GPtrArray *workspace_get_complete_stack_copy(struct tag *tag);
GPtrArray *workspace_get_stack_copy(struct tag *tag);
GPtrArray *workspace_get_tiled_list(struct tag *tag);
GPtrArray *workspace_get_tiled_list_copy(struct tag *tag);

// get with server floating containers instead
GPtrArray *tagset_get_global_floating_copy(struct tag *tag);
GPtrArray *tagset_get_visible_list_copy(struct tag *tag);
GPtrArray *tagset_get_hidden_list_copy(struct tag *tag);

void workspace_id_to_tag(BitSet *dest, int ws_id);

bool container_viewable_on_monitor(struct monitor *m,
        struct container *con);
bool container_potentially_viewable_on_monitor(struct monitor *m,
        struct container *con);
bool exist_on(struct monitor *m, BitSet *workspaces, struct container *con);
bool tagset_contains_client(BitSet *workspaces, struct client *c);
bool visible_on(struct monitor *m, BitSet *workspaces, struct container *con);

bool tagset_exist_on(struct monitor *m, struct container *con);
bool tagset_visible_on(struct monitor *m, struct container *con);

void push_tagset(struct tag *tag, BitSet *workspaces);
void focus_tagset(struct tag *tag, BitSet *workspaces);

void tagset_workspaces_reconnect(struct tag *tag);
void tagset_workspaces_disconnect(struct tag *tag);
void tagset_workspaces_connect(struct tag *tag);

void tagset_load_workspaces();

#endif /* TAGSET_H */
