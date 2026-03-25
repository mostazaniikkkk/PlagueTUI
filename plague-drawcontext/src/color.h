#ifndef PG_COLOR_H
#define PG_COLOR_H

#include <stdint.h>

#ifdef _WIN32
  #define PG_API __declspec(dllexport)
#else
  #define PG_API __attribute__((visibility("default")))
#endif

typedef struct {
    float r, g, b, a;
} PG_Color;

extern const PG_Color PG_COLOR_BLACK;
extern const PG_Color PG_COLOR_WHITE;
extern const PG_Color PG_COLOR_TRANSPARENT;

PG_API PG_Color pg_color_rgba     (float r, float g, float b, float a);
PG_API PG_Color pg_color_from_hex (uint32_t hex);
PG_API PG_Color pg_color_mix      (PG_Color a, PG_Color b, float t);
PG_API PG_Color pg_color_with_alpha(PG_Color c, float a);
PG_API int      pg_color_eq       (PG_Color a, PG_Color b);

#endif
