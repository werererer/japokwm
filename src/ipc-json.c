#include <errno.h>
#include <json.h>
#include <stdio.h>
#include <ctype.h>
#include <wlr/backend/libinput.h>
#include <wlr/util/box.h>
#include <wlr/types/wlr_output.h>
#include <xkbcommon/xkbcommon.h>
#include <string.h>
#include "json_object.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "ipc-json.h"
#include "server.h"
#include "container.h"
#include "client.h"
#include "tag.h"
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

// note: full_name must use malloced memory
static void add_infix(char **full_name, const char *prefix, const char *postfix)
{
    const char *delimiter = ":";

    GPtrArray *content = split_string(*full_name, delimiter);
    char *position = strdup("");
    char *name;
    char *content0 = g_ptr_array_index(content, 0);
    if (content->len > 1) {
        position = realloc(position, strlen(content0)+strlen(delimiter)+1);
        strcpy(position, content0);
        strcat(position, delimiter);
        int name_byte_len = strlen(*full_name)-strlen(position)+1;
        name = malloc(name_byte_len);
        memmove(name, *full_name + strlen(position), name_byte_len);
    } else {
        name = content0;
    }

    int full_name_string_length = strlen(position) + strlen(prefix)
        + strlen(name) + strlen(postfix) + 1;
    *full_name = realloc(*full_name, full_name_string_length);
    strcpy(*full_name, position);
    strcat(*full_name, prefix);
    strcat(*full_name, name);
    strcat(*full_name, postfix);

    free(name);
    free(position);
}

static bool sel_con_has_bitset_here(struct container *con, struct tag *tag)
{
    if (!con)
        return false;
    bool min_count = bitset_count(con->client->sticky_tags) > 1;
    bool visible = bitset_test(con->client->sticky_tags, tag->id);
    return min_count && visible;
}

json_object *ipc_json_describe_tagsets()
{
    json_object *array = json_object_new_array();

    struct monitor *sel_m = server_get_selected_monitor();
    struct tag *sel_tag = monitor_get_active_tag(sel_m);
    struct container *sel_con = tag_get_focused_container(sel_tag);
    for (GList *iterator = server_get_tags(); iterator; iterator = iterator->next) {
        struct tag *tag = iterator->data;
        struct monitor *m = tag_get_monitor(tag);

        if (!m)
            continue;
        if (!tag_is_visible(tag, m))
            continue;

        char *full_name = strdup(tag->name);

        if (sel_con_has_bitset_here(sel_con, tag)) {
            add_infix(&full_name, "", "+");
        }
        if (is_tag_the_selected_one(tag)) {
            add_infix(&full_name, "*", "*");
        }

        bool is_active = tag_is_active(tag);
        json_object *tagset_object = ipc_json_describe_tag(full_name, is_active, m);
        json_object_array_add(array, tagset_object);

        // for the second monitor
        if (is_tag_extern(tag)) {
            struct monitor *selected_monitor = tag_get_selected_monitor(tag);
            char *hidden_name = strdup(tag->name);
            add_infix(&hidden_name, "(", ")");
            json_object *tagset_object = ipc_json_describe_tag(hidden_name, false, selected_monitor);
            json_object_array_add(array, tagset_object);
            free(hidden_name);
        }
        free(full_name);
    }
    return array;
}

