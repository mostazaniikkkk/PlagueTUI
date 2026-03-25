#ifndef PE_TREE_H
#define PE_TREE_H

#include "../include/plague_events.h"

void pe_tree_init      (void);
int  pe_tree_add       (int parent_id);
void pe_tree_set_parent(int node_id, int parent_id);
int  pe_tree_parent    (int node_id);

#endif
