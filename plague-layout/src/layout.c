/* Punto de entrada público — delega a tree.c, compute.c y helpers.c */

#include "../include/plague_layout.h"
#include "tree.h"
#include "compute.h"

void pl_tree_init(void) {
    pl_tree_internal_init();
}

int pl_tree_add(PL_LayoutNode node) {
    return pl_tree_internal_add(node);
}

void pl_tree_set_child(int parent_idx, int child_idx) {
    pl_tree_internal_set_child(parent_idx, child_idx);
}

int pl_tree_count(void) {
    return pl_tree_internal_count();
}

PL_LayoutNode pl_tree_get(int index) {
    return pl_tree_internal_get(index);
}

void pl_tree_compute(int root_idx, TG_Region container) {
    pl_compute(root_idx, container);
}

PL_LayoutResult pl_tree_result(int index) {
    return pl_tree_internal_result(index);
}
