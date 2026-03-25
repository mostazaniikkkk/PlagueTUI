# plague-compositor

Compositor de widgets para PlagueTUI. Mantiene un árbol de nodos y los recorre en DFS para emitir comandos de dibujo hacia cualquier backend gráfico a través de una vtable (`PC_DrawContext`).

Este módulo **no dibuja nada directamente** — delega cada operación al contexto recibido. En tests se usa el `StubContext` de plague-drawcontext como backend.

**Depende de**: plague-geometry, plague-drawcontext (solo headers — sin linking en tiempo de compilación)

---

## Compilar

```bat
build.bat
```

El DLL queda en `bin/plague_compositor.dll`.

---

## Tipos

### PC_NodeType

```c
typedef enum {
    PC_NODE_CONTAINER = 0,
    PC_NODE_LABEL,
    PC_NODE_PANEL,
} PC_NodeType;
```

### PC_WidgetNode

```c
typedef struct {
    PC_NodeType  type;
    TG_Region    region;         // posición y dimensiones absolutas
    PG_Color     bg_color;       // alpha=0 → sin fondo
    PG_Color     border_color;
    float        border_width;   // 0 → sin borde
    float        border_radius;  // 0 → esquinas rectas
    char         text[256];      // vacío → sin texto
    PG_TextStyle text_style;
    int          children[32];
    int          child_count;
    int          parent;         // -1 si es raíz
    int          visible;        // 0 → nodo y hijos ignorados
} PC_WidgetNode;
```

### PC_DrawContext

Vtable de punteros de función que representa un backend de dibujo:

```c
typedef struct {
    void (*fill_rect)     (TG_Region, PG_Color);
    void (*stroke_rect)   (TG_Region, PG_Color, float);
    void (*draw_text)     (TG_Offset, const char*, PG_TextStyle);
    void (*rounded_rect)  (TG_Region, float, PG_Color);
    void (*draw_shadow)   (TG_Region, float, TG_Offset, PG_Color);
    void (*clip_push)     (TG_Region);
    void (*clip_pop)      (void);
    void (*translate_push)(TG_Offset);
    void (*translate_pop) (void);
} PC_DrawContext;
```

---

## Usar desde C / C++

```c
#include "include/plague_compositor.h"

// Inicializar árbol
pc_tree_init();

// Crear nodos con helpers
PC_WidgetNode panel = pc_node_panel(
    (TG_Region){0, 0, 800, 600},
    pg_color_from_hex(0x1a1a2eff),
    8.0f
);
int panel_idx = pc_tree_add(panel);

PC_WidgetNode label = pc_node_label(
    (TG_Region){10, 10, 200, 30},
    "Hola PlagueTUI",
    pg_text_style("Inter", 14.0f, pg_color_rgba(1,1,1,1), 0, 0)
);
int label_idx = pc_tree_add(label);

pc_tree_set_child(panel_idx, label_idx);

// Renderizar con un DrawContext real (Cairo, Skia, etc.)
PC_DrawContext ctx = { .fill_rect = my_fill_rect, .draw_text = my_draw_text, ... };
pc_tree_render(panel_idx, &ctx);
```

---

## Usar desde Python

```python
import ctypes
from pathlib import Path

comp = ctypes.CDLL("bin/plague_compositor.dll")
dc   = ctypes.CDLL("../plague-drawcontext/bin/plague_drawcontext.dll")

# Definir structs (ver tests/conftest.py para el ejemplo completo)

# Construir vtable apuntando al StubContext
FillRectFn = ctypes.CFUNCTYPE(None, TG_Region, PG_Color)

ctx = PC_DrawContext(
    fill_rect = FillRectFn(dc.pg_stub_fill_rect),
    # ... resto de punteros
)

# Inicializar y poblar el árbol
comp.pc_tree_init()

node = PC_WidgetNode()
node.region   = TG_Region(0, 0, 100, 50)
node.bg_color = PG_Color(1.0, 0.0, 0.0, 1.0)
node.visible  = 1
node.parent   = -1
comp.pc_tree_add(node)

# Renderizar
dc.pg_stub_reset()
comp.pc_tree_render(0, ctypes.byref(ctx))
print(dc.pg_stub_count())  # 1
```

---

## API Completa

### Árbol (singleton global)

| Función | Descripción |
|---------|-------------|
| `pc_tree_init()` | Limpia el árbol |
| `pc_tree_add(node)` | Agrega un nodo, retorna su índice (-1 si lleno) |
| `pc_tree_set_child(parent, child)` | Vincula parent → child |
| `pc_tree_count()` | Número de nodos actuales |
| `pc_tree_get(index)` | Retorna el nodo en esa posición |

Capacidad: **256 nodos**, **32 hijos por nodo**.

### Renderizado

| Función | Descripción |
|---------|-------------|
| `pc_tree_render(root_idx, ctx)` | Renderiza el subárbol desde `root_idx` |

Orden de emisión por nodo: fondo → borde → texto → hijos (DFS).
Nodos con `visible = 0` se saltan completos, incluyendo todos sus hijos.

### Helpers de construcción

| Función | Descripción |
|---------|-------------|
| `pc_node_container(region)` | Contenedor sin apariencia |
| `pc_node_label(region, text, style)` | Nodo con texto |
| `pc_node_panel(region, bg, radius)` | Panel con fondo y radio de esquinas |

---

## Tests

```bat
test.bat
```

Compila el DLL y corre los 31 tests unitarios con `unittest`.
