#include "tree.h"
#include <string.h>

static PL_LayoutNode   g_nodes  [PL_MAX_NODES];
static PL_LayoutResult g_results[PL_MAX_NODES];
static int             g_count = 0;

void pl_tree_internal_init(void) {
    g_count = 0;
    memset(g_nodes,   0, sizeof(g_nodes));
    memset(g_results, 0, sizeof(g_results));
}

int pl_tree_internal_add(PL_LayoutNode node) {
    if (g_count >= PL_MAX_NODES) return -1;
    g_nodes[g_count] = node;
    return g_count++;
}

void pl_tree_internal_set_child(int parent_idx, int child_idx) {
    if (parent_idx < 0 || parent_idx >= g_count) return;
    if (child_idx  < 0 || child_idx  >= g_count) return;
    PL_LayoutNode* p = &g_nodes[parent_idx];
    if (p->child_count >= PL_MAX_CHILDREN) return;
    p->children[p->child_count++] = child_idx;
    g_nodes[child_idx].parent = parent_idx;
}

int pl_tree_internal_count(void) { return g_count; }

PL_LayoutNode pl_tree_internal_get(int index) {
    if (index < 0 || index >= g_count) {
        PL_LayoutNode empty; memset(&empty, 0, sizeof(empty));
        return empty;
    }
    return g_nodes[index];
}

PL_LayoutResult pl_tree_internal_result(int index) {
    if (index < 0 || index >= g_count) {
        PL_LayoutResult empty; memset(&empty, 0, sizeof(empty));
        return empty;
    }
    return g_results[index];
}

void pl_tree_internal_set_result(int index, PL_LayoutResult result) {
    if (index < 0 || index >= g_count) return;
    g_results[index] = result;
}
