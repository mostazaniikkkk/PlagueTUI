#include "../include/plague_layout.h"
#include <string.h>

PL_SizeValue pl_size_fixed(int cells) {
    PL_SizeValue s = {PL_SIZE_FIXED, cells};
    return s;
}

PL_SizeValue pl_size_fraction(int fr) {
    PL_SizeValue s = {PL_SIZE_FRACTION, fr > 0 ? fr : 1};
    return s;
}

static PL_LayoutNode base_node(void) {
    PL_LayoutNode n;
    memset(&n, 0, sizeof(n));
    n.parent  = -1;
    n.visible = 1;
    n.dock    = PL_DOCK_CENTER;
    return n;
}

PL_LayoutNode pl_node_vertical(PL_SizeValue width, PL_SizeValue height, TG_Spacing padding) {
    PL_LayoutNode n = base_node();
    n.layout  = PL_LAYOUT_VERTICAL;
    n.width   = width;
    n.height  = height;
    n.padding = padding;
    return n;
}

PL_LayoutNode pl_node_horizontal(PL_SizeValue width, PL_SizeValue height, TG_Spacing padding) {
    PL_LayoutNode n = base_node();
    n.layout  = PL_LAYOUT_HORIZONTAL;
    n.width   = width;
    n.height  = height;
    n.padding = padding;
    return n;
}

PL_LayoutNode pl_node_dock_root(TG_Spacing padding) {
    PL_LayoutNode n = base_node();
    n.layout  = PL_LAYOUT_DOCK;
    n.width   = pl_size_fraction(1);
    n.height  = pl_size_fraction(1);
    n.padding = padding;
    return n;
}

PL_LayoutNode pl_node_docked(PL_DockEdge edge, PL_SizeValue width, PL_SizeValue height) {
    PL_LayoutNode n = base_node();
    n.layout = PL_LAYOUT_VERTICAL;  /* layout interno por defecto */
    n.dock   = edge;
    n.width  = width;
    n.height = height;
    return n;
}

PL_LayoutNode pl_node_grid(PL_SizeValue width, PL_SizeValue height, int cols, int rows) {
    PL_LayoutNode n = base_node();
    n.layout    = PL_LAYOUT_GRID;
    n.width     = width;
    n.height    = height;
    n.grid_cols = cols;
    n.grid_rows = rows;
    return n;
}
