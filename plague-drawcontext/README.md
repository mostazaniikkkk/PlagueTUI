# plague-drawcontext

API abstracta de dibujo 2D para PlagueTUI. Define el vocabulario de comandos de renderizado que el Compositor emite hacia cualquier backend gráfico (Cairo, Skia, etc.).

Este módulo **no dibuja nada** — define qué se puede pedir. Incluye un `StubContext` para testing que registra los comandos sin ejecutarlos.

**Depende de**: plague-geometry

---

## Compilar

```bat
build.bat
```

El DLL queda en `bin/plague_drawcontext.dll`.

---

## Tipos

### Color

```c
typedef struct { float r, g, b, a; } PG_Color;  // canales en [0.0, 1.0]
```

### TextStyle

```c
typedef struct {
    char     font_name[64];
    float    font_size;
    PG_Color color;
    int      bold;
    int      italic;
} PG_TextStyle;
```

### DrawCommand

Tagged union que representa una operación de dibujo:

```c
typedef struct {
    PG_DrawCommandType type;  // enum: qué operación es
    union { ... };            // datos según el tipo
} PG_DrawCommand;
```

---

## Usar desde Python

```python
import ctypes
from pathlib import Path

lib = ctypes.CDLL(str(Path("bin/plague_drawcontext.dll")))

class Color(ctypes.Structure):
    _fields_ = [("r", ctypes.c_float), ("g", ctypes.c_float),
                ("b", ctypes.c_float), ("a", ctypes.c_float)]

lib.pg_color_rgba.argtypes = [ctypes.c_float] * 4
lib.pg_color_rgba.restype  = Color

# Crear un color
red = lib.pg_color_rgba(1.0, 0.0, 0.0, 1.0)

# Mezclar dos colores al 50%
white = lib.pg_color_rgba(1.0, 1.0, 1.0, 1.0)
lib.pg_color_mix.argtypes = [Color, Color, ctypes.c_float]
lib.pg_color_mix.restype  = Color
pink = lib.pg_color_mix(red, white, 0.5)
```

### Usar el StubContext desde Python

```python
# Registrar comandos
lib.pg_stub_reset.argtypes = []
lib.pg_stub_reset.restype  = None
lib.pg_stub_count.argtypes = []
lib.pg_stub_count.restype  = ctypes.c_int

lib.pg_stub_fill_rect.argtypes = [Region, Color]
lib.pg_stub_fill_rect.restype  = None

lib.pg_stub_reset()
lib.pg_stub_fill_rect(Region(0, 0, 100, 100), red)
assert lib.pg_stub_count() == 1
```

---

## Usar desde C / C++

```c
#include "include/plague_drawcontext.h"

// Color desde hex RRGGBBAA
PG_Color bg = pg_color_from_hex(0x1a1a2eff);

// Mezcla con transparencia
PG_Color overlay = pg_color_with_alpha(bg, 0.8f);

// TextStyle
PG_TextStyle style = pg_text_style("Inter", 14.0f,
                                    pg_color_rgba(1, 1, 1, 1), 0, 0);
```

---

## API Completa

### Color

| Función | Descripción |
|---------|-------------|
| `pg_color_rgba(r, g, b, a)` | Constructor RGBA |
| `pg_color_from_hex(0xRRGGBBAA)` | Desde entero hex |
| `pg_color_mix(a, b, t)` | Interpolación lineal (`t` en `[0,1]`) |
| `pg_color_with_alpha(c, a)` | Retorna color con alpha modificado |
| `pg_color_eq(a, b)` | Igualdad |

Constantes: `PG_COLOR_BLACK`, `PG_COLOR_WHITE`, `PG_COLOR_TRANSPARENT`

### TextStyle

| Función | Descripción |
|---------|-------------|
| `pg_text_style(font, size, color, bold, italic)` | Constructor |

### StubContext

| Función | Descripción |
|---------|-------------|
| `pg_stub_reset()` | Limpia el buffer de comandos |
| `pg_stub_count()` | Número de comandos registrados |
| `pg_stub_get(index)` | Obtiene el comando en la posición dada |
| `pg_stub_fill_rect(region, color)` | Registra rectángulo relleno |
| `pg_stub_stroke_rect(region, color, width)` | Registra rectángulo con borde |
| `pg_stub_draw_text(pos, text, style)` | Registra texto |
| `pg_stub_rounded_rect(region, radius, color)` | Registra rectángulo redondeado |
| `pg_stub_draw_shadow(region, blur, offset, color)` | Registra sombra |
| `pg_stub_clip_push(region)` | Push de región de recorte |
| `pg_stub_clip_pop()` | Pop de región de recorte |
| `pg_stub_translate_push(offset)` | Push de transformación |
| `pg_stub_translate_pop()` | Pop de transformación |

Buffer máximo del stub: **256 comandos**.

---

## Tests

```bat
test.bat
```

Compila el DLL y corre los tests unitarios con `unittest`.
