#include <json.h>
#include <stdio.h>
#include <ctype.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_output.h>
#include <xkbcommon/xkbcommon.h>
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "ipc-json.h"
#include "server.h"
#include "client.h"

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

static json_object *ipc_json_create_node(int id, char *name,
		bool focused, json_object *focus, struct wlr_box *box) {
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
	json_object_object_add(object, "urgent", json_object_new_boolean(false));
	json_object_object_add(object, "marks", json_object_new_array());
	json_object_object_add(object, "fullscreen_mode", json_object_new_int(0));
	json_object_object_add(object, "nodes", json_object_new_array());
	json_object_object_add(object, "floating_nodes", json_object_new_array());
	json_object_object_add(object, "sticky", json_object_new_boolean(false));

	return object;
}

static void ipc_json_describe_tag(struct tag *tag,
		json_object *object) {
	int num;
	char *name = tag->name;
	
	if (isdigit(name[0])) {
		/* errno = 0; */
		char *endptr = NULL;
		long long parsed_num = strtoll(name, &endptr, 10);
		if (parsed_num > INT32_MAX || parsed_num < 0 || endptr == name) {
			num = -1;
		} else {
			num = (int) parsed_num;
		}
	} else {
		num = -1;
	}
	json_object_object_add(object, "num", json_object_new_int(num));
	json_object_object_add(object, "fullscreen_mode", json_object_new_int(1));
	json_object_object_add(object, "output", selMon->output ?
			json_object_new_string(selMon->output->name) : NULL);
	json_object_object_add(object, "type", json_object_new_string("workspace"));
	json_object_object_add(object, "urgent",
			json_object_new_boolean(false)); 
	json_object_object_add(object, "representation", NULL);

	json_object_object_add(object, "layout",
			json_object_new_string("tabbed"));
	json_object_object_add(object, "orientation",
			json_object_new_string("normal"));

	// Floating
	json_object *floating_array = json_object_new_array();
	/* for (int i = 0; i < workspace->floating->length; ++i) { */
	/* 	struct sway_container *floater = workspace->floating->items[i]; */
	/* 	json_object_array_add(floating_array, */
	/* 			ipc_json_describe_node(&floater->node)); */
	/* } */
	json_object_object_add(object, "floating_nodes", floating_array);
}

struct focus_inactive_data {
	struct sway_node *node;
	json_object *object;
};

json_object *ipc_json_describe_tagset(struct tagset *tagset) {
    unsigned int focusedTag = positionToFlag(tagset->focusedTag);
    bool focused = focusedTag & tagset->selTags[0];
    char *title = tagsetFocusedTag(tagset)->name;

    struct wlr_box box;
    box = selMon->m;

    json_object *object = ipc_json_create_node(3, title, focused, NULL, &box);

    /* if (node->type == N_WORKSPACE) */
    ipc_json_describe_tag(tagsetFocusedTag(tagset), object);
    return object;
}

json_object *ipc_json_describe_node(struct client *c) {
    bool focused = selClient() == c;
    char *title = c->title;

    struct wlr_box box;
    box = selMon->m;

    json_object *focus = json_object_new_array();

    json_object *object = ipc_json_create_node(
            c->id, title, focused, focus, &box);

    return object;
}
