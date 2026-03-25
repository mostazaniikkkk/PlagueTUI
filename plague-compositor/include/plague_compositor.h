#ifndef PLAGUE_COMPOSITOR_H
#define PLAGUE_COMPOSITOR_H

#include "plague_geometry.h"
#include "plague_drawcontext.h"

#ifdef _WIN32
  #define PC_API __declspec(dllexport)
#else
  #define PC_API __attribute__((visibility("default")))
#endif

/* ---- DrawContext vtable ---- */

typedef void (*PC_FillRectFn)     (TG_Region region, PG_Color color);
typedef void (*PC_StrokeRectFn)   (TG_Region region, PG_Color color, float width);
typedef void (*PC_DrawTextFn)     (TG_Offset pos, const char* text, PG_TextStyle style);
typedef void (*PC_ClipPushFn)     (TG_Region region);
typedef void (*PC_ClipPopFn)      (void);
typedef void (*PC_TranslatePushFn)(TG_Offset offset);
typedef void (*PC_TranslatePopFn) (void);

typedef struct {
    PC_FillRectFn      fill_rect;
    PC_StrokeRectFn    stroke_rect;
    PC_DrawTextFn      draw_text;
    PC_ClipPushFn      clip_push;
    PC_ClipPopFn       clip_pop;
    PC_TranslatePushFn translate_push;
    PC_TranslatePopFn  translate_pop;
} PC_DrawContext;

/* ---- WidgetNode ---- */

#define PC_MAX_NODES    256
#define PC_MAX_CHILDREN 32

typedef enum {
    PC_NODE_CONTAINER = 0,
    PC_NODE_LABEL,
    PC_NODE_PANEL,
} PC_NodeType;

typedef struct {
    PC_NodeType  type;
    TG_Region    region;
    PG_Color     bg_color;
    PG_Color     border_color;
    float        border_width;
    char         text[256];
    PG_TextStyle text_style;
    int          children[PC_MAX_CHILDREN];
    int          child_count;
    int          parent;
    int          visible;
} PC_WidgetNode;

/* ---- Árbol ---- */

PC_API void          pc_tree_init    (void);
PC_API int           pc_tree_add     (PC_WidgetNode node);
PC_API void          pc_tree_set_child(int parent_idx, int child_idx);
PC_API int           pc_tree_count   (void);
PC_API PC_WidgetNode pc_tree_get     (int index);

/* ---- Renderizado ---- */

PC_API void pc_tree_render(int root_idx, const PC_DrawContext* ctx);

/* ---- Helpers ---- */

PC_API PC_WidgetNode pc_node_container(TG_Region region);
PC_API PC_WidgetNode pc_node_label    (TG_Region region, const char* text, PG_TextStyle style);
PC_API PC_WidgetNode pc_node_panel    (TG_Region region, PG_Color bg);

#endif
