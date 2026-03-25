#ifndef PT_CELL_BUFFER_H
#define PT_CELL_BUFFER_H

#include "../include/plague_terminal.h"

#define PT_MAX_COLS 512
#define PT_MAX_ROWS 256

void    pt_cb_init  (int cols, int rows);
void    pt_cb_resize(int cols, int rows);
void    pt_cb_clear (void);
void    pt_cb_set   (int col, int row, PT_Cell cell);
PT_Cell pt_cb_get   (int col, int row);
int     pt_cb_cols  (void);
int     pt_cb_rows  (void);

/* Diff back vs front → emite ANSI para celdas cambiadas → copia back→front */
void    pt_cb_flush (void);

#endif
