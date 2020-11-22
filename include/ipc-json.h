#ifndef _SWAY_IPC_JSON_H
#define _SWAY_IPC_JSON_H
#include <json.h>
#include "client.h"

json_object *ipc_json_describe_tag(struct tag *tag, bool focused, bool selected);
json_object *ipc_json_describe_node(struct client *c);

#endif
