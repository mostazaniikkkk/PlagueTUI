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

Pasa `pw_default_tcss()` a `pa_css_load()` si no tienes tu propia hoja de estilos.

---

### Display — texto y datos

#### Static / Label

Texto sin interacción. `Label` añade una clase CSS de variante.

```c
int  pw_static_create(const char *text, int parent_wid);
int  pw_label_create (const char *text, const char *variant, int parent_wid);
// variant: NULL | "primary" | "secondary" | "success" | "warning" | "error"
void pw_label_set_text(int wid, const char *text);
```

#### Rule

Separador horizontal relleno de un carácter unicode.

```c
int  pw_rule_create  (const char *line_char, int parent_wid);
// line_char: "─" | "━" | "═" | NULL → "─"
void pw_rule_set_char(int wid, const char *line_char);
```

#### Placeholder

Rectángulo de color con etiqueta centrada. Útil para maquetar.

```c
int  pw_placeholder_create    (const char *label, int parent_wid);
// label: NULL → muestra el wid numérico
void pw_placeholder_set_variant(int wid, int variant_index);
// variant_index 0-6 cicla colores predefinidos
```

#### ProgressBar

Barra de progreso con caracteres unicode `░▓█`. Tamaño `fr(1) × fixed(1)`.

```c
int   pw_progressbar_create     (float total, int parent_wid);
void  pw_progressbar_set_progress(int wid, float progress);
// Llamar DESPUÉS de pa_render() para recalcular el ancho:
void  pw_progressbar_update     (int wid);
float pw_progressbar_get_progress(int wid);
float pw_progressbar_get_total  (int wid);
```

#### Sparkline

Gráfico de barras en una sola fila usando `▁▂▃▄▅▆▇█`.

```c
int  pw_sparkline_create  (int parent_wid);
void pw_sparkline_set_data(int wid, const float *data, int count);
// Los valores se normalizan automáticamente al rango [min, max].
```

#### Digits

Números de gran tamaño (3 filas) trazados con box-drawing characters.

```c
int  pw_digits_create  (const char *text, int parent_wid);
void pw_digits_set_text(int wid, const char *text);
// Caracteres soportados: 0-9  :  .  ,  +  -  (espacio)
```

#### LoadingIndicator

Spinner de braille animado `⣾⣽⣻⢿⡿⣟⣯⣷`. El widget NO registra un timer propio.

```c
int  pw_loading_create(int parent_wid);
void pw_loading_start (int wid);
void pw_loading_stop  (int wid);
void pw_loading_tick  (int wid, int elapsed_ms);  // avanza la animación
```

Uso típico en el loop:

```c
int fired = pa_tick_timers(elapsed_ms);
if (fired == my_timer) pw_loading_tick(spinner_wid, elapsed_ms);
```

#### Markdown

Renderizador Markdown de solo lectura. Tamaño `fr(1) × fr(1)`.

```c
int  pw_markdown_create  (int parent_wid);
void pw_markdown_set_text(int wid, const char *text);
```

Sintaxis soportada:

| Elemento | Sintaxis |
|----------|----------|
| Encabezados | `# H1`  `## H2`  `### H3` |
| Negrita | `**texto**` |
| Cursiva | `*texto*` |
| Código inline | `` `código` `` |
| Viñetas | `- item` o `* item` |

---

### Sistema

#### Header

Barra de título anclada al borde superior (`height=1`). Muestra icono + título centrado + reloj opcional.

```c
int  pw_header_create   (const char *title, const char *icon, int parent_wid);
// icon: string UTF-8 corto (emoji OK). NULL = sin icono.
void pw_header_set_title(int wid, const char *title);
void pw_header_set_icon (int wid, const char *icon);
void pw_header_set_clock(int wid, int show);   // 1=mostrar reloj, 0=ocultar
void pw_header_tick     (int wid);             // actualiza el texto del reloj
```

Llama a `pw_header_tick()` una vez por tick del loop cuando el reloj está activo.

#### Footer

Barra de atajos anclada al borde inferior (`height=1`). Muestra pares `[tecla] descripción`.

```c
int  pw_footer_create    (int parent_wid);
void pw_footer_add_key   (int wid, const char *key_label, const char *description);
// Máximo 16 hints por footer.
void pw_footer_clear_keys(int wid);
void pw_footer_refresh   (int wid);   // rebuild manual (add/clear lo llaman ya)
```

