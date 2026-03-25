#ifndef PA_WIDGETS_H
#define PA_WIDGETS_H

#include "../include/plague_app.h"

#define PA_TEXT_LEN    256
#define PA_NAME_LEN     64
#define PA_CLASS_LEN   128

typedef struct {
    char           type   [PA_NAME_LEN];
    char           id     [PA_NAME_LEN];
    char           classes[PA_CLASS_LEN];
    char           text   [PA_TEXT_LEN];
    PL_SizeValue   width;
    PL_SizeValue   height;
    PL_LayoutType  layout_type;
    PL_DockEdge    dock;
    TG_Spacing     padding;
    int            parent_wid;
    int            children[32];
    int            child_count;
    int            visible;
    int            used;
} PA_Widget;

extern PA_Widget g_widgets[PA_MAX_WIDGETS];
extern int       g_widget_count;

void pa_widgets_init(void);

#endif /* PA_WIDGETS_H */
