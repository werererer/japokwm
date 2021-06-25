#include <errno.h>
#include <json.h>
#include <stdio.h>
#include <ctype.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_output.h>
#include <xkbcommon/xkbcommon.h>
#include <string.h>
#include "json_object.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "ipc-json.h"
#include "server.h"
#include "container.h"
#include "client.h"
#include "workspace.h"
#include "monitor.h"
#include "utils/coreUtils.h"
#include "stringop.h"

static json_object *ipc_json_create_rect(struct wlr_box *box) {
    json_object *rect = json_object_new_object();

    json_object_object_add(rect, "x", json_object_new_int(box->x));
    json_object_object_add(rect, "y", json_object_new_int(box->y));
    json_object_object_add(rect, "width", json_object_new_int(box->width));
    json_object_object_add(rect, "height", json_object_new_int(box->height));

    return rect;
}

static json_object *ipc_json_create_empty_rect(void) {
    struct wlr_box empty = {0, 0, 0, 0};

    return ipc_json_create_rect(&empty);
}

static json_object *ipc_json_create_node(int id, const char *name,
        bool focused, json_object *focus, struct wlr_box *box) {
    json_object *object = json_object_new_object();

    json_object_object_add(object, "id", json_object_new_int(id));
    json_object_object_add(object, "name",
            name ? json_object_new_string(name) : NULL);
    json_object_object_add(object, "rect",
            box ? ipc_json_create_rect(box) : ipc_json_create_empty_rect());
    json_object_object_add(object, "focused", json_object_new_boolean(focused));
    json_object_object_add(object, "focus", focus);

    // set default values to be compatible with i3
    json_object_object_add(object, "border",
            json_object_new_string("none"));
    json_object_object_add(object, "current_border_width",
            json_object_new_int(0));
    json_object_object_add(object, "layout", NULL);
    json_object_object_add(object, "orientation", NULL);
    json_object_object_add(object, "percent", NULL);
    json_object_object_add(object, "window_rect", ipc_json_create_empty_rect());
    json_object_object_add(object, "deco_rect", ipc_json_create_empty_rect());
    json_object_object_add(object, "geometry", ipc_json_create_empty_rect());
    json_object_object_add(object, "window", NULL);
    json_object_object_add(object, "urgent", json_object_new_boolean(false));

    json_object_object_add(object, "marks", json_object_new_array());
    json_object_object_add(object, "fullscreen_mode", json_object_new_int(0));
    json_object_object_add(object, "nodes", json_object_new_array());
    json_object_object_add(object, "floating_nodes", json_object_new_array());
    json_object_object_add(object, "sticky", json_object_new_boolean(false));

    return object;
}

struct focus_inactive_data {
    struct sway_node *node;
    json_object *object;
};

json_object *ipc_json_describe_tagset(struct tagset *tagset)
{
    json_object *array = json_object_new_array();
    for (int i = 0; i < server.mons.length; i++) {
        struct monitor *m = server.mons.items[i];
        struct tagset *tagset = monitor_get_active_tagset(m);

        for (int i = 0; i < tagset->workspaces.size; i++) {
            bool bit = bitset_test(&tagset->workspaces, i);
            bool is_selected = tagset->selected_ws_id == i;

            if (!bit && !is_selected)
                continue;

            struct workspace *ws = get_workspace(i);

            char *full_name = strdup(ws->name);

            const char *delimiter = ":";
            struct wlr_list content = split_string(full_name, delimiter);
            char *position = strdup("");
            char *name;
            if (content.length > 1) {
                position = realloc(position, strlen(content.items[0])+strlen(delimiter)+1);
                strcpy(position, content.items[0]);
                strcat(position, delimiter);
                int new_string_bytes = strlen(full_name)-strlen(position)+1;
                name = malloc(new_string_bytes);
                memmove(name, full_name + strlen(position), new_string_bytes);
            } else {
                name = content.items[0];
            }

            const char *prefix = "*";
            const char *postfix = "*";

            if (is_selected) {
                full_name = realloc(full_name, 
                        strlen(position) + strlen(prefix) + strlen(name) + strlen(postfix) + 1);
                strcpy(full_name, position);
                strcat(full_name, prefix);
                strcat(full_name, name);
                strcat(full_name, postfix);
            }

            free(name);
            free(position);

            json_object *tagset_object = ipc_json_describe_tag(full_name, is_selected, tagset->m);
            json_object_array_add(array, tagset_object);

            free(full_name);
        }
    }
    return array;
}

json_object *ipc_json_describe_tag(const char *name, bool is_selected, struct monitor *m)
{
    struct wlr_box box;
    box = m->geom;

    char *s = strdup(name);

    bool is_active_workspace = true;

    json_object *object = ipc_json_create_node(0, s, is_active_workspace, NULL, &box);

    json_object *children = json_object_new_array();
    json_object_object_add(object, "nodes", children);

    int num;
    if (isdigit(name[0])) {
        errno = 0;
        char *endptr = NULL;
        long long parsed_num = strtoll(name, &endptr, 10);
        if (errno != 0 || parsed_num > INT32_MAX || parsed_num < 0 || endptr == name) {
            num = -1;
        } else {
            num = (int) parsed_num;
        }
    } else {
        num = -1;
    }
    json_object_object_add(object, "num", json_object_new_int(num));

    json_object_object_add(object, "fullscreen_mode", json_object_new_int(0));
    json_object_object_add(object, "output", m ?
            json_object_new_string(m->wlr_output->name) : NULL);
    json_object_object_add(object, "type", json_object_new_string("workspace"));
    json_object_object_add(object, "urgent",
            json_object_new_boolean(false));

    free(s);
    return object;
}

json_object *ipc_json_describe_monitor(struct monitor *m)
{
    json_object *object = ipc_json_create_node(
            3, m->wlr_output->name, false, NULL, &m->geom);
    json_object *children = json_object_new_array();
    json_object_object_add(object, "nodes", children);
    return object;
}

json_object *ipc_json_describe_container(struct container *con)
{
    json_object *object = ipc_json_create_node(
            5, con ? con->client->title : NULL, true, NULL,
            con ? &con->geom : NULL);

    json_object_object_add(object, "type", json_object_new_string("con"));

    json_object *children = json_object_new_array();
    json_object_object_add(object, "nodes", children);

    return object;
}

json_object *ipc_json_describe_selected_container(struct monitor *m)
{
    json_object *root_object = ipc_json_create_node(1, "root", false, NULL, NULL);
    json_object *root_children = json_object_new_array();
    json_object_object_add(root_object, "nodes", root_children);

    json_object *monitor_object = ipc_json_describe_monitor(m);
    json_object_array_add(root_children, monitor_object);
    json_object *monitor_children;
    json_object_object_get_ex(monitor_object, "nodes", &monitor_children);

    struct workspace *ws = monitor_get_active_workspace(selected_monitor);
    json_object *workspace_object = ipc_json_describe_tag(ws->name, true, selected_monitor);
    json_object_array_add(monitor_children, workspace_object);
    json_object *workspace_children;
    json_object_object_get_ex(monitor_object, "nodes", &workspace_children);

    struct container *sel = get_focused_container(m);
    json_object *obj = ipc_json_describe_container(sel);
    json_object_array_add(workspace_children, obj);

    return root_object;
}
