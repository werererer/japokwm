#ifndef _SWAY_IPC_JSON_H
#define _SWAY_IPC_JSON_H
#include <json.h>
#include "client.h"
#include "workspace.h"

json_object *ipc_json_describe_workspace(struct monitor *m, struct workspace *ws, bool focused);
json_object *ipc_json_describe_node(struct monitor *m, struct client *c);

#endif
