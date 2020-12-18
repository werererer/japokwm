#ifndef _SWAY_IPC_JSON_H
#define _SWAY_IPC_JSON_H
#include <json.h>
#include "client.h"

json_object *ipc_json_describe_tag(struct monitor *m, struct tag *tag, bool focused, bool selected);
json_object *ipc_json_describe_node(struct monitor *m, struct client *c);

#endif
