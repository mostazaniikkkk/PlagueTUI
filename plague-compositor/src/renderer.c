#include "renderer.h"
#include "tree.h"

static void render_node(int idx, const PC_DrawContext* ctx) {
    PC_WidgetNode node = pc_tree_get(idx);

    if (!node.visible) return;

    /* 1. Fondo */
    if (node.bg_color.a > 0.0f)
        ctx->fill_rect(node.region, node.bg_color);

    /* 2. Borde */
    if (node.border_width > 0.0f)
        ctx->stroke_rect(node.region, node.border_color, node.border_width);

    /* 3. Texto */
    if (node.text[0] != '\0') {
        TG_Offset pos = { node.region.x, node.region.y };
        ctx->draw_text(pos, node.text, node.text_style);
    }

    /* 4. Hijos (DFS) */
    for (int i = 0; i < node.child_count; i++)
        render_node(node.children[i], ctx);
}

void pc_tree_render(int root_idx, const PC_DrawContext* ctx) {
    if (!ctx) return;
    if (root_idx < 0 || root_idx >= pc_tree_count()) return;
    render_node(root_idx, ctx);
}
