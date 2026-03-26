#ifndef PA_WIDGETS_H
#define PA_WIDGETS_H

#include "../include/plague_app.h"
#include <stdint.h>

#define PA_TEXT_LEN   4096
#define PA_NAME_LEN     64
#define PA_CLASS_LEN   128

/* Bitmask de estado — valores idénticos a PC_STATE_* para pasar directo a CSS */
#define PA_STATE_HOVER    0x01
#define PA_STATE_FOCUSED  0x02
#define PA_STATE_PRESSED  0x04
#define PA_STATE_DISABLED 0x08

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
    /* Infraestructura de widgets interactivos */
    uint8_t        focusable;  /* participa en Tab order */
    uint8_t        disabled;   /* no responde a input */
    uint8_t        overlay;    /* se dibuja encima de todo */
    uint8_t        state;      /* bitmask PA_STATE_* */
    int            scroll_x;   /* offset de scroll horizontal */
    int            scroll_y;   /* offset de scroll vertical */
} PA_Widget;

extern PA_Widget g_widgets[PA_MAX_WIDGETS];
extern int       g_widget_count;

void pa_widgets_init(void);

#endif /* PA_WIDGETS_H */
