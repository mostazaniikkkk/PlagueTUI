#include "bindings.h"
#include "tree.h"

typedef struct {
    int node_id;
    int key;
    int modifiers;
    int active;
} Binding;

static Binding g_bindings[PE_MAX_BINDINGS];

void pe_bindings_init(void)
{
    for (int i = 0; i < PE_MAX_BINDINGS; i++)
        g_bindings[i].active = 0;
}

int pe_bind_key(int node_id, int key, int modifiers)
{
    for (int i = 0; i < PE_MAX_BINDINGS; i++) {
        if (!g_bindings[i].active) {
            g_bindings[i].node_id   = node_id;
            g_bindings[i].key       = key;
            g_bindings[i].modifiers = modifiers;
            g_bindings[i].active    = 1;
            return i + 1;  /* binding_id 1-based */
        }
    }
    return 0;  /* pool lleno */
}

void pe_unbind(int binding_id)
{
    if (binding_id < 1 || binding_id > PE_MAX_BINDINGS) return;
    g_bindings[binding_id - 1].active = 0;
}

void pe_unbind_all(int node_id)
{
    for (int i = 0; i < PE_MAX_BINDINGS; i++)
        if (g_bindings[i].active && g_bindings[i].node_id == node_id)
            g_bindings[i].active = 0;
}

/*
 * Sube desde target_node hacia la raíz buscando un binding que coincida.
 * Devuelve el binding_id del primero que encuentre, o 0 si ninguno.
 */
int pe_dispatch_key(int target_node, int key, int modifiers)
{
    int node = target_node;
    int depth = 0;  /* protección contra ciclos */

    while (node != PE_NO_PARENT && node >= 0 && depth < PE_MAX_NODES) {
        for (int i = 0; i < PE_MAX_BINDINGS; i++) {
            if (g_bindings[i].active
                    && g_bindings[i].node_id   == node
                    && g_bindings[i].key       == key
                    && g_bindings[i].modifiers == modifiers) {
                return i + 1;
            }
        }
        node = pe_tree_parent(node);
        depth++;
    }
    return 0;
}
