# PlagueTUI

Reimplementación de [Textual](https://github.com/Textualize/textual) en C puro, distribuida como DLLs nativos consumibles desde cualquier lenguaje con FFI (Python, Rust, Go, C++, …).

```
plague-app          ← orquestador de alto nivel
  ├── plague-events     ← cola de eventos, bindings, bubbling, timers
  ├── plague-css        ← parser TCSS + motor de cascada
  ├── plague-layout     ← motor de layout (vertical/horizontal/dock/grid)
  └── plague-terminal   ← cell buffer, ANSI writer, input Win32
        ├── plague-drawcontext  ← tipos Color, TextStyle, comandos de dibujo
        └── plague-geometry     ← Offset, Size, Region, Spacing (Zig)
```

---

## Compilar

Cada módulo tiene su propio `build.bat`. Se compilan en orden de dependencia:

```bat
cd plague-geometry   && zig build          && cd ..
cd plague-drawcontext && build.bat         && cd ..
cd plague-terminal   && build.bat          && cd ..
cd plague-layout     && build.bat          && cd ..
cd plague-css        && build.bat          && cd ..
cd plague-events     && build.bat          && cd ..
cd plague-app        && build.bat          && cd ..
```

Cada `build.bat` deja el `.dll` en `<módulo>/bin/`.
`plague-app/build.bat` copia además todas las DLLs dependientes a `plague-app/bin/`.

### Requisitos

| Herramienta | Versión mínima | Para qué |
|-------------|---------------|----------|
| Zig | 0.13+ | plague-geometry |
| GCC (MinGW-w64) | 13+ | resto de módulos |
| Python | 3.10+ | tests y demo |

---

## Ejecutar el demo

```bat
cd _demo
python demo.py
```

Controles: `↑ ↓` navegar · `Enter` seleccionar · `q` salir · `Ctrl+Q` salida de emergencia.

Demo de referencia con Textual real (para comparar estilos):

```bat
cd _demo
.venv\Scripts\python demo_textual.py
```

---

## API — ABI C

Todos los módulos exportan una ABI C pura. El punto de entrada principal es `plague-app`.

### Tipos compartidos

```c
// plague-geometry
typedef struct { int x, y; }                    TG_Offset;
typedef struct { int x, y, width, height; }     TG_Region;
typedef struct { int top, right, bottom, left; } TG_Spacing;

// plague-drawcontext
typedef struct { float r, g, b, a; }  PG_Color;

// plague-layout
typedef struct { int type; int value; } PL_SizeValue;
// type: PL_SIZE_FIXED=0  → value = celdas exactas
//       PL_SIZE_FRACTION=1 → value = partes (fr)
```

### plague-app — API completa

```c
// Ciclo de vida
int  pa_init(void);                        // 1=ok, 0=error
int  pa_init_headless(int cols, int rows); // sin I/O real (tests)
void pa_shutdown(void);
int  pa_get_cols(void);
int  pa_get_rows(void);

// Estilos
int pa_css_load(const char *tcss);         // devuelve handle; 0=error

// Widgets  (PA_NO_PARENT = -1 para el widget raíz)
int  pa_widget_add(const char *type, const char *id,
                   const char *classes, int parent_id);
void pa_widget_set_text   (int wid, const char *text);
void pa_widget_set_size   (int wid, PL_SizeValue w, PL_SizeValue h);
void pa_widget_set_dock   (int wid, PL_DockEdge edge);
void pa_widget_set_layout (int wid, PL_LayoutType layout_type);
void pa_widget_set_padding(int wid, TG_Spacing padding);
TG_Region pa_widget_region(int wid);      // región del último render

// Render
void pa_render(void);

// Eventos
int  pa_bind_key(int wid, int key, int mods); // devuelve binding_id
int  pa_poll(void);      // no bloquea; -1=quit, 0=sin evento, >0=binding_id
int  pa_wait_poll(void); // bloquea hasta evento
void pa_quit(void);
```

#### Constantes de layout

```c
PL_LAYOUT_VERTICAL   = 0
PL_LAYOUT_HORIZONTAL = 1
PL_LAYOUT_DOCK       = 2
PL_LAYOUT_GRID       = 3

PL_DOCK_TOP    = 0
PL_DOCK_BOTTOM = 1
PL_DOCK_LEFT   = 2
PL_DOCK_RIGHT  = 3
PL_DOCK_CENTER = 4
```

#### Teclas especiales

```c
PE_KEY_UP    = 0x0100
PE_KEY_DOWN  = 0x0101
PE_KEY_LEFT  = 0x0102
PE_KEY_RIGHT = 0x0103
PE_KEY_ENTER = 0x000D
PE_KEY_F1    = 0x0110  // … F12 = 0x011B
```

---

## Ejemplo completo en Python

```python
import ctypes, sys, os
from pathlib import Path

BASE = Path(__file__).parent

# En Windows registrar todos los bin\ antes de cargar
if sys.platform == "win32":
    for mod in ("plague-terminal", "plague-layout", "plague-css",
                "plague-events", "plague-app"):
        os.add_dll_directory(str(BASE / mod / "bin"))

app = ctypes.CDLL(str(BASE / "plague-app" / "bin" / "plague_app.dll"))

# ── Tipos ──────────────────────────────────────────────────────────────────
class TG_Spacing(ctypes.Structure):
    _fields_ = [("top", ctypes.c_int), ("right", ctypes.c_int),
                ("bottom", ctypes.c_int), ("left", ctypes.c_int)]

class PL_SizeValue(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int), ("value", ctypes.c_int)]

def fixed(n):  return PL_SizeValue(0, n)   # PL_SIZE_FIXED
def fr(n=1):   return PL_SizeValue(1, n)   # PL_SIZE_FRACTION
def pad(n):    return TG_Spacing(n, n, n, n)

# ── Firmas ─────────────────────────────────────────────────────────────────
app.pa_init.restype              = ctypes.c_int
app.pa_shutdown.restype          = None
app.pa_css_load.argtypes         = [ctypes.c_char_p]
app.pa_css_load.restype          = ctypes.c_int
app.pa_widget_add.argtypes       = [ctypes.c_char_p, ctypes.c_char_p,
                                     ctypes.c_char_p, ctypes.c_int]
app.pa_widget_add.restype        = ctypes.c_int
app.pa_widget_set_text.argtypes  = [ctypes.c_int, ctypes.c_char_p]
app.pa_widget_set_size.argtypes  = [ctypes.c_int, PL_SizeValue, PL_SizeValue]
app.pa_widget_set_dock.argtypes  = [ctypes.c_int, ctypes.c_int]
app.pa_widget_set_layout.argtypes= [ctypes.c_int, ctypes.c_int]
app.pa_widget_set_padding.argtypes=[ctypes.c_int, TG_Spacing]
app.pa_render.restype            = None
app.pa_bind_key.argtypes         = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
app.pa_bind_key.restype          = ctypes.c_int
app.pa_wait_poll.restype         = ctypes.c_int

# ── TCSS ───────────────────────────────────────────────────────────────────
TCSS = b"""
Screen { background: #1e1e2e; }
Header { background: #313244; color: #cdd6f4; border: solid #89b4fa; }
Main   { background: #1e1e2e; color: #cdd6f4; padding: 1; }
"""

# ── App ────────────────────────────────────────────────────────────────────
if not app.pa_init():
    sys.exit("Error al inicializar la terminal")

try:
    app.pa_css_load(TCSS)

    # Árbol de widgets
    root = app.pa_widget_add(b"Screen", b"", b"", -1)
    app.pa_widget_set_layout(root, 2)  # PL_LAYOUT_DOCK

    header = app.pa_widget_add(b"Header", b"", b"", root)
    app.pa_widget_set_dock(header, 0)           # PL_DOCK_TOP
    app.pa_widget_set_size(header, fr(1), fixed(3))
    app.pa_widget_set_text(header, b"Mi App")

    main = app.pa_widget_add(b"Main", b"", b"", root)
    app.pa_widget_set_dock(main, 4)             # PL_DOCK_CENTER
    app.pa_widget_set_text(main, b"Hola mundo!")

    # Bindings
    bid_q = app.pa_bind_key(root, ord('q'), 0)

    app.pa_render()

    while True:
        ev = app.pa_wait_poll()
        if ev < 0 or ev == bid_q:
            break

finally:
    app.pa_shutdown()
```

---

## Estructura del repositorio

```
PlagueTUI/
├── plague-geometry/       ← Zig · tipos geométricos base
├── plague-drawcontext/    ← C   · Color, TextStyle, StubContext
├── plague-terminal/       ← C   · cell buffer, ANSI, input Win32
├── plague-layout/         ← C   · motor de layout
├── plague-css/            ← C   · parser TCSS + cascada
├── plague-events/         ← C   · cola, bindings, bubbling, timers
├── plague-app/            ← C   · orquestador (API pública)
└── _demo/
    ├── demo.py            ← demo PlagueTUI
    └── demo_textual.py    ← demo Textual (referencia visual)
```

---

## Tests

Cada módulo tiene `test.bat` que compila y ejecuta su suite con `unittest`:

```bat
cd plague-geometry    && zig build test
cd plague-drawcontext && test.bat
cd plague-terminal    && test.bat
cd plague-layout      && test.bat
cd plague-css         && test.bat
cd plague-events      && test.bat
cd plague-app         && test.bat
```