---

### Interactivos — toggles

#### ToggleButton

Base de Checkbox y Switch. Texto `○ label` (desmarcado) / `● label` (marcado). Focusable.

```c
int  pw_toggle_create     (const char *label, int checked, int parent_wid);
void pw_toggle_set_checked(int wid, int checked);
int  pw_toggle_get_checked(int wid);
void pw_toggle_set_label  (int wid, const char *label);
void pw_toggle_toggle     (int wid);   // invierte el estado
```

#### Checkbox

`[ ] label` (desmarcado) / `[X] label` (marcado). Comparte infraestructura con ToggleButton.

```c
int  pw_checkbox_create     (const char *label, int checked, int parent_wid);
void pw_checkbox_set_checked(int wid, int checked);
int  pw_checkbox_get_checked(int wid);
void pw_checkbox_set_label  (int wid, const char *label);
void pw_checkbox_toggle     (int wid);
```

#### Switch

Interruptor deslizante. Ancho siempre 4 celdas. `○╺━━` (off) / `━━╸●` (on).

```c
int  pw_switch_create    (int active, int parent_wid);
void pw_switch_set_active(int wid, int active);
int  pw_switch_get_active(int wid);
void pw_switch_toggle    (int wid);
```

#### Button

Botón clickeable con borde CSS. Focusable. Variantes de color opcionales.

```c
int  pw_button_create   (const char *label, const char *variant, int parent_wid);
// variant: NULL/"default" | "primary" | "success" | "warning" | "error"
void pw_button_set_label(int wid, const char *label);
```

---

### Interactivos — selección

#### RadioButton / RadioSet

`◯ label` (desmarcado) / `◉ label` (marcado). Normalmente gestionados por un `RadioSet`.

```c
// RadioButton individual (uso libre)
int  pw_radio_create     (const char *label, int checked, int parent_wid);
void pw_radio_set_checked(int wid, int checked);
int  pw_radio_get_checked(int wid);

// RadioSet — contenedor con exclusión mutua
int  pw_radioset_create      (int parent_wid);
int  pw_radioset_add         (int set_wid, const char *label);  // devuelve wid del radio
void pw_radioset_select      (int set_wid, int index);
int  pw_radioset_get_selected(int set_wid);   // índice del radio activo
int  pw_radioset_count       (int set_wid);
// El primer radio añadido se selecciona automáticamente.
```

#### OptionList

Lista scrollable con cursor `▸`. Un único ítem activo.

```c
int  pw_optionlist_create     (int parent_wid);
int  pw_optionlist_add_option (int wid, const char *text);  // devuelve índice
void pw_optionlist_clear      (int wid);
void pw_optionlist_set_cursor (int wid, int index);
int  pw_optionlist_get_cursor (int wid);
void pw_optionlist_cursor_next(int wid);
void pw_optionlist_cursor_prev(int wid);
int  pw_optionlist_count      (int wid);
// El cursor se sujeta en los extremos (sin wrap).
```

#### ListView

Lista scrollable con cursor `▶`. API idéntica a OptionList.

```c
int  pw_listview_create     (int parent_wid);
int  pw_listview_add_item   (int wid, const char *text);
void pw_listview_clear      (int wid);
void pw_listview_set_cursor (int wid, int index);
int  pw_listview_get_cursor (int wid);
void pw_listview_cursor_next(int wid);
void pw_listview_cursor_prev(int wid);
int  pw_listview_count      (int wid);
```

#### SelectionList

Lista multi-selección. Render: `▸ [X] item` (cursor + seleccionado) / `  [ ] item` (normal).

```c
int  pw_selectionlist_create          (int parent_wid);
int  pw_selectionlist_add_option      (int wid, const char *text, int initially_selected);
void pw_selectionlist_clear           (int wid);
void pw_selectionlist_set_cursor      (int wid, int index);
int  pw_selectionlist_get_cursor      (int wid);
void pw_selectionlist_cursor_next     (int wid);
void pw_selectionlist_cursor_prev     (int wid);
void pw_selectionlist_toggle_selection(int wid, int index);
void pw_selectionlist_set_selected    (int wid, int index, int selected);
int  pw_selectionlist_is_selected     (int wid, int index);
int  pw_selectionlist_count           (int wid);
```

---

### Contenedores y overlays

