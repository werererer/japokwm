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
        bool focused, bool urgent, json_object *focus, struct wlr_box *box) {
    json_object *object = json_object_new_object();

    json_object_object_add(object, "id", json_object_new_int(id));
    json_object_object_add(object, "name",
            name ? json_object_new_string(name) : NULL);
    json_object_object_add(object, "rect", ipc_json_create_rect(box));
    json_object_object_add(object, "focused", json_object_new_boolean(focused));
    json_object_object_add(object, "focus", focus);

    // set default values to be compatible with i3
    json_object_object_add(object, "border",
            json_object_new_string("none"));
    json_object_object_add(object, "current_border_width",
            json_object_new_int(0));
    json_object_object_add(object, "layout",
            json_object_new_string("tabbed"));
    json_object_object_add(object, "orientation",
            json_object_new_string("tabbed"));
    json_object_object_add(object, "percent", NULL);
    json_object_object_add(object, "window_rect", ipc_json_create_empty_rect());
    json_object_object_add(object, "deco_rect", ipc_json_create_empty_rect());
    json_object_object_add(object, "geometry", ipc_json_create_empty_rect());
    json_object_object_add(object, "window", NULL);
    json_object_object_add(object, "urgent", json_object_new_boolean(urgent));

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

json_object *ipc_json_describe_workspace(struct monitor *m, struct workspace *ws, bool focused)
{
    struct wlr_box box;
    box = m->geom;

    char *s = strdup(ws->name);

    json_object *object = ipc_json_create_node(0, s, focused, false, NULL, &box);
    json_object_object_add(object, "num", json_object_new_int(0));
    json_object_object_add(object, "fullscreen_mode", json_object_new_int(0));
    json_object_object_add(object, "output", ws->m ?
            json_object_new_string(ws->m->wlr_output->name) : json_object_new_string(m->wlr_output->name));
    json_object_object_add(object, "type", json_object_new_string("workspace"));
    json_object_object_add(object, "urgent",
            json_object_new_boolean(false));

    free(s);
    return object;
}

json_object *ipc_json_describe_node(struct monitor *m, struct container *con) {
    bool focused = get_focused_container(m) == con;
    struct client *c = con->client;
    char *title = c->title;

    struct wlr_box *box;
    box = &m->geom;

    json_object *focus = json_object_new_array();

    json_object *object = ipc_json_create_node(
            0, title, focused, false, focus, box);

    return object;
}

json_object *ipc_json_describe_node_recursive(struct monitor *m, struct container *con) {
	json_object *object = ipc_json_describe_node(m, con);

    /* struct workspace *ws = get_workspace_in_monitor(m); */
	/* json_object *children = json_object_new_array(); */
    /* for (int i = 0; i < length_of_composed_list(&ws->focus_stack_visible_lists); i++) { */
    /*     struct container *con = get_in_composed_list(&ws->focus_stack_visible_lists, i); */
    /*     json_object_array_add(children, ipc_json_describe_node(m, con)); */
    /* } */
	/* json_object_object_add(object, "nodes", children); */

	return object;
}

