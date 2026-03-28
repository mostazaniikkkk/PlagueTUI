#include "../include/plague_app.h"
#include "widgets.h"
#include "render.h"
#include "focus.h"

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
 * Click bindings
 * --------------------------------------------------------------------------- */

#define PA_MAX_CLICK_BINDS  64
#define PA_MAX_SCROLL_BINDS 32
#define PA_CLICK_BID_BASE   0x8000   /* rango separado de key bindings */

typedef struct { int wid; int bid; } ClickBind;
static ClickBind g_click_binds[PA_MAX_CLICK_BINDS];
static int       g_click_count    = 0;
static int       g_next_click_bid = PA_CLICK_BID_BASE;

typedef struct { int wid; int bid; } ScrollBind;
static ScrollBind g_scroll_binds[PA_MAX_SCROLL_BINDS];
static int        g_scroll_count    = 0;
static int        g_last_scroll_dy  = 0;

static int g_last_mouse_x = 0;
static int g_last_mouse_y = 0;

/* ---------------------------------------------------------------------------
 * Helpers internos: sincronización de estado de focus/hover
 * --------------------------------------------------------------------------- */

/* Actualiza PA_STATE_FOCUSED en todos los widgets según el foco actual. */
static void sync_focus_states(void)
{
    int focused = pa_focus_get_wid();
    for (int i = 0; i < g_widget_count; i++) {
        if (!g_widgets[i].used) continue;
        if (i == focused)
            g_widgets[i].state |= PA_STATE_FOCUSED;
        else
            g_widgets[i].state &= (uint8_t)~PA_STATE_FOCUSED;
    }
}

void pa_mouse_pos(int *x, int *y)
{
    if (x) *x = g_last_mouse_x;
    if (y) *y = g_last_mouse_y;
}

/* Actualiza PA_STATE_HOVER según la posición del mouse. */
static void update_hover(int mx, int my)
{
    g_last_mouse_x = mx;
    g_last_mouse_y = my;
    for (int i = 0; i < g_widget_count; i++) {
        if (!g_widgets[i].used) continue;
        TG_Region r = pa_last_region(i);
        if (mx >= r.x && mx < r.x + r.width &&
            my >= r.y && my < r.y + r.height)
            g_widgets[i].state |= PA_STATE_HOVER;
        else
            g_widgets[i].state &= (uint8_t)~PA_STATE_HOVER;
    }
}

/* Hit-test: busca el click binding para el widget más superficial en (mx, my).
 * Primero prueba overlays (z superior), luego widgets normales.
 * También transfiere el foco al widget clickeado si es focusable. */
