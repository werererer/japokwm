#ifndef _SWAY_IPC_JSON_H
#define _SWAY_IPC_JSON_H
#include <json.h>
#include "client.h"
#include "tagset.h"
#include "workspace.h"

json_object *ipc_json_describe_tagset(struct tagset *tagset);
json_object *ipc_json_describe_tag(const char *name, bool is_selected, struct monitor *m);
json_object *ipc_json_describe_selected_container(struct monitor *m);

#endif
