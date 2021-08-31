#ifndef RULE_H
#define RULE_H

#include "container.h"

struct rule {
    char *id;
    char *title;
    int lua_func_ref;
};

struct rule *create_rule(const char *id, const char *title, int lua_func_ref);
void destroy_rule(struct rule *rule);

void apply_rule(struct rule *rule, struct container *con);
void apply_rules(GPtrArray *rules, struct container *con);

#endif /* RULE_H */
