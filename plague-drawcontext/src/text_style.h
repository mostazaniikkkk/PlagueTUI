#ifndef PG_TEXT_STYLE_H
#define PG_TEXT_STYLE_H

#include "color.h"

#ifdef _WIN32
  #define PG_API __declspec(dllexport)
#else
  #define PG_API __attribute__((visibility("default")))
#endif

typedef struct {
    char     font_name[64];
    float    font_size;
    PG_Color color;
    int      bold;
    int      italic;
} PG_TextStyle;

PG_API PG_TextStyle pg_text_style(const char* font_name, float font_size,
                                   PG_Color color, int bold, int italic);

#endif
