#include "render.h"
#include "widgets.h"

#include "plague_layout.h"
#include "plague_terminal.h"
#include "plague_css.h"
#include "color.h"
#include "text_style.h"

#include <string.h>

/* Último árbol de resultados de layout */
static TG_Region g_regions[PA_MAX_WIDGETS];

TG_Region pa_last_region(int wid)
{
    if (wid < 0 || wid >= PA_MAX_WIDGETS) {
        TG_Region z = {0, 0, 0, 0};
        return z;
    }
    return g_regions[wid];
}

/* ---------------------------------------------------------------------------
 * Conversión de color CSS (uint8 0-255) a PG_Color (float 0-1)
 * --------------------------------------------------------------------------- */

static PG_Color css_to_pg(PC_Color c)
{
    PG_Color g;
    g.r = c.r / 255.0f;
    g.g = c.g / 255.0f;
    g.b = c.b / 255.0f;
    g.a = c.a / 255.0f;
    return g;
}

/* ---------------------------------------------------------------------------
 * Construcción del nodo de layout para un widget
 * --------------------------------------------------------------------------- */

static PL_LayoutNode build_layout_node(int wid)
{
    PA_Widget *w = &g_widgets[wid];
    PL_LayoutNode n;
    memset(&n, 0, sizeof(n));

    n.layout    = w->layout_type;
    n.width     = w->width;
    n.height    = w->height;
    n.padding   = w->padding;
    n.dock      = w->dock;
    n.overflow  = PL_OVERFLOW_HIDDEN;
    n.visible   = w->visible;
    return n;
}

/* ---------------------------------------------------------------------------
 * Dibujo de un widget individual
 * --------------------------------------------------------------------------- */

static void draw_widget(int i, int css_handle)
{
    PA_Widget *w = &g_widgets[i];
    if (!w->used || !w->visible) return;

    TG_Region reg = g_regions[i];
    if (reg.width <= 0 || reg.height <= 0) return;

    /* Obtener estilo CSS pasando el estado actual del widget */
    PC_ComputedStyle style = {0};
    if (css_handle > 0)
        style = pc_compute_style(css_handle, w->type, w->id, w->classes,
                                 (int)w->state);

    /* --- Fondo --- */
    PG_Color bg = { 0.08f, 0.08f, 0.12f, 1.0f };
    if (style.set_flags & PC_HAS_BACKGROUND)
        bg = css_to_pg(style.background);
    pt_fill_rect(reg, bg);

    /* --- Borde --- */
    if ((style.set_flags & PC_HAS_BORDER)
            && style.border_style != PC_BORDER_NONE) {
        PG_Color bc = { 0.3f, 0.3f, 0.5f, 1.0f };
        if (style.border_color.a > 0)
            bc = css_to_pg(style.border_color);
        pt_stroke_rect(reg, bc, 1.0f);
    }

    /* --- Texto --- */
    if (w->text[0] != '\0') {
        PG_Color fg = { 1.0f, 1.0f, 1.0f, 1.0f };
        if (style.set_flags & PC_HAS_COLOR)
            fg = css_to_pg(style.color);

        PG_TextStyle ts;
        memset(&ts, 0, sizeof(ts));
        ts.color  = fg;
        ts.bold   = (style.set_flags & PC_HAS_BOLD)   ? style.bold   : 0;
        ts.italic = (style.set_flags & PC_HAS_ITALIC) ? style.italic : 0;

        /* Posición del texto con scroll aplicado */
        TG_Region cr = pl_tree_result(i).content_region;
        int text_x = cr.x - w->scroll_x;
        int has_nl = (strchr(w->text, '\n') != NULL);
        int text_y = has_nl ? (cr.y - w->scroll_y)
                            : (cr.y + cr.height / 2);
        if (text_y < cr.y) text_y = cr.y;

        TG_Offset pos = { text_x, text_y };
        if (has_nl) {
            pt_clip_push(cr);
            pt_draw_text(pos, w->text, ts);
            pt_clip_pop();
        } else {
            pt_draw_text(pos, w->text, ts);
        }
    }
}

/* ---------------------------------------------------------------------------
 * Render principal
 * --------------------------------------------------------------------------- */

void pa_do_render(int cols, int rows, int css_handle)
{
    if (g_widget_count == 0) return;

    /* 1. Reconstruir árbol de layout */
    pl_tree_init();
    for (int i = 0; i < g_widget_count; i++) {
        if (!g_widgets[i].used) continue;
        pl_tree_add(build_layout_node(i));
    }
    for (int i = 0; i < g_widget_count; i++) {
        if (!g_widgets[i].used) continue;
        PA_Widget *w = &g_widgets[i];
        for (int c = 0; c < w->child_count; c++)
            pl_tree_set_child(i, w->children[c]);
    }

    /* 2. Calcular posiciones */
    TG_Region screen = { 0, 0, cols, rows };
    pl_tree_compute(0, screen);

    /* Guardar regiones */
    for (int i = 0; i < g_widget_count; i++)
        g_regions[i] = pl_tree_result(i).region;

    /* 3. Limpiar buffer */
    pt_clear();

    /* 4. Pasada 1: widgets normales (no overlay) */
    for (int i = 0; i < g_widget_count; i++) {
        if (g_widgets[i].overlay) continue;
        draw_widget(i, css_handle);
    }

    /* 5. Pasada 2: overlays (encima de todo) */
    for (int i = 0; i < g_widget_count; i++) {
        if (!g_widgets[i].overlay) continue;
        draw_widget(i, css_handle);
    }

    /* 6. Emitir ANSI */
    pt_flush();
}
