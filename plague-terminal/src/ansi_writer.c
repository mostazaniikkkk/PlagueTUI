#include "ansi_writer.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  static HANDLE g_out = INVALID_HANDLE_VALUE;
#endif

static char g_buf[65536];
static int  g_len = 0;

static void buf_write(const char* s, int n) {
    if (g_len + n >= (int)sizeof(g_buf))
        pt_ansi_commit();
    memcpy(g_buf + g_len, s, n);
    g_len += n;
}

void pt_ansi_init(void) {
#ifdef _WIN32
    g_out = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    g_len = 0;
}

void pt_ansi_commit(void) {
    if (g_len == 0) return;
#ifdef _WIN32
    if (g_out != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(g_out, g_buf, (DWORD)g_len, &written, NULL);
    }
#else
    fwrite(g_buf, 1, (size_t)g_len, stdout);
    fflush(stdout);
#endif
    g_len = 0;
}

void pt_ansi_move(int col, int row) {
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "\x1b[%d;%dH", row + 1, col + 1);
    buf_write(tmp, n);
}

void pt_ansi_fg_rgb(uint8_t r, uint8_t g, uint8_t b) {
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "\x1b[38;2;%d;%d;%dm", r, g, b);
    buf_write(tmp, n);
}

void pt_ansi_bg_rgb(uint8_t r, uint8_t g, uint8_t b) {
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "\x1b[48;2;%d;%d;%dm", r, g, b);
    buf_write(tmp, n);
}

void pt_ansi_write(const char* s, int len) {
    buf_write(s, len);
}

void pt_ansi_reset(void) {
    buf_write("\x1b[0m", 4);
}

void pt_ansi_alt_screen_on(void) {
    buf_write("\x1b[?1049h", 8);
    pt_ansi_commit();
}

void pt_ansi_alt_screen_off(void) {
    buf_write("\x1b[?1049l", 8);
    pt_ansi_commit();
}

void pt_ansi_hide_cursor(void) {
    buf_write("\x1b[?25l", 6);
    pt_ansi_commit();
}

void pt_ansi_show_cursor(void) {
    buf_write("\x1b[?25h", 6);
    pt_ansi_commit();
}

void pt_ansi_clear_screen(void) {
    buf_write("\x1b[2J", 4);
    pt_ansi_commit();
}
