# plague-app

DLL orquestadora de PlagueTUI. Une terminal + layout + CSS + events en una API de alto nivel para construir aplicaciones TUI.

## Responsabilidad

Coordina todos los módulos del stack:

```
pa_render()
    │
    ├── plague-layout  → calcula TG_Region por widget
    ├── plague-css     → calcula PC_ComputedStyle por widget
    └── plague-terminal → pt_fill_rect / pt_stroke_rect / pt_draw_text / pt_flush

pa_poll() / pa_wait_poll()
    │
    ├── plague-terminal → pt_poll_event / pt_wait_event
    └── plague-events   → pe_dispatch_key (bubbling por árbol de nodos)
```

## API pública

```c
// Ciclo de vida
int  pa_init(void);                        // terminal real
int  pa_init_headless(int cols, int rows); // sin I/O (tests)
void pa_shutdown(void);
int  pa_get_cols(void);
int  pa_get_rows(void);

// CSS
int pa_css_load(const char *tcss);   // devuelve handle; reemplaza la hoja activa

// Widgets
int  pa_widget_add(const char *type, const char *id,
                   const char *classes, int parent_id);
void pa_widget_set_text   (int wid, const char *text);
void pa_widget_set_size   (int wid, PL_SizeValue w, PL_SizeValue h);
void pa_widget_set_dock   (int wid, PL_DockEdge edge);
void pa_widget_set_layout (int wid, PL_LayoutType layout_type);
void pa_widget_set_padding(int wid, TG_Spacing padding);
TG_Region pa_widget_region(int wid);   // región calculada en el último render

// Render
void pa_render(void);   // layout + CSS + draw + flush

// Eventos
int  pa_bind_key(int wid, int key, int mods);  // devuelve binding_id
int  pa_poll(void);       // no bloquea; -1=quit, 0=sin evento, >0=binding_id
int  pa_wait_poll(void);  // bloquea hasta evento
void pa_quit(void);
```

## Uso típico

```python
lib.pa_init()
lib.pa_css_load(TCSS.encode())

root    = lib.pa_widget_add(b"Screen",  b"", b"", -1)
lib.pa_widget_set_layout(root, PL_LAYOUT_DOCK)

header  = lib.pa_widget_add(b"Header",  b"header", b"", root)
lib.pa_widget_set_dock(header, PL_DOCK_TOP)
lib.pa_widget_set_size(header, fr(1), fixed(3))
lib.pa_widget_set_text(header, b"PlagueTUI Demo")

# ... más widgets ...

quit_bid = lib.pa_bind_key(root, ord('q'), 0)

lib.pa_render()
while True:
    result = lib.pa_wait_poll()
    if result == quit_bid or result < 0:
        break

lib.pa_shutdown()
```

## Render pass

Por cada widget en orden de creación (padres antes que hijos):
1. Construye el nodo de plague-layout a partir de las propiedades del widget.
2. Llama `pl_tree_compute` para obtener `TG_Region`.
3. Aplica `pc_compute_style` para obtener colores, borde y estilo de texto.
4. Dibuja fondo (`pt_fill_rect`), borde (`pt_stroke_rect`) y texto (`pt_draw_text`).
5. Llama `pt_flush` al final del frame.

## Dependencias

| Módulo | Para qué |
|--------|----------|
| `plague-geometry` | Tipos `TG_Region`, `TG_Spacing` |
| `plague-layout` | Cálculo de posiciones |
| `plague-terminal` | Dibujo y lectura de eventos |
| `plague-css` | Estilos computados por widget |
| `plague-events` | Bubbling de teclas y timers |

## Compilar y testear

```bat
build.bat   # compila plague_app.dll y copia deps a bin\
test.bat    # compila + ejecuta tests con unittest
```

El `build.bat` copia automáticamente las DLLs dependientes a `bin\` para que el loader de Windows las encuentre.

## Tests

`tests/test_app.py` cubre: init headless, tamaño de pantalla, widgets, layout dock completo (header+footer+sidebar+main), CSS, bindings y `pa_quit`.
