#include "rules/mon_rule.h"

#include <stdlib.h>
#include <string.h>

#include "utils/parseConfigUtils.h"
#include "server.h"

struct mon_rule *create_mon_rule(const char *output_name, int lua_func_ref)
{
    struct mon_rule *mon_rule = calloc(1, sizeof(*mon_rule));
    mon_rule->output_name = strdup(output_name);
    mon_rule->lua_func_ref = lua_func_ref;
    return mon_rule;
}

void destroy_mon_rule(struct mon_rule *mon_rule)
{
    free(mon_rule->output_name);
    free(mon_rule);
}

void apply_mon_rule(struct mon_rule *mon_rule, struct monitor *m)
{
    if (mon_rule->lua_func_ref <= 0)
        return;

    if (strstr(m->wlr_output->name, mon_rule->output_name)) {
        if (mon_rule->lua_func_ref <= 0)
            return;
        lua_rawgeti(L, LUA_REGISTRYINDEX, mon_rule->lua_func_ref);
        lua_call_safe(L, 0, 0, 0);
    }
}

void apply_mon_rules(GPtrArray *mon_rules, struct monitor *m)
{
    for (int i = 0; i < mon_rules->len; i++) {
        struct mon_rule *mon_rule = g_ptr_array_index(mon_rules, i);
        apply_mon_rule(mon_rule, m);
    }
}
