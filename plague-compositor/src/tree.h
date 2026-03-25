#ifndef PC_TREE_H
#define PC_TREE_H

#include "widget_node.h"

#define PC_MAX_NODES 256

void          pc_tree_init    (void);
int           pc_tree_add     (PC_WidgetNode node);
void          pc_tree_set_child(int parent_idx, int child_idx);
int           pc_tree_count   (void);
PC_WidgetNode pc_tree_get     (int index);

#endif
