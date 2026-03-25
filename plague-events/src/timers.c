#include "timers.h"
#include "queue.h"
#include <string.h>

typedef struct {
    int interval_ms;
    int accumulated_ms;
    int repeat;
    int active;
} Timer;

static Timer g_timers[PE_MAX_TIMERS];

void pe_timers_init(void)
{
    memset(g_timers, 0, sizeof(g_timers));
}

int pe_timer_create(int interval_ms, int repeat)
{
    if (interval_ms <= 0) return 0;
    for (int i = 0; i < PE_MAX_TIMERS; i++) {
        if (!g_timers[i].active) {
            g_timers[i].interval_ms    = interval_ms;
            g_timers[i].accumulated_ms = 0;
            g_timers[i].repeat         = repeat;
            g_timers[i].active         = 1;
            return i + 1;  /* timer_id 1-based */
        }
    }
    return 0;
}

void pe_timer_cancel(int timer_id)
{
    if (timer_id < 1 || timer_id > PE_MAX_TIMERS) return;
    g_timers[timer_id - 1].active = 0;
}

int pe_timer_active(int timer_id)
{
    if (timer_id < 1 || timer_id > PE_MAX_TIMERS) return 0;
    return g_timers[timer_id - 1].active;
}

void pe_timer_tick(int elapsed_ms)
{
    for (int i = 0; i < PE_MAX_TIMERS; i++) {
        if (!g_timers[i].active) continue;

        g_timers[i].accumulated_ms += elapsed_ms;

        while (g_timers[i].accumulated_ms >= g_timers[i].interval_ms) {
            PE_Event ev;
            memset(&ev, 0, sizeof(ev));
            ev.type             = PE_EVENT_TIMER;
            ev.data.timer.timer_id = i + 1;
            pe_queue_push(ev);

            if (g_timers[i].repeat) {
                g_timers[i].accumulated_ms -= g_timers[i].interval_ms;
            } else {
                g_timers[i].active         = 0;
                g_timers[i].accumulated_ms = 0;
                break;
            }
        }
    }
}