#### ContentSwitcher

Muestra exactamente un hijo a la vez; oculta el resto. Sin visual propio.

```c
int  pw_switcher_create(int parent_wid);
int  pw_switcher_add   (int switcher_wid, int child_wid);  // devuelve slot index
void pw_switcher_show  (int switcher_wid, int index);
int  pw_switcher_active(int switcher_wid);   // índice visible (-1 si vacío)
int  pw_switcher_count (int switcher_wid);
// El primer hijo añadido es visible; el resto quedan ocultos.
```

#### Toast

Notificación flotante con auto-dismiss. Se ancla al borde inferior del padre.

```c
int  pw_toast_create    (const char *message, int duration_ms, int parent_wid);
void pw_toast_show      (int wid, const char *message, int duration_ms);
void pw_toast_tick      (int wid, int elapsed_ms);
int  pw_toast_is_visible(int wid);   // 0 = ya expiró
```

Uso típico:

```c
int fired = pa_tick_timers(elapsed_ms);
if (fired == toast_timer) pw_toast_tick(toast_wid, elapsed_ms);
```

#### Collapsible

Sección expandible / colapsable con cabecera clicable.

```c
int  pw_collapsible_create      (const char *title, int parent_wid);
int  pw_collapsible_header_wid  (int wid);   // bind click a este wid
int  pw_collapsible_content     (int wid);   // añade hijos aquí
void pw_collapsible_toggle      (int wid);   // expande ↔ colapsa
int  pw_collapsible_is_collapsed(int wid);   // 1 si oculto
```

#### Tabs

Barra de pestañas horizontal. Tab activa: `[Label]`, inactivas: ` Label `.

```c
int  pw_tabs_create         (int parent_wid);
int  pw_tabs_add            (int wid, const char *label);   // devuelve índice
void pw_tabs_register_clicks(int wid);   // llamar tras añadir todas las tabs
int  pw_tabs_handle_click   (int wid, int bid);   // 1 si consumido
void pw_tabs_set_active     (int wid, int index);
int  pw_tabs_get_active     (int wid);
void pw_tabs_next           (int wid);
void pw_tabs_prev           (int wid);
int  pw_tabs_count          (int wid);
```

#### TabbedContent

Barra de tabs + ContentSwitcher combinados.

```c
int  pw_tabbedcontent_create         (int parent_wid);
int  pw_tabbedcontent_add_pane       (int wid, const char *label);   // devuelve wid del pane
void pw_tabbedcontent_register_clicks(int wid);   // llamar tras añadir todos los panes
int  pw_tabbedcontent_handle_click   (int wid, int bid);
void pw_tabbedcontent_set_active     (int wid, int index);
int  pw_tabbedcontent_get_active     (int wid);
```

---

### Input de texto

#### Input

Campo de texto de una línea con cursor `▌`. Tamaño `fr(1) × fixed(1)`. Focusable.

```c
int         pw_input_create       (const char *placeholder, int max_len, int parent_wid);
// max_len: 0 = 255 caracteres
void        pw_input_register_keys(int wid);          // llamar una vez tras create
int         pw_input_handle       (int wid, int bid); // 1 si bid consumido
const char *pw_input_get_value    (int wid);          // puntero a buffer interno; copiar
void        pw_input_set_value    (int wid, const char *text);
int         pw_input_is_submitted (int wid);          // 1 (y resetea) si Enter
void        pw_input_set_password (int wid, int on);  // 1 = mostrar • en lugar de texto
void        pw_input_tick         (int wid, int elapsed_ms); // blink del cursor
```

Teclas soportadas cuando enfocado: caracteres imprimibles, `Backspace`, `Delete`, `← →`, `Home`, `End`, `Enter`, `Ctrl+A`, `Ctrl+V`.

#### TextArea

Editor de texto multi-línea (hasta 64 líneas × 255 caracteres). Focusable.

```c
int         pw_textarea_create       (int parent_wid);
void        pw_textarea_register_keys(int wid);          // llamar una vez tras create
int         pw_textarea_handle       (int wid, int bid); // 1 si bid consumido
const char *pw_textarea_get_text     (int wid);          // líneas separadas por '\n'
void        pw_textarea_set_text     (int wid, const char *text);
void        pw_textarea_tick         (int wid, int elapsed_ms); // blink + foco
```