json_object *ipc_json_describe_bar_config() {
    json_object *json = json_object_new_object();
    // json_object_object_add(json, "mode", json_object_new_string("show"));
    json_object_object_add(json, "hidden_state",
            json_object_new_string("show"));
    json_object_object_add(json, "position",
            json_object_new_string("bottom"));
    json_object_object_add(json, "status_command", NULL);
    // json_object_object_add(json, "font",
    //         json_object_new_string((bar->font) ? bar->font : config->font));

    // json_object *gaps = json_object_new_object();
    // json_object_object_add(gaps, "top",
    //         json_object_new_int(bar->gaps.top));
    // json_object_object_add(gaps, "right",
    //         json_object_new_int(bar->gaps.right));
    // json_object_object_add(gaps, "bottom",
    //         json_object_new_int(bar->gaps.bottom));
    // json_object_object_add(gaps, "left",
    //         json_object_new_int(bar->gaps.left));
    // json_object_object_add(json, "gaps", gaps);

    // if (bar->separator_symbol) {
    //     json_object_object_add(json, "separator_symbol",
    //             json_object_new_string(bar->separator_symbol));
    // }
    json_object_object_add(json, "bar_height",
            json_object_new_int(0));
    json_object_object_add(json, "status_padding",
            json_object_new_int(0));
    json_object_object_add(json, "status_edge_padding",
            json_object_new_int(0));
    json_object_object_add(json, "wrap_scroll",
            json_object_new_boolean(false));
    json_object_object_add(json, "tag_buttons",
            json_object_new_boolean(true));
    json_object_object_add(json, "strip_tag_numbers",
            json_object_new_boolean(false));
    json_object_object_add(json, "strip_tag_name",
            json_object_new_boolean(false));
    json_object_object_add(json, "tag_min_width",
            json_object_new_int(0));
    json_object_object_add(json, "binding_mode_indicator",
            json_object_new_boolean(false));
    json_object_object_add(json, "verbose",
            json_object_new_boolean(false));
    json_object_object_add(json, "pango_markup",
            json_object_new_boolean(false));

    // json_object *colors = json_object_new_object();
    // json_object_object_add(colors, "background",
    //         json_object_new_string(bar->colors.background));
    // json_object_object_add(colors, "statusline",
    //         json_object_new_string(bar->colors.statusline));
    // json_object_object_add(colors, "separator",
    //         json_object_new_string(bar->colors.separator));

//     json_object_object_add(colors, "focused_background",
//             json_object_new_string(bar->colors.focused_background));

    // if (bar->colors.focused_statusline) {
    //     json_object_object_add(colors, "focused_statusline",
    //             json_object_new_string(bar->colors.focused_statusline));
    // } else {
    //     json_object_object_add(colors, "focused_statusline",
    //             json_object_new_string(bar->colors.statusline));
    // }
    //
    // if (bar->colors.focused_separator) {
    //     json_object_object_add(colors, "focused_separator",
    //             json_object_new_string(bar->colors.focused_separator));
    // } else {
    //     json_object_object_add(colors, "focused_separator",
    //             json_object_new_string(bar->colors.separator));
    // }
    //
    // json_object_object_add(colors, "focused_tag_border",
    //         json_object_new_string(bar->colors.focused_tag_border));
    // json_object_object_add(colors, "focused_tag_bg",
    //         json_object_new_string(bar->colors.focused_tag_bg));
    // json_object_object_add(colors, "focused_tag_text",
    //         json_object_new_string(bar->colors.focused_tag_text));
    //
    // json_object_object_add(colors, "inactive_tag_border",
    //         json_object_new_string(bar->colors.inactive_tag_border));
    // json_object_object_add(colors, "inactive_tag_bg",
    //         json_object_new_string(bar->colors.inactive_tag_bg));
    // json_object_object_add(colors, "inactive_tag_text",
    //         json_object_new_string(bar->colors.inactive_tag_text));
    //
    // json_object_object_add(colors, "active_tag_border",
    //         json_object_new_string(bar->colors.active_tag_border));
    // json_object_object_add(colors, "active_tag_bg",
    //         json_object_new_string(bar->colors.active_tag_bg));
    // json_object_object_add(colors, "active_tag_text",
    //         json_object_new_string(bar->colors.active_tag_text));
    //
    // json_object_object_add(colors, "urgent_tag_border",
    //         json_object_new_string(bar->colors.urgent_tag_border));
    // json_object_object_add(colors, "urgent_tag_bg",
    //         json_object_new_string(bar->colors.urgent_tag_bg));
    // json_object_object_add(colors, "urgent_tag_text",
    //         json_object_new_string(bar->colors.urgent_tag_text));
    //
    // if (bar->colors.binding_mode_border) {
    //     json_object_object_add(colors, "binding_mode_border",
    //             json_object_new_string(bar->colors.binding_mode_border));
    // } else {
    //     json_object_object_add(colors, "binding_mode_border",
    //             json_object_new_string(bar->colors.urgent_tag_border));
    // }
    //
    // if (bar->colors.binding_mode_bg) {
    //     json_object_object_add(colors, "binding_mode_bg",
    //             json_object_new_string(bar->colors.binding_mode_bg));
    // } else {
    //     json_object_object_add(colors, "binding_mode_bg",
    //             json_object_new_string(bar->colors.urgent_tag_bg));
    // }
    //
    // if (bar->colors.binding_mode_text) {
    //     json_object_object_add(colors, "binding_mode_text",
    //             json_object_new_string(bar->colors.binding_mode_text));
    // } else {
    //     json_object_object_add(colors, "binding_mode_text",
    //             json_object_new_string(bar->colors.urgent_tag_text));
    // }
    //
    // json_object_object_add(json, "colors", colors);
    //
    // if (bar->bindings->length > 0) {
    //     json_object *bindings = json_object_new_array();
    //     for (int i = 0; i < bar->bindings->length; ++i) {
    //         struct bar_binding *binding = bar->bindings->items[i];
    //         json_object *bind = json_object_new_object();
    //         json_object_object_add(bind, "input_code",
    //                 json_object_new_int(event_to_x11_button(binding->button)));
    //         json_object_object_add(bind, "event_code",
    //                 json_object_new_int(binding->button));
    //         json_object_object_add(bind, "command",
    //                 json_object_new_string(binding->command));
    //         json_object_object_add(bind, "release",
    //                 json_object_new_boolean(binding->release));
    //         json_object_array_add(bindings, bind);
    //     }
    //     json_object_object_add(json, "bindings", bindings);
    // }
    //
    // // Add outputs if defined
    // if (bar->outputs && bar->outputs->length > 0) {
    //     json_object *outputs = json_object_new_array();
    //     for (int i = 0; i < bar->outputs->length; ++i) {
    //         const char *name = bar->outputs->items[i];
    //         json_object_array_add(outputs, json_object_new_string(name));
    //     }
    //     json_object_object_add(json, "outputs", outputs);
    // }
    return json;
}


json_object *ipc_json_describe_tag(const char *name, bool is_active_tag, struct monitor *m)
{
    struct wlr_box box;
    box = m->geom;

    char *s = strdup(name);

    json_object *object = ipc_json_create_node(0, s, is_active_tag, NULL, &box);

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
    json_object_object_add(object, "type", json_object_new_string("tag"));
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
    if (!con)
        return NULL;
    struct wlr_box geom = container_get_current_geom(con);
    json_object *object = ipc_json_create_node(
            5, con ? con->client->title : NULL, true, NULL,
            con ? &geom : NULL);

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

    struct monitor *sel_m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(sel_m);
    json_object *tag_object = ipc_json_describe_tag(tag->name, true, sel_m);
    json_object_array_add(monitor_children, tag_object);
    json_object *tag_children;
    json_object_object_get_ex(monitor_object, "nodes", &tag_children);

    struct container *sel = monitor_get_focused_container(m);
    json_object *obj = ipc_json_describe_container(sel);
    json_object_array_add(tag_children, obj);

    return root_object;
}
