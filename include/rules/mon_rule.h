#ifndef MON_RULE_H
#define MON_RULE_H

#include "monitor.h"

struct mon_rule {
    char *output_name;
    int lua_func_ref;
};

struct mon_rule *create_mon_rule(const char *output_name, int lua_func_ref);
void destroy_mon_rule(struct mon_rule *mon_rule);

void apply_mon_rule(struct mon_rule *mon_rule, struct monitor *m);
void apply_mon_rules(GPtrArray *mon_rules, struct monitor *m);

#endif /* MON_RULE_H */
