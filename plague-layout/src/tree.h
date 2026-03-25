#ifndef PL_TREE_H
#define PL_TREE_H

#include "../include/plague_layout.h"

void           pl_tree_internal_init   (void);
int            pl_tree_internal_add    (PL_LayoutNode node);
void           pl_tree_internal_set_child(int parent_idx, int child_idx);
int            pl_tree_internal_count  (void);
PL_LayoutNode  pl_tree_internal_get    (int index);
PL_LayoutResult pl_tree_internal_result(int index);
void           pl_tree_internal_set_result(int index, PL_LayoutResult result);

#endif
