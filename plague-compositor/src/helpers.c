#include "helpers.h"
#include <string.h>

PC_WidgetNode pc_node_container(TG_Region region) {
    PC_WidgetNode n;
    memset(&n, 0, sizeof(n));
    n.type    = PC_NODE_CONTAINER;
    n.region  = region;
    n.parent  = -1;
    n.visible = 1;
    return n;
}

PC_WidgetNode pc_node_label(TG_Region region, const char* text, PG_TextStyle style) {
    PC_WidgetNode n;
    memset(&n, 0, sizeof(n));
    n.type       = PC_NODE_LABEL;
    n.region     = region;
    n.text_style = style;
    n.parent     = -1;
    n.visible    = 1;
    if (text) {
        int i = 0;
        while (text[i] && i < 255) { n.text[i] = text[i]; i++; }
        n.text[i] = '\0';
    }
    return n;
}

PC_WidgetNode pc_node_panel(TG_Region region, PG_Color bg) {
    PC_WidgetNode n;
    memset(&n, 0, sizeof(n));
    n.type     = PC_NODE_PANEL;
    n.region   = region;
    n.bg_color = bg;
    n.parent   = -1;
    n.visible  = 1;
    return n;
}
