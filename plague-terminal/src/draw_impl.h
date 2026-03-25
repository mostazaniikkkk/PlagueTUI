#ifndef PT_DRAW_IMPL_H
#define PT_DRAW_IMPL_H

#include "../include/plague_terminal.h"

void pt_draw_init         (void);
void pt_draw_fill_rect    (TG_Region region, PG_Color color);
void pt_draw_stroke_rect  (TG_Region region, PG_Color color, float stroke_width);
void pt_draw_draw_text    (TG_Offset pos, const char* text, PG_TextStyle style);
void pt_draw_clip_push    (TG_Region region);
void pt_draw_clip_pop     (void);
void pt_draw_translate_push(TG_Offset offset);
void pt_draw_translate_pop (void);

#endif
