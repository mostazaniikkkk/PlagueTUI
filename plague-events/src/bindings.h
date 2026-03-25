#ifndef PE_BINDINGS_H
#define PE_BINDINGS_H

#include "../include/plague_events.h"

void pe_bindings_init(void);
int  pe_bind_key    (int node_id, int key, int modifiers);
void pe_unbind      (int binding_id);
void pe_unbind_all  (int node_id);
int  pe_dispatch_key(int target_node, int key, int modifiers);

#endif
