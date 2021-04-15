#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <stdio.h>
#include <stdlib.h>

#include "layout.h"
#include "container.h"
#include "bitset/bitset.h"
#include "container_lists.h"

/* A tag is simply a workspace that can be focused (like a normal workspace)
 * and can selected: which just means that all clients on the selected tags
 * will be combined to be shown on the focused tag
 * using this struct requires to use tagsetCreate and later tagsetDestroy
 * */
struct workspace {
    BitSet bitset;
    size_t id;
    const char *name;
    struct wlr_list loaded_layouts;
    struct layout *previous_layout;
    struct layout *layout;
    struct monitor *m;

    /* number of all windows in layout even if they are invisible). Note that
     * floating windows don't belong to the layout and are thereby not counted */
    int n_all;

    struct container_lists lists;
};

struct workspace *create_workspace(const char *name, size_t id, struct layout *lt);
void destroy_workspace(struct workspace *ws);

void update_workspaces(struct wlr_list *workspaces, struct wlr_list *tag_names);
void update_workspace_ids(struct wlr_list *workspaces);

bool exist_on(struct container *con, struct monitor *m);
bool is_workspace_occupied(struct workspace *ws);
bool hidden_on(struct container *con, struct monitor *m);
bool visible_on(struct container *con, struct monitor *m);
bool workspace_has_clients(struct workspace *ws);

int get_workspace_container_count(struct workspace *ws);
bool is_workspace_empty(struct workspace *ws);

struct container *get_container(struct monitor *m, int i);

struct workspace *find_next_unoccupied_workspace(struct wlr_list *workspaces, struct workspace *ws);
struct workspace *get_workspace(int id);
struct workspace *get_next_empty_workspace(struct wlr_list *workspaces, size_t i);
struct workspace *get_prev_empty_workspace(struct wlr_list *workspaces, size_t i);

struct wlr_list *get_visible_lists(struct workspace *ws);
struct wlr_list *get_tiled_list(struct workspace *ws);
struct wlr_list *get_floating_list(struct workspace *ws);
struct wlr_list *get_hidden_list(struct workspace *ws);
struct wlr_list *get_focus_stack_lists(struct workspace *ws);

void add_container_to_containers(struct container *con, struct monitor *m, int i);
void add_container_to_focus_stack(struct container *con, struct monitor *m);
void add_container_to_stack(struct container *con);
void focus_most_recent_container(struct monitor *m, enum focus_actions a);
void focus_next_unoccupied_workspace(struct monitor *m, struct wlr_list *workspaces, struct workspace *ws);
void copy_layout_from_selected_workspace(struct wlr_list *workspaces);
void create_workspaces(struct wlr_list *workspaces, struct wlr_list *tag_names,
        struct layout *default_layout);
void destroy_workspaces(struct wlr_list *workspaces);
void load_default_layout(lua_State *L, struct workspace *ws);
void load_layout(lua_State *L, const char *name);
void reset_loaded_layout(struct workspace *ws);
void reset_loaded_layouts(struct wlr_list *workspaces);
void set_container_workspace(struct container *con, struct workspace *ws);
void set_layout(lua_State *L);
void set_selected_layout(struct workspace *ws, struct layout *layout);
void move_container_to_workspace(struct container *con, struct workspace *ws);
void workspace_assign_monitor(struct workspace *ws, struct monitor *m);
void rename_workspace(struct workspace *ws, const char *name);
void focus_workspace(struct monitor *m, struct workspace_selector *ws_selector);
void push_workspace(struct monitor *m, struct workspace_selector ws_selector);
void push_layout(struct workspace *ws, struct layout *lt);

#endif /* WORKSPACE_H */
