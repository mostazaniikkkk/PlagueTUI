#include "queue.h"
#include <string.h>

#define QUEUE_CAP 256

static PE_Event g_buf[QUEUE_CAP];
static int      g_head = 0;  /* próximo pop  */
static int      g_tail = 0;  /* próximo push */
static int      g_size = 0;

void pe_queue_init(void)
{
    g_head = g_tail = g_size = 0;
}

int pe_queue_push(PE_Event ev)
{
    if (g_size >= QUEUE_CAP) return 0;
    g_buf[g_tail] = ev;
    g_tail = (g_tail + 1) % QUEUE_CAP;
    g_size++;
    return 1;
}

int pe_queue_pop(PE_Event *out)
{
    if (g_size == 0) return 0;
    *out   = g_buf[g_head];
    g_head = (g_head + 1) % QUEUE_CAP;
    g_size--;
    return 1;
}

int pe_queue_size(void)  { return g_size; }

void pe_queue_clear(void) { g_head = g_tail = g_size = 0; }
