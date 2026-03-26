#include "focus.h"
#include "widgets.h"

static int g_focus_wid = -1;

void pa_focus_init(void)
{
    g_focus_wid = -1;
}

void pa_focus_set_wid(int wid)
{
    g_focus_wid = wid;
}

int pa_focus_get_wid(void)
{
    return g_focus_wid;
}

void pa_focus_advance(int direction)
{
    /* Construir lista de widgets focusables en orden de creación */
    int list[PA_MAX_WIDGETS];
    int count = 0;
    for (int i = 0; i < g_widget_count; i++) {
        if (g_widgets[i].used && g_widgets[i].focusable && !g_widgets[i].disabled)
            list[count++] = i;
    }
    if (count == 0) return;

    /* Localizar posición actual en la lista */
    int cur = -1;
    for (int i = 0; i < count; i++) {
        if (list[i] == g_focus_wid) { cur = i; break; }
    }

    /* Calcular siguiente */
    int next;
    if (cur < 0)
        next = (direction > 0) ? 0 : count - 1;
    else
        next = (cur + direction + count) % count;

    g_focus_wid = list[next];
}
