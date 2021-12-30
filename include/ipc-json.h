#ifndef _SWAY_IPC_JSON_H
#define _SWAY_IPC_JSON_H
#include <json.h>
#include "client.h"
#include "tagset.h"
#include "tag.h"

json_object *ipc_json_describe_tagsets();
json_object *ipc_json_describe_tag(const char *name, bool is_selected, struct output *m);
json_object *ipc_json_describe_selected_container(struct output *m);
json_object *ipc_json_describe_bar_config();

#endif
