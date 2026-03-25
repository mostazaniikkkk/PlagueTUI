#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "terminal_io.h"
#include <string.h>

static HANDLE g_hin  = INVALID_HANDLE_VALUE;
static HANDLE g_hout = INVALID_HANDLE_VALUE;
static DWORD  g_orig_in_mode  = 0;
static DWORD  g_orig_out_mode = 0;

int pt_io_init(void) {
    SetConsoleOutputCP(65001);  /* UTF-8: necesario para box-drawing y Unicode */
    SetConsoleCP(65001);

    g_hin  = GetStdHandle(STD_INPUT_HANDLE);
    g_hout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_hin == INVALID_HANDLE_VALUE || g_hout == INVALID_HANDLE_VALUE)
        return -1;

    GetConsoleMode(g_hin,  &g_orig_in_mode);
    GetConsoleMode(g_hout, &g_orig_out_mode);

    /* Habilitar secuencias VT en salida */
    DWORD out_mode = g_orig_out_mode
                   | ENABLE_VIRTUAL_TERMINAL_PROCESSING
                   | DISABLE_NEWLINE_AUTO_RETURN;
    SetConsoleMode(g_hout, out_mode);

    /* Modo raw: sin echo, sin buffer de línea, con ratón y resize.
     * SIN ENABLE_VIRTUAL_TERMINAL_INPUT: así las flechas llegan como
     * Win32 VK codes (VK_UP etc.) en vez de secuencias VT (\x1b[A). */
    DWORD in_mode = ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT;
    SetConsoleMode(g_hin, in_mode);

    return 0;
}

void pt_io_shutdown(void) {
    if (g_hin  != INVALID_HANDLE_VALUE) SetConsoleMode(g_hin,  g_orig_in_mode);
    if (g_hout != INVALID_HANDLE_VALUE) SetConsoleMode(g_hout, g_orig_out_mode);
}

void pt_io_get_size(int* cols, int* rows) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (g_hout != INVALID_HANDLE_VALUE
        && GetConsoleScreenBufferInfo(g_hout, &csbi)) {
        *cols = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
    } else {
        *cols = 80;
        *rows = 24;
    }
}

/* Convierte una tecla Win32 a PT_Event */
static void translate_key(const KEY_EVENT_RECORD* ke, PT_Event* out) {
    out->type          = PT_EVENT_KEY;
    out->data.key.keycode = ke->wVirtualKeyCode;
    out->data.key.ch_len  = 0;
    out->data.key.mods    = 0;
    memset(out->data.key.ch, 0, 4);

    DWORD ctrl = ke->dwControlKeyState;
    if (ctrl & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) out->data.key.mods |= PT_MOD_CTRL;
    if (ctrl & (LEFT_ALT_PRESSED  | RIGHT_ALT_PRESSED))  out->data.key.mods |= PT_MOD_ALT;
    if (ctrl & SHIFT_PRESSED)                            out->data.key.mods |= PT_MOD_SHIFT;

    WCHAR wc = ke->uChar.UnicodeChar;
    if (wc > 0) {
        if (wc < 0x80) {
            out->data.key.ch[0] = (char)wc;
            out->data.key.ch_len = 1;
        } else if (wc < 0x800) {
            out->data.key.ch[0] = (char)(0xC0 | (wc >> 6));
            out->data.key.ch[1] = (char)(0x80 | (wc & 0x3F));
            out->data.key.ch_len = 2;
        } else {
            out->data.key.ch[0] = (char)(0xE0 | (wc >> 12));
            out->data.key.ch[1] = (char)(0x80 | ((wc >> 6) & 0x3F));
            out->data.key.ch[2] = (char)(0x80 | (wc & 0x3F));
            out->data.key.ch_len = 3;
        }
    }
}

static int read_event(PT_Event* out, int blocking) {
    if (g_hin == INVALID_HANDLE_VALUE) { out->type = PT_EVENT_NONE; return 0; }
    for (;;) {
        if (!blocking) {
            DWORD avail = 0;
            GetNumberOfConsoleInputEvents(g_hin, &avail);
            if (avail == 0) { out->type = PT_EVENT_NONE; return 0; }
        }
        INPUT_RECORD rec;
        DWORD read = 0;
        if (!ReadConsoleInputW(g_hin, &rec, 1, &read) || read == 0) {
            out->type = PT_EVENT_NONE;
            return 0;
        }
        switch (rec.EventType) {
        case KEY_EVENT:
            if (!rec.Event.KeyEvent.bKeyDown) continue;
            translate_key(&rec.Event.KeyEvent, out);
            return 1;
        case MOUSE_EVENT: {
            MOUSE_EVENT_RECORD* me = &rec.Event.MouseEvent;
            out->type          = PT_EVENT_MOUSE;
            out->data.mouse.x  = me->dwMousePosition.X;
            out->data.mouse.y  = me->dwMousePosition.Y;
            out->data.mouse.mods = 0;
            if (me->dwEventFlags & MOUSE_WHEELED) {
                out->data.mouse.button = (me->dwButtonState & 0xFF000000)
                                       ? PT_MOUSE_SCROLL_DOWN
                                       : PT_MOUSE_SCROLL_UP;
            } else {
                out->data.mouse.button = -1;
                if (me->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
                    out->data.mouse.button = PT_MOUSE_LEFT;
                else if (me->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
                    out->data.mouse.button = PT_MOUSE_RIGHT;
            }
            return 1;
        }
        case WINDOW_BUFFER_SIZE_EVENT:
            out->type             = PT_EVENT_RESIZE;
            out->data.resize.cols = rec.Event.WindowBufferSizeEvent.dwSize.X;
            out->data.resize.rows = rec.Event.WindowBufferSizeEvent.dwSize.Y;
            return 1;
        default:
            continue;
        }
    }
}

int  pt_io_poll_event(PT_Event* out) { return read_event(out, 0); }
void pt_io_wait_event(PT_Event* out) { read_event(out, 1); }
