#ifndef TAGSET_H
#define TAGSET_H

#include <stdlib.h>

#include "bitset/bitset.h"
#include "client.h"
#include "list_set.h"
#include "layout.h"
#include "utils/coreUtils.h"

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
 * OVERLAPPING TAGSETS:
 * When a tagset is loaded it will be checked if another loaded workspace has
 * the same selected_ws_id. If so then it will be checked if they have the same
 * monitor. If that is not the case the new tagset
 * unload the old tagset and load the new.
 * */
struct tagset {
    struct monitor *m;
    int selected_ws_id;
    BitSet workspaces;

    /* number of all windows in layout even if they are invisible). Note that
     * floating windows don't belong to the layout and are thereby not counted */
    int n_all;

    struct list_set *list_set;
    bool loaded;
};

struct tagset *create_tagset(struct monitor *m, int selected_ws_id, BitSet workspaces);
void destroy_tagset(struct tagset *tagset);

void focus_most_recent_container(struct tagset *tagset);
void focus_tagset(struct tagset *tagset);
void tagset_set_tags(struct tagset *tagset, BitSet bitset);
void tagset_focus_workspace(int ws_id);
void tagset_toggle_add(struct tagset *tagset, BitSet bitset);
void tagset_focus_tags(int ws_id, struct BitSet bitset);

struct tagset *get_tagset_from_active_workspace_id(int ws_id);
struct tagset *get_tagset_from_workspace_id(int ws_id);

struct container *get_container(struct tagset *tagset, int i);

GPtrArray *tagset_get_visible_lists(struct tagset *tagset);
GPtrArray *tagset_get_tiled_list(struct tagset *tagset);
GPtrArray *tagset_get_floating_list(struct tagset *tagset);
GPtrArray *tagset_get_hidden_list(struct tagset *tagset);

void workspace_id_to_tag(BitSet *dest, int ws_id);

bool exist_on(struct tagset *tagset, struct container *con);
bool tagset_contains_client(struct tagset *tagset, struct client *c);
bool visible_on(struct tagset *tagset, struct container *con);

void focus_tagset(struct tagset *tagset);
void push_tagset(struct tagset *tagset);

struct layout *tagset_get_layout(struct tagset *tagset);

#endif /* TAGSET_H */
