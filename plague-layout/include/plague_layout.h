#ifndef PLAGUE_LAYOUT_H
#define PLAGUE_LAYOUT_H

#include "../../plague-geometry/include/plague_geometry.h"

#ifdef _WIN32
  #define PL_API __declspec(dllexport)
#else
  #define PL_API __attribute__((visibility("default")))
#endif

// ---------------------------------------------------------------------------
// Tamaño — cómo se especifica una dimensión
// ---------------------------------------------------------------------------

typedef enum {
    PL_SIZE_FIXED    = 0,  /* valor exacto en celdas              */
    PL_SIZE_FRACTION = 1,  /* fracción del espacio disponible     */
} PL_SizeType;

typedef struct {
    PL_SizeType type;
    int         value;  /* celdas (FIXED) o unidades fr (FRACTION) */
} PL_SizeValue;

// ---------------------------------------------------------------------------
// Tipo de layout — cómo se disponen los hijos
// ---------------------------------------------------------------------------

typedef enum {
    PL_LAYOUT_VERTICAL   = 0,
    PL_LAYOUT_HORIZONTAL = 1,
    PL_LAYOUT_DOCK       = 2,
    PL_LAYOUT_GRID       = 3,
} PL_LayoutType;

// ---------------------------------------------------------------------------
// Dock — posición dentro de un padre con layout DOCK
// ---------------------------------------------------------------------------

typedef enum {
    PL_DOCK_TOP    = 0,
    PL_DOCK_BOTTOM = 1,
    PL_DOCK_LEFT   = 2,
    PL_DOCK_RIGHT  = 3,
    PL_DOCK_CENTER = 4,
} PL_DockEdge;

// ---------------------------------------------------------------------------
// Overflow
// ---------------------------------------------------------------------------

typedef enum {
    PL_OVERFLOW_HIDDEN = 0,
    PL_OVERFLOW_SCROLL = 1,
    PL_OVERFLOW_AUTO   = 2,
} PL_Overflow;

// ---------------------------------------------------------------------------
// LayoutNode — nodo de entrada al motor de layout
// ---------------------------------------------------------------------------

#define PL_MAX_NODES    256
#define PL_MAX_CHILDREN 32

typedef struct {
    PL_LayoutType layout;                  /* cómo organiza a sus hijos    */
    PL_SizeValue  width;                   /* restricción de ancho propio  */
    PL_SizeValue  height;                  /* restricción de alto propio   */
    TG_Spacing    margin;                  /* espacio exterior             */
    TG_Spacing    padding;                 /* espacio interior             */
    PL_DockEdge   dock;                    /* posición en padre DOCK       */
    PL_Overflow   overflow;
    int           grid_cols;              /* para GRID: columnas          */
    int           grid_rows;              /* para GRID: filas             */
    int           children[PL_MAX_CHILDREN];
    int           child_count;
    int           parent;
    int           visible;
} PL_LayoutNode;

// ---------------------------------------------------------------------------
// LayoutResult — región calculada para cada nodo
// ---------------------------------------------------------------------------

typedef struct {
    TG_Region region;          /* región exterior (incluye padding) */
    TG_Region content_region;  /* región interior (excluye padding) */
} PL_LayoutResult;

// ---------------------------------------------------------------------------
// Árbol
// ---------------------------------------------------------------------------

PL_API void          pl_tree_init   (void);
PL_API int           pl_tree_add    (PL_LayoutNode node);
PL_API void          pl_tree_set_child(int parent_idx, int child_idx);
PL_API int           pl_tree_count  (void);
PL_API PL_LayoutNode pl_tree_get    (int index);

// ---------------------------------------------------------------------------
// Cómputo
// ---------------------------------------------------------------------------

PL_API void            pl_tree_compute(int root_idx, TG_Region container);
PL_API PL_LayoutResult pl_tree_result (int index);

// ---------------------------------------------------------------------------
// Helpers — constructores de uso frecuente
// ---------------------------------------------------------------------------

PL_API PL_SizeValue  pl_size_fixed   (int cells);
PL_API PL_SizeValue  pl_size_fraction(int fr);

PL_API PL_LayoutNode pl_node_vertical  (PL_SizeValue width, PL_SizeValue height, TG_Spacing padding);
PL_API PL_LayoutNode pl_node_horizontal(PL_SizeValue width, PL_SizeValue height, TG_Spacing padding);
PL_API PL_LayoutNode pl_node_dock_root (TG_Spacing padding);
PL_API PL_LayoutNode pl_node_docked    (PL_DockEdge edge, PL_SizeValue width, PL_SizeValue height);
PL_API PL_LayoutNode pl_node_grid      (PL_SizeValue width, PL_SizeValue height, int cols, int rows);

#endif /* PLAGUE_LAYOUT_H */
