#ifndef TAGSET_H
#define TAGSET_H

#include <stdlib.h>
#include <wlr/types/wlr_list.h>

#include "bitset/bitset.h"
#include "client.h"
#include "list_set.h"

struct tagset {
    struct monitor *m;
    int selected_ws_id;
    BitSet workspaces;

    struct wlr_list loaded_layouts;
    struct layout *previous_layout;
    struct layout *layout;

    /* number of all windows in layout even if they are invisible). Note that
     * floating windows don't belong to the layout and are thereby not counted */
    int n_all;

    struct list_set list_set;
};

struct tagset *create_tagset(struct monitor *m, struct layout *lt, int selected_ws_id, BitSet workspaces);
void destroy_tagset(struct tagset *tagset);

void focus_most_recent_container(struct tagset *tagset, enum focus_actions a);
void focus_tagset(struct tagset *tagset);
void tagset_set_tags(struct tagset *tagset, BitSet bitset);
void tagset_set_workspace_id(int ws_id);
void tagset_toggle_add(struct tagset *tagset, BitSet bitset);
void tagset_toggle_add_workspace_id(struct tagset *tagset, int ws_id);

struct tagset *get_tagset_from_workspace_id(struct wlr_list *workspaces, int ws_id);

struct container *get_container(struct tagset *tagset, int i);

struct wlr_list *tagset_get_visible_lists(struct tagset *tagset);
struct wlr_list *tagset_get_tiled_list(struct tagset *tagset);
struct wlr_list *tagset_get_floating_list(struct tagset *tagset);
struct wlr_list *tagset_get_hidden_list(struct tagset *tagset);

BitSet workspace_id_to_tag(int ws_id);

// TODO change argument order
bool exist_on(struct container *con, struct tagset *tagset);
bool hidden_on(struct container *con, struct tagset *tagset);
bool tagset_contains_client(struct tagset *tagset, struct client *c);
bool tagset_has_clients(struct tagset *tagset);
bool visible_on(struct container *con, struct tagset *tagset);

void focus_tagset(struct tagset *tagset);
void push_layout(struct tagset *tagset, struct layout *lt);
void push_tagset(struct tagset *tagset);
void load_default_layout(lua_State *L);
void load_layout(lua_State *L, const char *name);
void reset_loaded_layout(struct tagset *tagset);
void remove_loaded_layouts(struct wlr_list *workspaces);
void tagset_set_tagset(struct tagset *tagset);
void tagset_unset_tagset(struct tagset *tagset);

int tagset_get_container_count(struct tagset *tagset);

#endif /* TAGSET_H */
