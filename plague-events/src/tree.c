#include "tree.h"

typedef struct {
    int parent;  /* PE_NO_PARENT si es raíz */
    int used;
} Node;

static Node g_nodes[PE_MAX_NODES];
static int  g_count = 0;

void pe_tree_init(void)
{
    for (int i = 0; i < PE_MAX_NODES; i++) {
        g_nodes[i].parent = PE_NO_PARENT;
        g_nodes[i].used   = 0;
    }
    g_count = 0;
}

int pe_tree_add(int parent_id)
{
    if (g_count >= PE_MAX_NODES) return -1;
    int id = g_count++;
    g_nodes[id].parent = parent_id;
    g_nodes[id].used   = 1;
    return id;
}

void pe_tree_set_parent(int node_id, int parent_id)
{
    if (node_id < 0 || node_id >= PE_MAX_NODES) return;
    g_nodes[node_id].parent = parent_id;
}

int pe_tree_parent(int node_id)
{
    if (node_id < 0 || node_id >= PE_MAX_NODES) return PE_NO_PARENT;
    if (!g_nodes[node_id].used) return PE_NO_PARENT;
    return g_nodes[node_id].parent;
}
