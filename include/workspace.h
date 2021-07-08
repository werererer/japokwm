#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <stdio.h>
#include <stdlib.h>
#include <wlr/types/wlr_list.h>
#include "layout.h"
#include "list_set.h"

/* A tag is simply a workspace that can be focused (like a normal workspace)
 * and can selected: which just means that all clients on the selected tags
 * will be combined to be shown on the focused tag
 * using this struct requires to use tagsetCreate and later tagsetDestroy
 * */
struct workspace {
    struct wlr_list loaded_layouts;
    struct layout *previous_layout;
    struct layout *layout;

    size_t id;
    const char *name;
    struct monitor *m;

    // the tagset that currently hast this workspace selected
    struct tagset *tagset;

    struct list_set list_set;
};

struct workspace *create_workspace(const char *name, size_t id, struct layout *lt);
void destroy_workspace(struct workspace *ws);

void update_workspaces(struct wlr_list *workspaces, struct wlr_list *tag_names);
void update_workspace_ids(struct wlr_list *workspaces);

bool is_workspace_occupied(struct workspace *ws);

int get_workspace_container_count(struct workspace *ws);
bool is_workspace_empty(struct workspace *ws);

struct workspace *find_next_unoccupied_workspace(struct wlr_list *workspaces, struct workspace *ws);
struct workspace *get_workspace(int id);
struct workspace *get_next_empty_workspace(struct wlr_list *workspaces, size_t i);
struct workspace *get_prev_empty_workspace(struct wlr_list *workspaces, size_t i);

void focus_next_unoccupied_workspace(struct monitor *m, struct wlr_list *workspaces, struct workspace *ws);
void create_workspaces(struct wlr_list *workspaces, struct wlr_list *tag_names);
void copy_layout_from_selected_workspace(struct wlr_list *workspaces);
void destroy_workspaces(struct wlr_list *workspaces);
void layout_set_set_layout(lua_State *L);
void push_layout(struct workspace *ws, struct layout *lt);
void load_default_layout(lua_State *L);
void load_layout(lua_State *L, const char *name);
void reset_loaded_layout(struct workspace *ws);
void remove_loaded_layouts(struct wlr_list *workspaces);
void workspace_assign_monitor(struct workspace *ws, struct monitor *m);
void rename_workspace(struct workspace *ws, const char *name);

#endif /* WORKSPACE_H */
