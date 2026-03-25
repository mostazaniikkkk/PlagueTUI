# plague-layout

Motor de layout para PlagueTUI. Calcula la región (`TG_Region`) de cada nodo del árbol de widgets a partir de restricciones de tamaño, padding, margin y tipo de layout.

## Responsabilidad

Dado un árbol de `PL_LayoutNode` y un contenedor raíz, calcula la posición y dimensiones en celdas de terminal de cada nodo.

## API pública

```c
// Árbol
void          pl_tree_init(void);
int           pl_tree_add(PL_LayoutNode node);          // devuelve index
void          pl_tree_set_child(int parent, int child);
int           pl_tree_count(void);
PL_LayoutNode pl_tree_get(int index);

// Cómputo
void            pl_tree_compute(int root_idx, TG_Region container);
PL_LayoutResult pl_tree_result(int index);

// Helpers — constructores
PL_SizeValue  pl_size_fixed(int cells);
PL_SizeValue  pl_size_fraction(int fr);
PL_LayoutNode pl_node_vertical  (PL_SizeValue w, PL_SizeValue h, TG_Spacing padding);
PL_LayoutNode pl_node_horizontal(PL_SizeValue w, PL_SizeValue h, TG_Spacing padding);
PL_LayoutNode pl_node_dock_root (TG_Spacing padding);
PL_LayoutNode pl_node_docked    (PL_DockEdge edge, PL_SizeValue w, PL_SizeValue h);
PL_LayoutNode pl_node_grid      (PL_SizeValue w, PL_SizeValue h, int cols, int rows);
```

## Tipos de layout

| Tipo | Constante | Descripción |
|------|-----------|-------------|
| Vertical | `PL_LAYOUT_VERTICAL` | Hijos apilados verticalmente |
| Horizontal | `PL_LAYOUT_HORIZONTAL` | Hijos en fila |
| Dock | `PL_LAYOUT_DOCK` | Hijos anclados a bordes (top/bottom/left/right/center) |
| Grid | `PL_LAYOUT_GRID` | Cuadrícula N×M |

## Tamaños (`PL_SizeValue`)

```c
pl_size_fixed(20)    // exactamente 20 celdas
pl_size_fraction(1)  // 1fr — fracción del espacio disponible
pl_size_fraction(2)  // 2fr — el doble que 1fr
```

En la dimensión **no-apilada**, `fr` significa "ocupa todo el espacio disponible". En la dimensión apilada, las fracciones se reparten proporcionalmente entre hijos.

## Resultado

```c
typedef struct {
    TG_Region region;         // región exterior (incluye padding)
    TG_Region content_region; // región interior (excluye padding)
} PL_LayoutResult;
```

## Dependencias

- `plague-geometry` — `TG_Region`, `TG_Spacing`

## Compilar y testear

```bat
build.bat   # → bin\plague_layout.dll
test.bat    # compila + ejecuta tests con unittest
```

## Tests

| Archivo | Cubre |
|---------|-------|
| `test_vertical.py` | Layout vertical, fr weights, padding, margin |
| `test_horizontal.py` | Layout horizontal, sidebar+main, fr weights |
| `test_dock.py` | Header/footer, sidebar, layouts combinados |
| `test_grid.py` | Grid 2×2, 3×1, 1×3, padding, overflow de hijos |
