#include "compute.h"
#include "tree.h"

/* ---- Utilidades ---- */

static int clamp0(int v) { return v < 0 ? 0 : v; }

static TG_Region content_of(TG_Region outer, TG_Spacing padding) {
    TG_Region c = {
        outer.x + padding.left,
        outer.y + padding.top,
        clamp0(outer.width  - padding.left - padding.right),
        clamp0(outer.height - padding.top  - padding.bottom),
    };
    return c;
}

/* Resuelve el ancho de un hijo en el eje PERPENDICULAR al stack (siempre estira). */
static int resolve_cross_w(PL_SizeValue sv, int available, int margin_h) {
    if (sv.type == PL_SIZE_FIXED) return sv.value;
    return clamp0(available - margin_h);
}

static int resolve_cross_h(PL_SizeValue sv, int available, int margin_v) {
    if (sv.type == PL_SIZE_FIXED) return sv.value;
    return clamp0(available - margin_v);
}

/* Forward declaration */
static void compute_node(int idx, TG_Region outer);

/* ---- Vertical ---- */

static void layout_vertical(int parent_idx, TG_Region avail) {
    PL_LayoutNode node = pl_tree_internal_get(parent_idx);

    /* Paso 1: suma de alturas fijas y total de fr */
    int fixed_sum = 0, fr_sum = 0;
    for (int i = 0; i < node.child_count; i++) {
        PL_LayoutNode ch = pl_tree_internal_get(node.children[i]);
        if (!ch.visible) continue;
        int mv = ch.margin.top + ch.margin.bottom;
        if (ch.height.type == PL_SIZE_FIXED)
            fixed_sum += ch.height.value + mv;
        else {
            fixed_sum += mv;
            fr_sum += ch.height.value;
        }
    }

    int remaining = clamp0(avail.height - fixed_sum);
    int y = avail.y;

    for (int i = 0; i < node.child_count; i++) {
        int ci = node.children[i];
        PL_LayoutNode ch = pl_tree_internal_get(ci);
        if (!ch.visible) continue;

        int h = (ch.height.type == PL_SIZE_FIXED)
                ? ch.height.value
                : (fr_sum > 0 ? (remaining * ch.height.value) / fr_sum : 0);
        int w = resolve_cross_w(ch.width, avail.width,
                                ch.margin.left + ch.margin.right);

        TG_Region outer = {avail.x + ch.margin.left, y + ch.margin.top, w, h};
        compute_node(ci, outer);
        y += h + ch.margin.top + ch.margin.bottom;
    }
}

/* ---- Horizontal ---- */

static void layout_horizontal(int parent_idx, TG_Region avail) {
    PL_LayoutNode node = pl_tree_internal_get(parent_idx);

    int fixed_sum = 0, fr_sum = 0;
    for (int i = 0; i < node.child_count; i++) {
        PL_LayoutNode ch = pl_tree_internal_get(node.children[i]);
        if (!ch.visible) continue;
        int mh = ch.margin.left + ch.margin.right;
        if (ch.width.type == PL_SIZE_FIXED)
            fixed_sum += ch.width.value + mh;
        else {
            fixed_sum += mh;
            fr_sum += ch.width.value;
        }
    }

    int remaining = clamp0(avail.width - fixed_sum);
    int x = avail.x;

    for (int i = 0; i < node.child_count; i++) {
        int ci = node.children[i];
        PL_LayoutNode ch = pl_tree_internal_get(ci);
        if (!ch.visible) continue;

        int w = (ch.width.type == PL_SIZE_FIXED)
                ? ch.width.value
                : (fr_sum > 0 ? (remaining * ch.width.value) / fr_sum : 0);
        int h = resolve_cross_h(ch.height, avail.height,
                                ch.margin.top + ch.margin.bottom);

        TG_Region outer = {x + ch.margin.left, avail.y + ch.margin.top, w, h};
        compute_node(ci, outer);
        x += w + ch.margin.left + ch.margin.right;
    }
}

/* ---- Dock ---- */

static int dock_dim(PL_SizeValue sv, int fallback) {
    return (sv.type == PL_SIZE_FIXED) ? sv.value : fallback;
}

