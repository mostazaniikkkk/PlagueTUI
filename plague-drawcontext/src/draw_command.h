#ifndef PG_DRAW_COMMAND_H
#define PG_DRAW_COMMAND_H

#include "color.h"
#include "text_style.h"
#include "../../plague-geometry/include/plague_geometry.h"

typedef enum {
    PG_CMD_FILL_RECT,
    PG_CMD_STROKE_RECT,
    PG_CMD_DRAW_TEXT,
    PG_CMD_CLIP_PUSH,
    PG_CMD_CLIP_POP,
    PG_CMD_TRANSLATE_PUSH,
    PG_CMD_TRANSLATE_POP,
} PG_DrawCommandType;

typedef struct { TG_Region region; PG_Color color;                 } PG_FillRectData;
typedef struct { TG_Region region; PG_Color color; float stroke_width; } PG_StrokeRectData;
typedef struct { TG_Offset pos;    char text[256]; PG_TextStyle style; } PG_DrawTextData;
typedef struct { TG_Region region;                                 } PG_ClipPushData;
typedef struct { TG_Offset offset;                                 } PG_TranslatePushData;

typedef struct {
    PG_DrawCommandType type;
    union {
        PG_FillRectData      fill_rect;
        PG_StrokeRectData    stroke_rect;
        PG_DrawTextData      draw_text;
        PG_ClipPushData      clip_push;
        PG_TranslatePushData translate_push;
    };
} PG_DrawCommand;

#endif
