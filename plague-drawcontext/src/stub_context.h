#ifndef PG_STUB_CONTEXT_H
#define PG_STUB_CONTEXT_H

#include "draw_command.h"

#ifdef _WIN32
  #define PG_API __declspec(dllexport)
#else
  #define PG_API __attribute__((visibility("default")))
#endif

#define PG_STUB_MAX_COMMANDS 256

PG_API void           pg_stub_reset          (void);
PG_API int            pg_stub_count          (void);
PG_API PG_DrawCommand pg_stub_get            (int index);

PG_API void pg_stub_fill_rect      (TG_Region region, PG_Color color);
PG_API void pg_stub_stroke_rect    (TG_Region region, PG_Color color, float stroke_width);
PG_API void pg_stub_draw_text      (TG_Offset pos, const char* text, PG_TextStyle style);
PG_API void pg_stub_clip_push      (TG_Region region);
PG_API void pg_stub_clip_pop       (void);
PG_API void pg_stub_translate_push (TG_Offset offset);
PG_API void pg_stub_translate_pop  (void);

#endif
