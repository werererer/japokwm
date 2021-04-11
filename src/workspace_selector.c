#include "workspace_selector.h"

void select_workspace(struct workspace_selector *ws_selector, int ws_id)
{
    printf("select_workspace\n");
    ws_selector->ws_id = ws_id;
    bitset_reset_all(&ws_selector->ids);
    bitset_set(&ws_selector->ids, ws_id);
}

void tag_workspace(struct workspace_selector *ws_selector, BitSet *bitset)
{
    bitset_xor(&ws_selector->ids, bitset);
}
