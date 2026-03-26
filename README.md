# PlagueTUI

Reimplementación de [Textual](https://github.com/Textualize/textual) en C puro, distribuida como DLLs nativos consumibles desde cualquier lenguaje con FFI (Python, Rust, Go, C++, …).

```
plague-widgets      ← biblioteca de widgets de alto nivel (Phase 2-6)
  └── plague-app    ← orquestador: render, eventos, CSS, foco, ratón
        ├── plague-events     ← cola de eventos, bindings, bubbling, timers
        ├── plague-css        ← parser TCSS + motor de cascada
        ├── plague-layout     ← motor de layout (vertical/horizontal/dock)
        └── plague-terminal   ← cell buffer, ANSI writer, input Win32
              ├── plague-drawcontext  ← tipos Color, TextStyle
              └── plague-geometry     ← Offset, Size, Region, Spacing (Zig)
```

---

## Compilar

Cada módulo tiene su propio `build.bat`. Se compilan en orden de dependencia:

```bat
cd plague-geometry    && zig build   && cd ..
cd plague-drawcontext && build.bat   && cd ..
cd plague-terminal    && build.bat   && cd ..
cd plague-layout      && build.bat   && cd ..
cd plague-css         && build.bat   && cd ..
cd plague-events      && build.bat   && cd ..
cd plague-app         && build.bat   && cd ..
cd plague-widgets     && build.bat   && cd ..
```

Cada `build.bat` deja el `.dll` en `<módulo>/bin/` y copia sus dependencias al mismo directorio.

### Requisitos

| Herramienta | Versión mínima | Para qué |
|-------------|----------------|----------|
| Zig | 0.13+ | plague-geometry |
| GCC (MinGW-w64) | 11+ | resto de módulos |
| Python | 3.10+ | tests y demo |

---

## Demo interactivo

```bat
cd _demo
run_demo.bat
```

El demo ejerce todos los widgets implementados en una sola ventana interactiva.

| Tecla | Acción |
|-------|--------|
| `Tab` / `Shift+Tab` | Ciclar foco entre widgets interactivos |
| `j` / `k` | Mover cursor en la lista enfocada |
| `Space` | Toggle del widget enfocado (Checkbox / Switch / Toggle / SelectionList) |
| `p` | +10% a la barra de progreso |
| `1` / `2` / `3` | Seleccionar opción del RadioSet |
| `q` / `Ctrl+Q` | Salir |

El ratón está completamente habilitado: click en cualquier widget interactivo lo activa directamente.

---

## API — plague-app

### Tipos compartidos

```c
// plague-geometry
typedef struct { int x, y; }                    TG_Offset;
typedef struct { int x, y, width, height; }     TG_Region;
typedef struct { int top, right, bottom, left; } TG_Spacing;

// plague-layout
typedef struct { int type; int value; } PL_SizeValue;
// type: PL_SIZE_FIXED=0    → value = celdas exactas
//       PL_SIZE_FRACTION=1 → value = partes proporcionales (fr)
```

### Funciones

```c
// Ciclo de vida
int  pa_init(void);                        // 1=ok, 0=error
int  pa_init_headless(int cols, int rows); // sin I/O real (para tests)
void pa_shutdown(void);
int  pa_get_cols(void);
int  pa_get_rows(void);

// CSS
int pa_css_load(const char *tcss);         // devuelve handle; 0=error

// Árbol de widgets  (PA_NO_PARENT = -1 para la raíz)
int  pa_widget_add        (const char *type, const char *id,
                           const char *classes, int parent_id);
void pa_widget_set_text   (int wid, const char *text);
void pa_widget_set_size   (int wid, PL_SizeValue w, PL_SizeValue h);
void pa_widget_set_dock   (int wid, PL_DockEdge edge);
void pa_widget_set_layout (int wid, PL_LayoutType layout_type);
void pa_widget_set_padding(int wid, TG_Spacing padding);
void pa_widget_set_focusable(int wid, int focusable);
void pa_widget_set_visible(int wid, int visible);
TG_Region pa_widget_region(int wid);      // región del último render

// Render
void pa_render(void);

// Foco
int  pa_focus_get(void);                  // wid del widget enfocado (-1 si ninguno)
void pa_focus_next(void);
void pa_focus_prev(void);

// Eventos y bindings
int  pa_bind_key  (int wid, int key, int mods); // devuelve binding_id
int  pa_bind_click(int wid);                     // devuelve binding_id
void pa_mouse_pos (int *x, int *y);              // coordenadas del último click

int  pa_poll(void);       // no bloquea; -1=quit, 0=sin evento, >0=binding_id
int  pa_wait_poll(void);  // bloquea hasta evento
void pa_quit(void);

// Timers
int pa_timer_create(int interval_ms, int repeat); // devuelve timer_id
int pa_tick_timers (int elapsed_ms);              // devuelve timer_id si disparó
```

### Constantes de layout

```c
PL_LAYOUT_VERTICAL   = 0
PL_LAYOUT_HORIZONTAL = 1
PL_LAYOUT_DOCK       = 2

PL_DOCK_TOP    = 0
PL_DOCK_BOTTOM = 1
PL_DOCK_LEFT   = 2
PL_DOCK_RIGHT  = 3
PL_DOCK_CENTER = 4
```

---

## API — plague-widgets

Capa de widgets predefinidos sobre plague-app. Requiere llamar `pw_init()` después de `pa_init()`.

```c
void pw_init(void);
void pw_shutdown(void);
const char *pw_default_tcss(void);   // TCSS por defecto para todos los widgets
```

### Widgets de display

| Widget | Función de creación | Descripción |
|--------|---------------------|-------------|
| Static | `pw_static_create(text, parent)` | Texto plano, sin interacción |
| Label | `pw_label_create(text, variant, parent)` | Texto con variante de color |
| Rule | `pw_rule_create(line_char, parent)` | Separador horizontal |
| Placeholder | `pw_placeholder_create(label, parent)` | Cuadro de color para debug |
| ProgressBar | `pw_progressbar_create(total, parent)` | Barra de progreso `░▓█` |
| Sparkline | `pw_sparkline_create(parent)` | Mini gráfico de barras `▁▂▃▄▅▆▇█` |
| Digits | `pw_digits_create(text, parent)` | Números grandes en box-drawing (3 filas) |
| LoadingIndicator | `pw_loading_create(parent)` | Spinner de braille animado |

### Widgets de sistema

| Widget | Función de creación | Descripción |
|--------|---------------------|-------------|
| Header | `pw_header_create(title, icon, parent)` | Barra superior con título y reloj |
| Footer | `pw_footer_create(parent)` | Barra inferior con atajos de teclado |

### Widgets interactivos

| Widget | Función de creación | Descripción |
|--------|---------------------|-------------|
| ToggleButton | `pw_toggle_create(label, checked, parent)` | `○ label` / `● label` |
| Checkbox | `pw_checkbox_create(label, checked, parent)` | `[ ] label` / `[X] label` |
| Switch | `pw_switch_create(active, parent)` | `○╺━━` / `━━╸●` — ancho fijo 4 celdas |
| Button | `pw_button_create(label, variant, parent)` | Botón clickeable con variantes de color |
| RadioButton | `pw_radio_create(label, checked, parent)` | `◯ label` / `◉ label` |
| RadioSet | `pw_radioset_create(parent)` | Contenedor de RadioButtons (exclusión mutua) |

### Listas

| Widget | Función de creación | Descripción |
|--------|---------------------|-------------|
| OptionList | `pw_optionlist_create(parent)` | Lista con cursor `▸`, un ítem activo |
| ListView | `pw_listview_create(parent)` | Lista con cursor `▶` |
| SelectionList | `pw_selectionlist_create(parent)` | Lista multi-selección `[ ]` / `[X]` |

### Contenedores y overlays

| Widget | Función de creación | Descripción |
|--------|---------------------|-------------|
| ContentSwitcher | `pw_switcher_create(parent)` | Muestra exactamente un hijo a la vez |
| Toast | `pw_toast_create(message, duration_ms, parent)` | Notificación flotante con auto-dismiss |

---

## Ejemplo rápido en Python

```python
import ctypes, os, sys
from pathlib import Path

ROOT = Path(__file__).parent
os.add_dll_directory(str(ROOT / "plague-app"    / "bin"))
os.add_dll_directory(str(ROOT / "plague-widgets"/ "bin"))

app = ctypes.CDLL(str(ROOT / "plague-app"     / "bin" / "plague_app.dll"))
pw  = ctypes.CDLL(str(ROOT / "plague-widgets" / "bin" / "plague_widgets.dll"))

class PL_SizeValue(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int), ("value", ctypes.c_int)]

def fixed(n): return PL_SizeValue(0, n)
def fr(n=1):  return PL_SizeValue(1, n)

# Firmas mínimas
app.pa_widget_add.restype        = ctypes.c_int
app.pa_widget_add.argtypes       = [ctypes.c_char_p]*3 + [ctypes.c_int]
app.pa_widget_set_size.argtypes  = [ctypes.c_int, PL_SizeValue, PL_SizeValue]
app.pa_widget_set_layout.argtypes= [ctypes.c_int, ctypes.c_int]
app.pa_bind_key.restype          = ctypes.c_int
app.pa_bind_key.argtypes         = [ctypes.c_int]*3
app.pa_poll.restype              = ctypes.c_int
pw.pw_default_tcss.restype       = ctypes.c_char_p
pw.pw_label_create.restype       = ctypes.c_int
pw.pw_label_create.argtypes      = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]

app.pa_init()
pw.pw_init()
app.pa_css_load(pw.pw_default_tcss())

screen = app.pa_widget_add(b"Screen", b"", b"", -1)
app.pa_widget_set_layout(screen, 0)             # VERTICAL
app.pa_widget_set_size(screen, fr(), fr())

pw.pw_label_create(b"Hola desde PlagueTUI!", b"primary", screen)

bid_q = app.pa_bind_key(screen, ord('q'), 0)
app.pa_render()

while True:
    ev = app.pa_poll()
    if ev < 0 or ev == bid_q:
        break

pw.pw_shutdown()
app.pa_shutdown()
```

---

## Estructura del repositorio

```
PlagueTUI/
├── plague-geometry/       ← Zig  · tipos geométricos base
├── plague-drawcontext/    ← C    · Color, TextStyle
├── plague-terminal/       ← C    · cell buffer, ANSI writer, input Win32
├── plague-layout/         ← C    · motor de layout (vertical/horizontal/dock)
├── plague-css/            ← C    · parser TCSS + motor de cascada
├── plague-events/         ← C    · cola de eventos, bindings, bubbling, timers
├── plague-app/            ← C    · orquestador (API pública principal)
├── plague-widgets/        ← C    · biblioteca de widgets de alto nivel
└── _demo/
    └── demo_plague_widgets.py   ← demo interactivo (todos los widgets)
```

---

## Tests

```bat
cd plague-geometry    && zig build test
cd plague-layout      && test.bat
cd plague-css         && test.bat
cd plague-events      && test.bat
cd plague-app         && test.bat      && cd ..   :: 40 tests
cd plague-widgets     && test.bat      && cd ..   :: 181 tests
```

O desde Python directamente:

```bat
cd plague-app     && python -m unittest discover -v -s tests
cd plague-widgets && python -m unittest discover -v -s tests
```

---

## Estado del proyecto

| Módulo | Estado |
|--------|--------|
| plague-geometry | Estable |
| plague-drawcontext | Estable |
| plague-terminal | Estable — mouse Win32 con QUICK_EDIT deshabilitado |
| plague-layout | Estable |
| plague-css | Estable |
| plague-events | Estable |
| plague-app | Estable — incluye `pa_bind_click`, `pa_mouse_pos`, timers |
| plague-widgets | Estable — Phases 2-6 completas (21 tipos de widget) |
