#ifndef PC_DRAW_CONTEXT_H
#define PC_DRAW_CONTEXT_H

#include "plague_geometry.h"
#include "plague_drawcontext.h"

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

#endif