static void layout_dock(int parent_idx, TG_Region avail) {
    PL_LayoutNode node = pl_tree_internal_get(parent_idx);

    int x0 = avail.x, y0 = avail.y;
    int x1 = avail.x + avail.width;
    int y1 = avail.y + avail.height;

    /* Orden: TOP → BOTTOM → LEFT → RIGHT → CENTER */
    static const PL_DockEdge order[5] = {
        PL_DOCK_TOP, PL_DOCK_BOTTOM, PL_DOCK_LEFT, PL_DOCK_RIGHT, PL_DOCK_CENTER
    };

    for (int pass = 0; pass < 5; pass++) {
        PL_DockEdge edge = order[pass];
        for (int i = 0; i < node.child_count; i++) {
            int ci = node.children[i];
            PL_LayoutNode ch = pl_tree_internal_get(ci);
            if (!ch.visible || ch.dock != edge) continue;

            TG_Region outer;
            int w = x1 - x0, h = y1 - y0;

            switch (edge) {
            case PL_DOCK_TOP:
                h = dock_dim(ch.height, 1);
                outer.x = x0; outer.y = y0; outer.width = w; outer.height = h;
                compute_node(ci, outer);
                y0 += h;
                break;
            case PL_DOCK_BOTTOM:
                h = dock_dim(ch.height, 1);
                outer.x = x0; outer.y = y1 - h; outer.width = w; outer.height = h;
                compute_node(ci, outer);
                y1 -= h;
                break;
            case PL_DOCK_LEFT:
                w = dock_dim(ch.width, 1);
                outer.x = x0; outer.y = y0; outer.width = w; outer.height = h;
                compute_node(ci, outer);
                x0 += w;
                break;
            case PL_DOCK_RIGHT:
                w = dock_dim(ch.width, 1);
                outer.x = x1 - w; outer.y = y0; outer.width = w; outer.height = h;
                compute_node(ci, outer);
                x1 -= w;
                break;
            case PL_DOCK_CENTER:
                outer.x = x0; outer.y = y0;
                outer.width  = clamp0(x1 - x0);
                outer.height = clamp0(y1 - y0);
                compute_node(ci, outer);
                break;
            }
        }
    }
}

/* ---- Grid ---- */

static void layout_grid(int parent_idx, TG_Region avail) {
    PL_LayoutNode node = pl_tree_internal_get(parent_idx);
    int cols = node.grid_cols > 0 ? node.grid_cols : 1;
    int rows = node.grid_rows > 0 ? node.grid_rows : 1;

    int cell_w = avail.width  / cols;
    int cell_h = avail.height / rows;

    for (int i = 0; i < node.child_count; i++) {
        int ci = node.children[i];
        PL_LayoutNode ch = pl_tree_internal_get(ci);
        if (!ch.visible) continue;

        int col = i % cols;
        int row = i / cols;
        if (row >= rows) break;

        TG_Region outer = {
            avail.x + col * cell_w,
            avail.y + row * cell_h,
            cell_w,
            cell_h,
        };
        compute_node(ci, outer);
    }
}

/* ---- Nodo individual ---- */

static void compute_node(int idx, TG_Region outer) {
    PL_LayoutNode node = pl_tree_internal_get(idx);

    TG_Region content = content_of(outer, node.padding);

    PL_LayoutResult result;
    result.region         = outer;
    result.content_region = content;
    pl_tree_internal_set_result(idx, result);

    if (node.child_count == 0) return;

    switch (node.layout) {
    case PL_LAYOUT_VERTICAL:   layout_vertical  (idx, content); break;
    case PL_LAYOUT_HORIZONTAL: layout_horizontal(idx, content); break;
    case PL_LAYOUT_DOCK:       layout_dock      (idx, content); break;
    case PL_LAYOUT_GRID:       layout_grid      (idx, content); break;
    }
}

/* ---- Punto de entrada público ---- */

void pl_compute(int root_idx, TG_Region container) {
    if (root_idx < 0 || root_idx >= pl_tree_internal_count()) return;
    compute_node(root_idx, container);
}
