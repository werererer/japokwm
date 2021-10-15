#include "rules/rule.h"

#include <glib.h>
#include <assert.h>

#include "client.h"
#include "server.h"
#include "workspace.h"
#include "utils/parseConfigUtils.h"
#include "lib/lib_container.h"

struct rule *create_rule(const char *id, const char *title, int lua_func_ref)
{
    struct rule *rule = calloc(1, sizeof(*rule));
    rule->id = strdup(id);
    rule->title = strdup(title);
    rule->lua_func_ref = lua_func_ref;
    return rule;
}

void destroy_rule(struct rule *rule)
{
    free(rule->id);
    free(rule->title);
    free(rule);
}

void apply_rule(struct rule *rule, struct container *con)
{
    if (rule->lua_func_ref <= 0)
        return;

    bool same_id = g_strcmp0(rule->id, con->client->app_id) == 0;
    bool id_empty = g_strcmp0(rule->id, "") == 0;
    bool same_title = g_strcmp0(rule->title, con->client->title) == 0;
    bool title_empty = g_strcmp0(rule->title, "") == 0;
    if ((same_id || id_empty) && (same_title || title_empty)) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, rule->lua_func_ref);
        create_lua_container(L, con);
        lua_call_safe(L, 1, 0, 0);
    }
}

void apply_rules(GPtrArray *rules, struct container *con)
{
    for (int i = 0; i < rules->len; i++) {
        struct rule *rule = g_ptr_array_index(rules, i);
        apply_rule(rule, con);
    }
}

