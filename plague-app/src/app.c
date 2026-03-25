#include "../include/plague_app.h"
#include "widgets.h"
#include "render.h"

#include "plague_terminal.h"
#include "plague_events.h"
#include "plague_css.h"

#include <string.h>
#include <stdlib.h>

/* ---------------------------------------------------------------------------
 * Estado global de la app
 * --------------------------------------------------------------------------- */

static int g_cols       = 80;
static int g_rows       = 24;
static int g_css_handle = 0;
static int g_quit_flag  = 0;
static int g_headless   = 0;

/* ---------------------------------------------------------------------------
 * Ciclo de vida
 * --------------------------------------------------------------------------- */

static void common_init(void)
{
    pa_widgets_init();
    pe_reset_all();
    g_css_handle = 0;
    g_quit_flag  = 0;
}

int pa_init(void)
{
    g_headless = 0;
    if (pt_init() != 0) return 0;
    pt_get_size(&g_cols, &g_rows);
    common_init();
    return 1;
}

int pa_init_headless(int cols, int rows)
{
    g_headless = 1;
    g_cols = cols;
    g_rows = rows;
    pt_init_headless(cols, rows);
    common_init();
    return 1;
}

void pa_shutdown(void)
{
    if (g_css_handle > 0) {
        pc_stylesheet_free(g_css_handle);
        g_css_handle = 0;
    }
    pt_shutdown();
}

int pa_get_cols(void) { return g_cols; }
int pa_get_rows(void) { return g_rows; }

/* ---------------------------------------------------------------------------
 * CSS
 * --------------------------------------------------------------------------- */

int pa_css_load(const char *tcss)
{
    if (g_css_handle > 0) pc_stylesheet_free(g_css_handle);
    g_css_handle = pc_stylesheet_load(tcss);
    return g_css_handle;
}

/* ---------------------------------------------------------------------------
 * Widgets
 * --------------------------------------------------------------------------- */

int pa_widget_add(const char *type, const char *id,
                  const char *classes, int parent_id)
{
    if (g_widget_count >= PA_MAX_WIDGETS) return -1;

    int wid = g_widget_count++;
    PA_Widget *w = &g_widgets[wid];
    memset(w, 0, sizeof(*w));

    if (type)    strncpy(w->type,    type,    PA_NAME_LEN  - 1);
    if (id)      strncpy(w->id,      id,      PA_NAME_LEN  - 1);
    if (classes) strncpy(w->classes, classes, PA_CLASS_LEN - 1);

    /* Defaults */
    PL_SizeValue fr1 = { PL_SIZE_FRACTION, 1 };
    w->width       = fr1;
    w->height      = fr1;
    w->layout_type = PL_LAYOUT_VERTICAL;
    w->dock        = PL_DOCK_CENTER;
    w->parent_wid  = parent_id;
    w->visible     = 1;
    w->used        = 1;

    /* Registrar como hijo del padre */
    if (parent_id >= 0 && parent_id < g_widget_count - 1) {
        PA_Widget *p = &g_widgets[parent_id];
        if (p->child_count < 32)
            p->children[p->child_count++] = wid;
    }

    /* Árbol de eventos */
    int ev_parent = (parent_id >= 0) ? parent_id : PE_NO_PARENT;
    int ev_node   = pe_tree_add(ev_parent);
    (void)ev_node;  /* mismo índice que wid si se añaden en orden */

    return wid;
}

void pa_widget_set_text(int wid, const char *text)
{
    if (wid < 0 || wid >= g_widget_count) return;
    if (text) strncpy(g_widgets[wid].text, text, PA_TEXT_LEN - 1);
    else      g_widgets[wid].text[0] = '\0';
}

void pa_widget_set_size(int wid, PL_SizeValue w, PL_SizeValue h)
{
    if (wid < 0 || wid >= g_widget_count) return;
    g_widgets[wid].width  = w;
    g_widgets[wid].height = h;
}

void pa_widget_set_dock(int wid, PL_DockEdge edge)
{
    if (wid < 0 || wid >= g_widget_count) return;
    g_widgets[wid].dock = edge;
}

void pa_widget_set_layout(int wid, PL_LayoutType layout_type)
{
    if (wid < 0 || wid >= g_widget_count) return;
    g_widgets[wid].layout_type = layout_type;
}

