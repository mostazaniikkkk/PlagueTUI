#ifndef PT_ANSI_WRITER_H
#define PT_ANSI_WRITER_H

#include <stdint.h>

void pt_ansi_init        (void);
void pt_ansi_commit      (void);  /* vuelca el buffer de salida */

void pt_ansi_move        (int col, int row);
void pt_ansi_fg_rgb      (uint8_t r, uint8_t g, uint8_t b);
void pt_ansi_bg_rgb      (uint8_t r, uint8_t g, uint8_t b);
void pt_ansi_write       (const char* s, int len);
void pt_ansi_reset       (void);
void pt_ansi_bold        (void);   /* \x1b[1m */
void pt_ansi_italic      (void);   /* \x1b[3m */

void pt_ansi_alt_screen_on (void);
void pt_ansi_alt_screen_off(void);
void pt_ansi_hide_cursor   (void);
void pt_ansi_show_cursor   (void);
void pt_ansi_clear_screen  (void);

#endif