Teclas soportadas: caracteres imprimibles, `Backspace`, `Delete`, `← → ↑ ↓`, `Home`, `End`, `Enter`, `Ctrl+A`.

---

### Navegación jerárquica

#### Tree

Árbol jerárquico con expand/collapse y cursor navegable. Focusable.

```c
int  pw_tree_create         (int parent_wid);
int  pw_tree_add_node       (int wid, int parent_idx, const char *label, int is_leaf);
// parent_idx = -1 → nodo raíz. Devuelve índice del nodo (0-based) o -1.
void pw_tree_expand         (int wid, int node_idx);
void pw_tree_collapse       (int wid, int node_idx);
void pw_tree_toggle         (int wid);               // nodo bajo el cursor
void pw_tree_cursor_next    (int wid);
void pw_tree_cursor_prev    (int wid);
void pw_tree_set_cursor     (int wid, int visible_idx);
int  pw_tree_get_cursor     (int wid);
int  pw_tree_get_cursor_node(int wid);   // índice de nodo en cursor (-1 si vacío)
int  pw_tree_click_row      (int wid, int mouse_y); // mueve cursor al row clicado
```

---

### Log y texto enriquecido

#### Log / RichLog

Log scrollable. Añade líneas al final; las más antiguas caen al llegar a 128 (ring buffer).

```c
int  pw_log_create    (int parent_wid);   // texto plano
int  pw_richlog_create(int parent_wid);   // strip de tags [rojo], etc. antes de guardar
void pw_log_write     (int wid, const char *line);   // válido para Log y RichLog
void pw_log_clear     (int wid);
int  pw_log_line_count(int wid);   // 0 … 128
```

El scroll con rueda de ratón lo gestiona automáticamente `pw_dispatch_scroll()`.

---

### Datos tabulares

#### DataTable

Tabla con scroll bidireccional, 4 modos de cursor y zebra stripes. Focusable.

```c
// Constantes de tipo de cursor
#define PW_DT_CURSOR_NONE   0   // sin cursor visible
#define PW_DT_CURSOR_CELL   1   // celda individual resaltada
#define PW_DT_CURSOR_ROW    2   // fila entera resaltada (▶ marcador)
#define PW_DT_CURSOR_COLUMN 3   // columna entera resaltada

int  pw_datatable_create          (int parent_wid);
void pw_datatable_register_keys   (int wid);   // bind ↑↓←→ PgUp PgDn Home End Enter
int  pw_datatable_add_column      (int wid, const char *label, int width);
// width=0 → automático. Devuelve índice de columna.
int  pw_datatable_add_row         (int wid);   // devuelve índice de fila
void pw_datatable_set_cell        (int wid, int row, int col, const char *text);
void pw_datatable_clear_rows      (int wid);

// Opciones visuales
void pw_datatable_set_cursor_type (int wid, int cursor_type);   // PW_DT_CURSOR_*
void pw_datatable_set_show_header (int wid, int show);          // 1=mostrar cabecera
void pw_datatable_set_show_cursor (int wid, int show);
void pw_datatable_set_zebra       (int wid, int enable);        // filas alternadas

// Cursor y selección
int  pw_datatable_get_cursor_row  (int wid);
int  pw_datatable_get_cursor_col  (int wid);
void pw_datatable_move_cursor     (int wid, int row, int col);  // con auto-scroll

// Event loop
int  pw_datatable_handle          (int wid, int bid);   // 1 si consumido
int  pw_datatable_is_selected     (int wid);            // 1 (y resetea) si Enter
```

---

### Compuestos

#### Welcome

Widget compuesto: área Markdown + Button en una sola llamada.

```c
int pw_welcome_create    (int parent_wid, const char *markdown_text, const char *button_label);
int pw_welcome_button_wid(int wid);   // wid del Button hijo (para bind click)
```

---

### Dispatch automático

Llama estas funciones con cada `bid` devuelto por `pa_poll()` para que los widgets
gestionen sus propios eventos sin código adicional en la aplicación.

```c
// Clicks en ListView, OptionList, SelectionList, RadioSet
int pw_dispatch_click (int bid);   // 1 si consumido

// Scroll de rueda de ratón en Log y RichLog
int pw_dispatch_scroll(int bid);   // 1 si consumido
```

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
| plague-widgets | Estable — Phases 2-10 completas (35 tipos de widget) |
