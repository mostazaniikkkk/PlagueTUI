#ifndef PC_HELPERS_H
#define PC_HELPERS_H

#include "widget_node.h"

PC_WidgetNode pc_node_container(TG_Region region);
PC_WidgetNode pc_node_label    (TG_Region region, const char* text, PG_TextStyle style);
PC_WidgetNode pc_node_panel    (TG_Region region, PG_Color bg);

#endif
