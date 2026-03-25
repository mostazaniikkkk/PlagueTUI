#include "widgets.h"
#include <string.h>

PA_Widget g_widgets[PA_MAX_WIDGETS];
int       g_widget_count = 0;

void pa_widgets_init(void)
{
    memset(g_widgets, 0, sizeof(g_widgets));
    g_widget_count = 0;
}