void pa_widget_set_padding(int wid, TG_Spacing padding)
{
    if (wid < 0 || wid >= g_widget_count) return;
    g_widgets[wid].padding = padding;
}

TG_Region pa_widget_region(int wid)
{
    return pa_last_region(wid);
}

/* ---------------------------------------------------------------------------
 * Render
 * --------------------------------------------------------------------------- */

void pa_render(void)
{
    pa_do_render(g_cols, g_rows, g_css_handle);
}

/* ---------------------------------------------------------------------------
 * Bindings
 * --------------------------------------------------------------------------- */

int pa_bind_key(int wid, int key, int mods)
{
    return pe_bind_key(wid, key, mods);
}

/* ---------------------------------------------------------------------------
 * Event loop
 * --------------------------------------------------------------------------- */

void pa_quit(void) { g_quit_flag = 1; }

/* Traduce VK codes de Win32 a las constantes PE_KEY_* para teclas especiales. */
static int vk_to_pe(int vk)
{
    switch (vk) {
        case 0x26: return 0x0100; /* PE_KEY_UP        */
        case 0x28: return 0x0101; /* PE_KEY_DOWN      */
        case 0x25: return 0x0102; /* PE_KEY_LEFT      */
        case 0x27: return 0x0103; /* PE_KEY_RIGHT     */
        case 0x24: return 0x0104; /* PE_KEY_HOME      */
        case 0x23: return 0x0105; /* PE_KEY_END       */
        case 0x21: return 0x0106; /* PE_KEY_PAGE_UP   */
        case 0x22: return 0x0107; /* PE_KEY_PAGE_DOWN */
        case 0x2D: return 0x0108; /* PE_KEY_INSERT    */
        case 0x2E: return 0x007F; /* PE_KEY_DELETE    */
        case 0x70: return 0x0110; /* PE_KEY_F1        */
        case 0x71: return 0x0111; /* PE_KEY_F2        */
        case 0x72: return 0x0112; /* PE_KEY_F3        */
        case 0x73: return 0x0113; /* PE_KEY_F4        */
        case 0x74: return 0x0114; /* PE_KEY_F5        */
        case 0x75: return 0x0115; /* PE_KEY_F6        */
        case 0x76: return 0x0116; /* PE_KEY_F7        */
        case 0x77: return 0x0117; /* PE_KEY_F8        */
        case 0x78: return 0x0118; /* PE_KEY_F9        */
        case 0x79: return 0x0119; /* PE_KEY_F10       */
        case 0x7A: return 0x011A; /* PE_KEY_F11       */
        case 0x7B: return 0x011B; /* PE_KEY_F12       */
        default:   return vk;
    }
}

static int process_pt_event(PT_Event *ev)
{
    switch (ev->type) {
        case PT_EVENT_KEY: {
            /* Chars imprimibles y control (Enter, Escape, etc.) usan ch[0] → ASCII.
             * Teclas especiales (flechas, F1-F12, etc.) usan VK traducido a PE_KEY_*. */
            int key  = (ev->data.key.ch_len == 1)
                       ? (unsigned char)ev->data.key.ch[0]
                       : vk_to_pe(ev->data.key.keycode);
            int mods = ev->data.key.mods;

            /* Ctrl+Q: salida de emergencia — siempre activo, sin importar bindings */
            if ((mods & PT_MOD_CTRL) && (key == 'q' || key == 'Q' || key == 0x11)) {
                g_quit_flag = 1;
                return -1;
            }

            int focused = pe_focus_get();
            if (focused < 0) focused = 0;
            int bid = pe_dispatch_key(focused, key, mods);
            return bid;  /* 0 si nadie capturó */
        }
        case PT_EVENT_RESIZE: {
            g_cols = ev->data.resize.cols;
            g_rows = ev->data.resize.rows;
            if (!g_headless) pt_resize(g_cols, g_rows);
            pa_render();
            return 0;
        }
        default:
            return 0;
    }
}

int pa_poll(void)
{
    if (g_quit_flag) return -1;
    PT_Event ev;
    if (!pt_poll_event(&ev)) return 0;
    return process_pt_event(&ev);
}

int pa_wait_poll(void)
{
    if (g_quit_flag) return -1;
    PT_Event ev;
    pt_wait_event(&ev);
    return process_pt_event(&ev);
}
