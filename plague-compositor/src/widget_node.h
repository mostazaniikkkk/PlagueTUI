#ifndef PC_WIDGET_NODE_H
#define PC_WIDGET_NODE_H

#include "plague_geometry.h"
#include "plague_drawcontext.h"

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

#endif
