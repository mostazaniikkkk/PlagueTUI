#ifndef PLAGUE_TERMINAL_H
#define PLAGUE_TERMINAL_H

#include <stdint.h>
#include "../../plague-geometry/include/plague_geometry.h"
#include "../../plague-drawcontext/src/color.h"
#include "../../plague-drawcontext/src/text_style.h"

#ifdef _WIN32
  #define PT_API __declspec(dllexport)
#else
  #define PT_API __attribute__((visibility("default")))
#endif

// ---------------------------------------------------------------------------
// Cell — unidad mínima del buffer de terminal
// ---------------------------------------------------------------------------

typedef struct {
    char    ch[4];           /* glifo en UTF-8 (hasta 4 bytes)  */
    uint8_t ch_len;          /* bytes válidos en ch             */
    uint8_t r_fg, g_fg, b_fg; /* color de texto  (0-255)        */
    uint8_t r_bg, g_bg, b_bg; /* color de fondo  (0-255)        */
    uint8_t bold;
    uint8_t italic;
    uint8_t underline;
    uint8_t _pad[2];         /* relleno → 16 bytes totales      */
} PT_Cell;

// ---------------------------------------------------------------------------
// Eventos de entrada
// ---------------------------------------------------------------------------

typedef enum {
    PT_EVENT_NONE   = 0,
    PT_EVENT_KEY    = 1,
    PT_EVENT_MOUSE  = 2,
    PT_EVENT_RESIZE = 3,
} PT_EventType;

#define PT_MOD_SHIFT 0x01
#define PT_MOD_CTRL  0x02
#define PT_MOD_ALT   0x04

#define PT_MOUSE_LEFT        0
#define PT_MOUSE_RIGHT       1
#define PT_MOUSE_MIDDLE      2
#define PT_MOUSE_SCROLL_UP   3
#define PT_MOUSE_SCROLL_DOWN 4

typedef struct {
    PT_EventType type;
    union {
        struct { int keycode; int mods; char ch[4]; int ch_len; } key;
        struct { int x; int y; int button; int mods; }           mouse;
        struct { int cols; int rows; }                            resize;
    } data;
} PT_Event;

// ---------------------------------------------------------------------------
// Ciclo de vida
// ---------------------------------------------------------------------------

PT_API int  pt_init         (void);               /* terminal real — configura Win32 */
PT_API int  pt_init_headless(int cols, int rows);  /* sin I/O real — para tests       */
PT_API void pt_shutdown     (void);
PT_API void pt_get_size     (int* cols, int* rows);
PT_API void pt_resize       (int cols, int rows);

// ---------------------------------------------------------------------------
// Buffer de celdas
// ---------------------------------------------------------------------------

PT_API PT_Cell pt_get_cell(int col, int row);
PT_API void    pt_clear   (void);
PT_API void    pt_flush   (void);  /* emite ANSI para celdas cambiadas */

// ---------------------------------------------------------------------------
// Draw context — implementa la vtable PC_DrawContext del compositor
// ---------------------------------------------------------------------------

PT_API void pt_fill_rect      (TG_Region region, PG_Color color);
PT_API void pt_stroke_rect    (TG_Region region, PG_Color color, float stroke_width);
PT_API void pt_draw_text      (TG_Offset pos, const char* text, PG_TextStyle style);
PT_API void pt_clip_push      (TG_Region region);
PT_API void pt_clip_pop       (void);
PT_API void pt_translate_push (TG_Offset offset);
PT_API void pt_translate_pop  (void);

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

PT_API int  pt_poll_event(PT_Event* out);  /* 1 si hay evento, 0 si no */
PT_API void pt_wait_event(PT_Event* out);  /* bloquea hasta evento     */

#endif /* PLAGUE_TERMINAL_H */
