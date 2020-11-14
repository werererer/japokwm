#ifndef TAG_H
#define TAG_H

#include <stdlib.h>
/* A tag is simply a workspace that can be focused (like a normal workspace)
 * and can selected: which just means that all clients on the selected tags
 * will be combined to be shown on the focused tag
 * using this struct requires to use tagsetCreate and later tagsetDestroy
 * */
struct tag {
    char *name;
    struct layout layout;
};

void tagCreate(struct tag *tag, const char *name);
void tagDestroy(struct tag *tag);

#endif /* TAG_H */
