#include "draw_impl.h"
#include "cell_buffer.h"
#include <string.h>
#include <stdint.h>

#define STACK_DEPTH 8

static TG_Region g_clip     [STACK_DEPTH];
static int       g_clip_depth = 0;
static TG_Offset g_translate [STACK_DEPTH];
static int       g_translate_depth = 0;

/* Caracteres de borde (UTF-8, 3 bytes cada uno) */
static const char BOX_TL[3] = {'\xe2', '\x94', '\x8c'};  /* ┌ U+250C */
static const char BOX_TR[3] = {'\xe2', '\x94', '\x90'};  /* ┐ U+2510 */
static const char BOX_BL[3] = {'\xe2', '\x94', '\x94'};  /* └ U+2514 */
static const char BOX_BR[3] = {'\xe2', '\x94', '\x98'};  /* ┘ U+2518 */
static const char BOX_H [3] = {'\xe2', '\x94', '\x80'};  /* ─ U+2500 */
static const char BOX_V [3] = {'\xe2', '\x94', '\x82'};  /* │ U+2502 */

static uint8_t to_u8(float f) {
    if (f <= 0.0f) return 0;
    if (f >= 1.0f) return 255;
    return (uint8_t)(f * 255.0f);
}

static TG_Offset cur_translate(void) {
    if (g_translate_depth == 0) { TG_Offset z = {0, 0}; return z; }
    return g_translate[g_translate_depth - 1];
}

static TG_Region cur_clip(void) {
    if (g_clip_depth == 0) {
        TG_Region full = {0, 0, pt_cb_cols(), pt_cb_rows()};
        return full;
    }
    return g_clip[g_clip_depth - 1];
}

static void set_cell(int col, int row, PT_Cell cell) {
    TG_Offset t  = cur_translate();
    int sc = col + t.x;
    int sr = row + t.y;
    TG_Region cl = cur_clip();
    if (sc < cl.x || sc >= cl.x + cl.width)  return;
    if (sr < cl.y || sr >= cl.y + cl.height) return;
    pt_cb_set(sc, sr, cell);
}

void pt_draw_init(void) {
    g_clip_depth      = 0;
    g_translate_depth = 0;
}

/* ---- fill_rect ---- */

void pt_draw_fill_rect(TG_Region region, PG_Color color) {
    if (color.a <= 0.0f) return;
    PT_Cell cell;
    memset(&cell, 0, sizeof(cell));
    cell.ch[0]  = ' ';
    cell.ch_len = 1;
    cell.r_bg   = to_u8(color.r);
    cell.g_bg   = to_u8(color.g);
    cell.b_bg   = to_u8(color.b);
    for (int r = region.y; r < region.y + region.height; r++)
        for (int c = region.x; c < region.x + region.width; c++)
            set_cell(c, r, cell);
}

/* ---- stroke_rect ---- */

static void set_box(int col, int row, const char glyph[3], PG_Color color) {
    /* Heredar background del fill_rect subyacente */
    TG_Offset t = cur_translate();
    PT_Cell cell = pt_cb_get(col + t.x, row + t.y);
    cell.ch[0]  = glyph[0];
    cell.ch[1]  = glyph[1];
    cell.ch[2]  = glyph[2];
    cell.ch[3]  = 0;
    cell.ch_len = 3;
    cell.r_fg   = to_u8(color.r);
    cell.g_fg   = to_u8(color.g);
    cell.b_fg   = to_u8(color.b);
    set_cell(col, row, cell);
}

void pt_draw_stroke_rect(TG_Region region, PG_Color color, float stroke_width) {
    (void)stroke_width;  /* en terminal siempre 1 celda de grosor */
    if (region.width <= 0 || region.height <= 0) return;

    int x0 = region.x,                        y0 = region.y;
    int x1 = region.x + region.width  - 1,    y1 = region.y + region.height - 1;

    set_box(x0, y0, BOX_TL, color);
    set_box(x1, y0, BOX_TR, color);
    set_box(x0, y1, BOX_BL, color);
    set_box(x1, y1, BOX_BR, color);

    for (int c = x0 + 1; c < x1; c++) {
        set_box(c, y0, BOX_H, color);
        set_box(c, y1, BOX_H, color);
    }
    for (int r = y0 + 1; r < y1; r++) {
        set_box(x0, r, BOX_V, color);
        set_box(x1, r, BOX_V, color);
    }
}

/* ---- draw_text ---- */

static int utf8_len(unsigned char byte) {
    if (byte < 0x80) return 1;
    if (byte < 0xE0) return 2;
    if (byte < 0xF0) return 3;
    return 4;
}

void pt_draw_draw_text(TG_Offset pos, const char* text, PG_TextStyle style) {
    if (!text) return;
    int col = pos.x;
    const unsigned char* p = (const unsigned char*)text;
    while (*p) {
        /* Salto de línea: bajar una fila y volver al x original */
        if (*p == '\n') {
            pos.y++;
            col = pos.x;
            p++;
            continue;
        }

        int len = utf8_len(*p);
        int ok  = 1;
        for (int i = 1; i < len; i++)
            if (!p[i]) { ok = 0; break; }
        if (!ok) break;

        /* Heredar el background del fill_rect subyacente en lugar de poner negro */
        TG_Offset t = cur_translate();
        PT_Cell cell = pt_cb_get(col + t.x, pos.y + t.y);
        for (int i = 0; i < 4; i++) cell.ch[i] = 0;
        for (int i = 0; i < len; i++) cell.ch[i] = (char)p[i];
        cell.ch_len  = (uint8_t)len;
        cell.r_fg    = to_u8(style.color.r);
        cell.g_fg    = to_u8(style.color.g);
        cell.b_fg    = to_u8(style.color.b);
        cell.bold    = (uint8_t)style.bold;
        cell.italic  = (uint8_t)style.italic;

        set_cell(col, pos.y, cell);
        col++;
        p += len;
    }
}

/* ---- clip / translate ---- */

void pt_draw_clip_push(TG_Region region) {
    if (g_clip_depth >= STACK_DEPTH) return;
    TG_Offset t = cur_translate();
    /* Convertir region a coordenadas de pantalla e intersectar con clip actual */
    int sx0 = region.x + t.x,           sy0 = region.y + t.y;
    int sx1 = sx0 + region.width,        sy1 = sy0 + region.height;
    TG_Region cur = cur_clip();
    int cx0 = cur.x,                     cy0 = cur.y;
    int cx1 = cur.x + cur.width,         cy1 = cur.y + cur.height;
    int rx0 = sx0 > cx0 ? sx0 : cx0,    ry0 = sy0 > cy0 ? sy0 : cy0;
    int rx1 = sx1 < cx1 ? sx1 : cx1,    ry1 = sy1 < cy1 ? sy1 : cy1;
    TG_Region clipped = {rx0, ry0, rx1 > rx0 ? rx1 - rx0 : 0, ry1 > ry0 ? ry1 - ry0 : 0};
    g_clip[g_clip_depth++] = clipped;
}

void pt_draw_clip_pop(void) {
    if (g_clip_depth > 0) g_clip_depth--;
}

void pt_draw_translate_push(TG_Offset offset) {
    if (g_translate_depth >= STACK_DEPTH) return;
    TG_Offset cur = cur_translate();
    TG_Offset next = {cur.x + offset.x, cur.y + offset.y};
    g_translate[g_translate_depth++] = next;
}

void pt_draw_translate_pop(void) {
    if (g_translate_depth > 0) g_translate_depth--;
}
