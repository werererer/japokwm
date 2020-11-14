#ifndef _SWAY_IPC_JSON_H
#define _SWAY_IPC_JSON_H
#include <json.h>
#include "client.h"

json_object *ipc_json_describe_tagset(struct tagset *tagset);
json_object *ipc_json_describe_node(struct client *c);

#endif
