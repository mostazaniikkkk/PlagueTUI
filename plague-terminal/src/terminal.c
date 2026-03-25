#include "../include/plague_terminal.h"
#include "cell_buffer.h"
#include "ansi_writer.h"
#include "draw_impl.h"

#ifdef _WIN32
  #include "win32/terminal_io.h"
#endif

static int g_headless = 0;

/* ---- Ciclo de vida ---- */

int pt_init(void) {
    g_headless = 0;
    pt_ansi_init();
#ifdef _WIN32
    if (pt_io_init() != 0) return -1;
    int cols, rows;
    pt_io_get_size(&cols, &rows);
#else
    int cols = 80, rows = 24;
#endif
    pt_cb_init(cols, rows);
    pt_draw_init();
    pt_ansi_alt_screen_on();
    pt_ansi_hide_cursor();
    pt_ansi_clear_screen();
    return 0;
}

int pt_init_headless(int cols, int rows) {
    g_headless = 1;
    pt_ansi_init();
    pt_cb_init(cols, rows);
    pt_draw_init();
    return 0;
}

void pt_shutdown(void) {
    if (!g_headless) {
        pt_ansi_show_cursor();
        pt_ansi_alt_screen_off();
#ifdef _WIN32
        pt_io_shutdown();
#endif
    }
}

void pt_get_size(int* cols, int* rows) {
#ifdef _WIN32
    if (!g_headless) { pt_io_get_size(cols, rows); return; }
#endif
    *cols = pt_cb_cols();
    *rows = pt_cb_rows();
}

void pt_resize(int cols, int rows) {
    pt_cb_resize(cols, rows);
    pt_draw_init();
}

/* ---- Buffer ---- */

PT_Cell pt_get_cell(int col, int row) { return pt_cb_get(col, row); }
void    pt_clear   (void)             { pt_cb_clear(); }
void    pt_flush   (void)             { if (!g_headless) pt_cb_flush(); }

/* ---- Draw context ---- */

void pt_fill_rect      (TG_Region r, PG_Color c)                    { pt_draw_fill_rect(r, c); }
void pt_stroke_rect    (TG_Region r, PG_Color c, float sw)          { pt_draw_stroke_rect(r, c, sw); }
void pt_draw_text      (TG_Offset p, const char* t, PG_TextStyle s) { pt_draw_draw_text(p, t, s); }
void pt_clip_push      (TG_Region r)                                 { pt_draw_clip_push(r); }
void pt_clip_pop       (void)                                        { pt_draw_clip_pop(); }
void pt_translate_push (TG_Offset o)                                 { pt_draw_translate_push(o); }
void pt_translate_pop  (void)                                        { pt_draw_translate_pop(); }

/* ---- Input ---- */

int pt_poll_event(PT_Event* out) {
    if (g_headless) { out->type = PT_EVENT_NONE; return 0; }
#ifdef _WIN32
    return pt_io_poll_event(out);
#else
    out->type = PT_EVENT_NONE;
    return 0;
#endif
}

void pt_wait_event(PT_Event* out) {
    if (g_headless) { out->type = PT_EVENT_NONE; return; }
#ifdef _WIN32
    pt_io_wait_event(out);
#else
    out->type = PT_EVENT_NONE;
#endif
}
