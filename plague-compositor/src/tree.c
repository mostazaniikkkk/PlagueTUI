#include "tree.h"
#include <string.h>

static PC_WidgetNode g_nodes[PC_MAX_NODES];
static int           g_count = 0;

void pc_tree_init(void) {
    g_count = 0;
}

int pc_tree_add(PC_WidgetNode node) {
    if (g_count >= PC_MAX_NODES) return -1;
    g_nodes[g_count] = node;
    return g_count++;
}

void pc_tree_set_child(int parent_idx, int child_idx) {
    if (parent_idx < 0 || parent_idx >= g_count) return;
    if (child_idx  < 0 || child_idx  >= g_count) return;
    PC_WidgetNode* p = &g_nodes[parent_idx];
    if (p->child_count >= PC_MAX_CHILDREN) return;
    p->children[p->child_count++] = child_idx;
    g_nodes[child_idx].parent = parent_idx;
}

int pc_tree_count(void) {
    return g_count;
}

PC_WidgetNode pc_tree_get(int index) {
    if (index < 0 || index >= g_count) {
        PC_WidgetNode empty;
        memset(&empty, 0, sizeof(empty));
        return empty;
    }
    return g_nodes[index];
}
