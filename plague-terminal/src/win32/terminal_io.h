#ifndef PT_TERMINAL_IO_H
#define PT_TERMINAL_IO_H

#include "../../include/plague_terminal.h"

int  pt_io_init      (void);         /* 0 = ok, -1 = error */
void pt_io_shutdown  (void);
void pt_io_get_size  (int* cols, int* rows);
int  pt_io_poll_event(PT_Event* out); /* 1 si hay evento */
void pt_io_wait_event(PT_Event* out); /* bloquea          */

#endif
