# plague-terminal

Renderer de terminal para PlagueTUI. Implementa un cell buffer de doble buffer (back/front) con diff-flush ANSI, box-drawing para bordes, e input de teclado/ratón via Win32 Console API.

## Responsabilidad

Convierte draw calls abstractas en secuencias ANSI escritas a la terminal. Es la capa de salida final del pipeline de renderizado.

## API pública

```c
// Ciclo de vida
int  pt_init(void);                        // configura Win32, entra alt-screen
int  pt_init_headless(int cols, int rows); // sin I/O real (para tests)
void pt_shutdown(void);
void pt_get_size(int *cols, int *rows);
void pt_resize(int cols, int rows);

// Buffer
PT_Cell pt_get_cell(int col, int row);
void    pt_clear(void);
void    pt_flush(void);  // emite ANSI sólo para celdas modificadas

// Draw calls
void pt_fill_rect    (TG_Region, PG_Color);
void pt_stroke_rect  (TG_Region, PG_Color, float stroke_width);
void pt_draw_text    (TG_Offset, const char *text, PG_TextStyle);
void pt_clip_push    (TG_Region);
void pt_clip_pop     (void);
void pt_translate_push(TG_Offset);
void pt_translate_pop (void);

// Input
int  pt_poll_event(PT_Event *out);  // no bloquea
void pt_wait_event(PT_Event *out);  // bloquea hasta evento
```

## Tipos principales

```c
typedef struct {
    char    ch[4]; uint8_t ch_len;
    uint8_t r_fg, g_fg, b_fg;
    uint8_t r_bg, g_bg, b_bg;
    uint8_t bold, italic, underline, _pad[2];  // 16 bytes
} PT_Cell;

typedef struct {
    PT_EventType type;
    union {
        struct { int keycode; int mods; char ch[4]; int ch_len; } key;
        struct { int x; int y; int button; int mods; }           mouse;
        struct { int cols; int rows; }                            resize;
    } data;
} PT_Event;
```

## Dependencias

- `plague-geometry` — `TG_Region`, `TG_Offset`
- `plague-drawcontext` — `PG_Color`, `PG_TextStyle`

## Compilar y testear

```bat
build.bat   # → bin\plague_terminal.dll
test.bat    # compila + ejecuta tests con unittest
```

## Detalles de implementación

| Módulo | Archivo | Descripción |
|--------|---------|-------------|
| Cell buffer | `src/cell_buffer.c` | Arrays `back[]` y `front[]`, max 512×256 |
| ANSI writer | `src/ansi_writer.c` | Buffer de 64 KB, escribe con `WriteFile` (Win32) |
| Draw impl | `src/draw_impl.c` | `fill_rect`, `stroke_rect` (box-drawing UTF-8), `draw_text`, clip/translate stack |
| Win32 I/O | `src/win32/terminal_io.c` | Configura modos de consola, traduce `KEY_EVENT`/`MOUSE_EVENT` |

### Box-drawing

`stroke_rect` usa caracteres UTF-8: `┌ ┐ └ ┘ ─ │` (3 bytes cada uno).

### Headless mode

`pt_init_headless(cols, rows)` omite todo el setup de Win32. `pt_flush` es no-op. Se usa en todos los tests.
