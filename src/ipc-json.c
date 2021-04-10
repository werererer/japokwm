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
    json_object_object_add(object, "layout", NULL);
    json_object_object_add(object, "orientation", NULL);
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
    /* struct client *c = con->client; */
    const char *title = "lol";

    struct wlr_box *box;
    box = &m->geom;

    json_object *focus = json_object_new_array();

    json_object *object = ipc_json_create_node(
            0, title, focused, false, focus, box);

    return object;
}

json_object *ipc_json_describe_node_recursive(struct monitor *m, struct container *con) {
    /* json_object *object = ipc_json_describe_node(m, con); */
    struct wlr_box box = {
        .x = 0,
        .y = 0,
        .width = 600,
        .height = 600,
    };

    json_object *focus = json_object_new_array();
    json_object_array_add(focus, json_object_new_int(2));
    json_object *obj1 = ipc_json_create_node(1, "root", false, false, focus, &box);
    json_object *children = json_object_new_array();
    json_object_object_add(obj1, "nodes", children);

    json_object *focus_tmp = json_object_new_array();
    json_object_array_add(focus, json_object_new_int(3));
    json_object *obj_tmp= ipc_json_create_node(
            2, "test", true, false, focus_tmp, &box);
    json_object_object_add(obj_tmp, "type", json_object_new_string("con"));
    json_object_object_add(obj_tmp, "visible", json_object_new_boolean(true));
    json_object_object_add(obj_tmp, "shell", json_object_new_string("xdg_shell"));
    json_object_object_add(obj_tmp, "app_id", json_object_new_string("test"));
    json_object_array_add(children, obj_tmp);
    json_object *children_tmp = json_object_new_array();
    json_object_object_add(obj_tmp, "nodes", children_tmp);

    json_object *focus2 = json_object_new_array();
    json_object_array_add(focus, json_object_new_int(3));
    json_object *obj2 = ipc_json_create_node(
            2, "test", true, false, focus2, &box);
    json_object_object_add(obj2, "type", json_object_new_string("con"));
    json_object_object_add(obj2, "visible", json_object_new_boolean(true));
    json_object_object_add(obj2, "shell", json_object_new_string("xdg_shell"));
    json_object_object_add(obj2, "app_id", json_object_new_string("test"));
    json_object_array_add(children, obj2);
    json_object *children2 = json_object_new_array();
    json_object_object_add(obj2, "nodes", children2);

    json_object *focus3 = json_object_new_array();
    json_object_array_add(focus, json_object_new_int(4));
    json_object *obj3 = ipc_json_create_node(
            3, "eDP-1", false, false, focus3, &box);
    json_object_array_add(children2, obj3);
    json_object *children3 = json_object_new_array();
    json_object_object_add(obj3, "nodes", children3);

    json_object *focus4 = json_object_new_array();
    json_object_array_add(focus, json_object_new_int(5));
    json_object *obj4 = ipc_json_create_node(
            4, "1", false, false, focus4, &box);
    json_object_object_add(obj4, "layout", json_object_new_string("none"));
    json_object_array_add(children3, obj4);
    json_object *children4 = json_object_new_array();
    json_object_object_add(obj4, "nodes", children4);

    json_object *obj5 = ipc_json_create_node(
            5, "test", true, false, NULL, &box);
    json_object_array_add(children4, obj5);
    json_object *children5 = json_object_new_array();
    json_object_object_add(obj5, "nodes", children5);

	/* switch (node->type) { */
	/* case N_ROOT: */
	/* 	json_object_array_add(children, */
	/* 			NULL); */
	/* 	for (i = 0; i < mons->root->outputs->length; ++i) { */
	/* 		struct sway_output *output = root->outputs->items[i]; */
	/* 		json_object_array_add(children, */
	/* 				ipc_json_describe_node_recursive(&output->node)); */
	/* 	} */
	/* 	break; */
	/* case N_OUTPUT: */
	/* 	for (i = 0; i < node->sway_output->workspaces->length; ++i) { */
	/* 		struct sway_workspace *ws = node->sway_output->workspaces->items[i]; */
	/* 		json_object_array_add(children, */
	/* 				ipc_json_describe_node_recursive(&ws->node)); */
	/* 	} */
	/* 	break; */
	/* case N_WORKSPACE: */
	/* 	for (i = 0; i < node->sway_workspace->tiling->length; ++i) { */
	/* 		struct sway_container *con = node->sway_workspace->tiling->items[i]; */
	/* 				ipc_json_describe_node_recursive(&con->node); */
	/* 	} */
	/* 	break; */
	/* case N_CONTAINER: */
	/* 	if (node->sway_container->children) { */
	/* 		for (i = 0; i < node->sway_container->children->length; ++i) { */
	/* 			struct sway_container *child = */
	/* 				node->sway_container->children->items[i]; */
	/* 			json_object_array_add(children, */
	/* 					ipc_json_describe_node_recursive(&child->node)); */
	/* 		} */
	/* 	} */
	/* 	break; */
	/* } */

    /* json_object_object_add(obj, "focused", json_object_new_boolean(true)); */

    /* json_object *children = json_object_new_array(); */
    /* json_object_array_add(children, ipc_json_describe_node(m, con)); */
    /* json_object_object_add(obj, "nodes", children); */

    /* struct workspace *ws = get_workspace_in_monitor(m); */
    /* json_object *children = json_object_new_array(); */
    /* for (int i = 0; i < length_of_composed_list(&ws->focus_stack_visible_lists); i++) { */
    /*     struct container *con = get_in_composed_list(&ws->focus_stack_visible_lists, i); */
        /* json_object_array_add(children, ipc_json_describe_node(m, con)); */
    /* } */
    /* json_object_object_add(object, "nodes", children); */

    return obj1;
}

