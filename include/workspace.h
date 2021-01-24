#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <stdio.h>
#include <stdlib.h>
#include <wlr/types/wlr_list.h>
#include "layout.h"
/* A tag is simply a workspace that can be focused (like a normal workspace)
 * and can selected: which just means that all clients on the selected tags
 * will be combined to be shown on the focused tag
 * using this struct requires to use tagsetCreate and later tagsetDestroy
 * */
struct workspace {
    size_t id;
    const char *name;
    struct layout layout[2];
    struct monitor *m;
};

struct workspace *create_workspace(const char *name, size_t id, struct layout lt);
void destroy_workspace(struct workspace *ws);

bool is_workspace_occupied(struct workspace *ws);
bool workspace_has_clients(struct workspace *ws);

int workspace_count();

struct workspace *find_next_unoccupied_workspace(struct workspace *ws);
struct workspace *get_workspace(size_t i);

void init_workspaces();
void create_workspaces(struct wlr_list tagNames, struct layout default_layout);
void workspace_assign_monitor(struct workspace *ws, struct monitor *m);
void destroy_workspaces();
void load_default_layout(lua_State *L, struct workspace *ws);
void set_layout(lua_State *L, struct workspace *ws, int layouts_ref);
void load_layout(lua_State *L, struct workspace *ws, const char *layout_name);
void set_selected_layout(struct workspace *ws, struct layout layout);
void set_next_unoccupied_workspace(struct monitor *m, struct workspace *ws);
void copy_layout_from_selected_workspace();
/* sets the value of selTag[0] */
void set_workspace(struct monitor *m, struct workspace *ws);
void push_workspace(struct workspace *ws_stack[static 2], struct workspace *ws);

#endif /* WORKSPACE_H */
