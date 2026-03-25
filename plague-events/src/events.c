#include "../include/plague_events.h"
#include "queue.h"
#include "tree.h"
#include "bindings.h"
#include "timers.h"

/* ---------------------------------------------------------------------------
 * Estado global de foco
 * --------------------------------------------------------------------------- */

static int g_focus = -1;

/* ---------------------------------------------------------------------------
 * Re-exportación de la API pública
 * (las implementaciones ya están en los archivos .c de cada módulo)
 * --------------------------------------------------------------------------- */

/* Cola */
/* pe_queue_* ya están definidas en queue.c */

/* Árbol */
/* pe_tree_* ya están definidas en tree.c */

/* Bindings */
/* pe_bind_key, pe_unbind, pe_unbind_all, pe_dispatch_key en bindings.c */

/* Foco */
void pe_focus_set(int node_id) { g_focus = node_id; }
int  pe_focus_get(void)        { return g_focus; }

/* Reset global */
void pe_reset_all(void)
{
    pe_queue_init();
    pe_tree_init();
    pe_bindings_init();
    pe_timers_init();
    g_focus = -1;
}

/* Timers */
/* pe_timer_* ya están definidas en timers.c */