static int dispatch_click(int mx, int my)
{
    /* Dos pasadas: overlays primero */
    for (int pass = 0; pass < 2; pass++) {
        for (int i = g_widget_count - 1; i >= 0; i--) {
            if (!g_widgets[i].used || !g_widgets[i].visible) continue;
            if (pass == 0 && !g_widgets[i].overlay) continue;
            if (pass == 1 &&  g_widgets[i].overlay) continue;

            TG_Region r = pa_last_region(i);
            if (mx < r.x || mx >= r.x + r.width)  continue;
            if (my < r.y || my >= r.y + r.height) continue;

            /* Transferir foco si el widget es focusable */
            if (g_widgets[i].focusable && !g_widgets[i].disabled) {
                pa_focus_set_wid(i);
                sync_focus_states();
                pa_render();
            }

            /* Buscar click binding registrado */
            for (int j = 0; j < g_click_count; j++) {
                if (g_click_binds[j].wid == i)
                    return g_click_binds[j].bid;
            }
            return 0;  /* hit sin binding */
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * Ciclo de vida
 * --------------------------------------------------------------------------- */

static int dispatch_scroll(int mx, int my, int dy)
{
    for (int i = g_widget_count - 1; i >= 0; i--) {
        if (!g_widgets[i].used || !g_widgets[i].visible) continue;
        TG_Region r = pa_last_region(i);
        if (mx < r.x || mx >= r.x + r.width)  continue;
        if (my < r.y || my >= r.y + r.height) continue;
        for (int j = 0; j < g_scroll_count; j++) {
            if (g_scroll_binds[j].wid == i) {
                g_last_scroll_dy = dy;
                return g_scroll_binds[j].bid;
            }
        }
        return 0;
    }
    return 0;
}

static void common_init(void)
{
    pa_widgets_init();
    pa_focus_init();
    pe_reset_all();
    g_css_handle     = 0;
    g_quit_flag      = 0;
    g_click_count    = 0;
    g_next_click_bid = PA_CLICK_BID_BASE;
    g_scroll_count   = 0;
    g_last_scroll_dy = 0;
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
    (void)ev_node;

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

void pa_widget_set_focusable(int wid, int focusable)
{
    if (wid < 0 || wid >= g_widget_count) return;
    g_widgets[wid].focusable = (uint8_t)(focusable ? 1 : 0);
}

void pa_widget_set_disabled(int wid, int disabled)
{
    if (wid < 0 || wid >= g_widget_count) return;
    g_widgets[wid].disabled = (uint8_t)(disabled ? 1 : 0);
    if (disabled)
        g_widgets[wid].state |= PA_STATE_DISABLED;
    else
        g_widgets[wid].state &= (uint8_t)~PA_STATE_DISABLED;
}

void pa_widget_set_overlay(int wid, int overlay)
{
    if (wid < 0 || wid >= g_widget_count) return;
    g_widgets[wid].overlay = (uint8_t)(overlay ? 1 : 0);
}

void pa_widget_scroll_to(int wid, int x, int y)
{
    if (wid < 0 || wid >= g_widget_count) return;
    g_widgets[wid].scroll_x = x;
    g_widgets[wid].scroll_y = y;
}

TG_Region pa_widget_region(int wid)
{
    return pa_last_region(wid);
}

/* ---------------------------------------------------------------------------
 * Foco
 * --------------------------------------------------------------------------- */

void pa_focus_set(int wid)
{
    if (wid < -1 || wid >= g_widget_count) return;
    pa_focus_set_wid(wid);
    sync_focus_states();
}

int pa_focus_get(void)
{
    return pa_focus_get_wid();
}

void pa_focus_next(void)
{
    pa_focus_advance(+1);
    sync_focus_states();
}

void pa_focus_prev(void)
{
    pa_focus_advance(-1);
    sync_focus_states();
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

int pa_bind_click(int wid)
{
    if (wid < 0 || wid >= g_widget_count) return 0;
    if (g_click_count >= PA_MAX_CLICK_BINDS) return 0;
    int bid = g_next_click_bid++;
    g_click_binds[g_click_count].wid = wid;
    g_click_binds[g_click_count].bid = bid;
    g_click_count++;
    return bid;
}

int pa_bind_scroll(int wid)
{
    if (wid < 0 || wid >= g_widget_count) return 0;
    if (g_scroll_count >= PA_MAX_SCROLL_BINDS) return 0;
    int bid = g_next_click_bid++;
    g_scroll_binds[g_scroll_count].wid = wid;
    g_scroll_binds[g_scroll_count].bid = bid;
    g_scroll_count++;
    return bid;
}

int pa_scroll_dy(void) { return g_last_scroll_dy; }

/* ---------------------------------------------------------------------------
 * Timers
 * --------------------------------------------------------------------------- */

int pa_timer_create(int interval_ms, int repeat)
{
    return pe_timer_create(interval_ms, repeat);
}

void pa_timer_cancel(int timer_id)
{
    pe_timer_cancel(timer_id);
}

int pa_tick_timers(int elapsed_ms)
{
    pe_timer_tick(elapsed_ms);
    PE_Event ev;
    if (pe_queue_pop(&ev) && ev.type == PE_EVENT_TIMER)
        return ev.data.timer.timer_id;
    return 0;
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
            int key  = (ev->data.key.ch_len == 1)
                       ? (unsigned char)ev->data.key.ch[0]
                       : vk_to_pe(ev->data.key.keycode);
            int mods = ev->data.key.mods;

            /* Ctrl+Q: salida de emergencia — siempre activo.
             * Usamos keycode ('Q'=0x51) para no confundir VK_CONTROL (0x11)
             * con el caracter de control 0x11 que genera Ctrl+Q. */
            if ((mods & PT_MOD_CTRL) && ev->data.key.keycode == 'Q') {
                g_quit_flag = 1;
                return -1;
            }

            /* Tab / Shift+Tab: mover foco */
            if (key == '\t') {
                if (mods & PT_MOD_SHIFT) pa_focus_prev();
                else                     pa_focus_next();
                pa_render();
                return 0;
            }

            int focused = pa_focus_get_wid();
            if (focused < 0) focused = 0;
            int bid = pe_dispatch_key(focused, key, mods);
            return bid;
        }

        case PT_EVENT_MOUSE: {
            int mx = ev->data.mouse.x, my = ev->data.mouse.y;
            update_hover(mx, my);
            if (ev->data.mouse.button == PT_MOUSE_LEFT) {
                int bid = dispatch_click(mx, my);
                if (bid > 0) return bid;
            } else if (ev->data.mouse.button == PT_MOUSE_SCROLL_UP) {
                int bid = dispatch_scroll(mx, my, -1);
                if (bid > 0) return bid;
            } else if (ev->data.mouse.button == PT_MOUSE_SCROLL_DOWN) {
                int bid = dispatch_scroll(mx, my, +1);
                if (bid > 0) return bid;
            }
            return 0;
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
