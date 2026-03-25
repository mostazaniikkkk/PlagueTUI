#ifndef PE_TIMERS_H
#define PE_TIMERS_H

#include "../include/plague_events.h"

void pe_timers_init (void);
int  pe_timer_create(int interval_ms, int repeat);
void pe_timer_cancel(int timer_id);
int  pe_timer_active(int timer_id);
void pe_timer_tick  (int elapsed_ms);

#endif
