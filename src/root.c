#include "root.h"
#include "client.h"
#include "container.h"
#include "utils/coreUtils.h"
#include "tile/tileUtils.h"
#include "parseConfig.h"

struct root *create_root()
{
    struct root *root = calloc(1, sizeof(struct root));
    root->consider_layer_shell = true;
    memcpy(root->color, rootColor, sizeof(float)*4);
    return root;
}

void destroy_root(struct root *root)
{
    free(root);
}
