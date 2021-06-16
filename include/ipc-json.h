#ifndef _SWAY_IPC_JSON_H
#define _SWAY_IPC_JSON_H
#include <json.h>
#include "client.h"
#include "tagset.h"
#include "workspace.h"

json_object *ipc_json_describe_tagset(struct tagset *tagset, bool focused);
json_object *ipc_json_describe_workspace(struct workspace *ws, bool focused);
json_object *ipc_json_describe_selected_container(struct monitor *m);

#endif
