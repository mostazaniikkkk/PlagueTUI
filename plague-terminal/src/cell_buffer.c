#include "cell_buffer.h"
#include "ansi_writer.h"
#include <string.h>

static int     g_cols = 0;
static int     g_rows = 0;
static PT_Cell g_back [PT_MAX_ROWS][PT_MAX_COLS];
static PT_Cell g_front[PT_MAX_ROWS][PT_MAX_COLS];

static PT_Cell default_cell(void) {
    PT_Cell c;
    memset(&c, 0, sizeof(c));
    c.ch[0]  = ' ';
    c.ch_len = 1;
    return c;
}

void pt_cb_init(int cols, int rows) {
    if (cols > PT_MAX_COLS) cols = PT_MAX_COLS;
    if (rows > PT_MAX_ROWS) rows = PT_MAX_ROWS;
    g_cols = cols;
    g_rows = rows;
    PT_Cell def = default_cell();
    for (int r = 0; r < g_rows; r++)
        for (int c = 0; c < g_cols; c++) {
            g_back[r][c]  = def;
            g_front[r][c] = def;
        }
}

void pt_cb_resize(int cols, int rows) {
    pt_cb_init(cols, rows);
}

void pt_cb_clear(void) {
    PT_Cell def = default_cell();
    for (int r = 0; r < g_rows; r++)
        for (int c = 0; c < g_cols; c++)
            g_back[r][c] = def;
}

void pt_cb_set(int col, int row, PT_Cell cell) {
    if (col < 0 || col >= g_cols) return;
    if (row < 0 || row >= g_rows) return;
    g_back[row][col] = cell;
}

PT_Cell pt_cb_get(int col, int row) {
    if (col < 0 || col >= g_cols || row < 0 || row >= g_rows)
        return default_cell();
    return g_back[row][col];
}

int pt_cb_cols(void) { return g_cols; }
int pt_cb_rows(void) { return g_rows; }

void pt_cb_flush(void) {
    for (int r = 0; r < g_rows; r++) {
        for (int c = 0; c < g_cols; c++) {
            PT_Cell* back  = &g_back[r][c];
            PT_Cell* front = &g_front[r][c];
            if (memcmp(back, front, sizeof(PT_Cell)) == 0) continue;

            pt_ansi_move(c, r);
            pt_ansi_bg_rgb(back->r_bg, back->g_bg, back->b_bg);
            pt_ansi_fg_rgb(back->r_fg, back->g_fg, back->b_fg);
            pt_ansi_write(back->ch, back->ch_len ? back->ch_len : 1);
            *front = *back;
        }
    }
    pt_ansi_reset();
    pt_ansi_commit();
}
