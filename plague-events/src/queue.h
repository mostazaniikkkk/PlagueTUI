#ifndef PE_QUEUE_H
#define PE_QUEUE_H

#include "../include/plague_events.h"

void pe_queue_init (void);
int  pe_queue_push (PE_Event ev);
int  pe_queue_pop  (PE_Event *out);
int  pe_queue_size (void);
void pe_queue_clear(void);

#endif
