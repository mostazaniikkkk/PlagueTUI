#ifndef PLAGUE_GEOMETRY_H
#define PLAGUE_GEOMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// ---------------------------------------------------------------------------
// Offset — posición o desplazamiento 2D en celdas de terminal
// ---------------------------------------------------------------------------

typedef struct {
    int x;
    int y;
} TG_Offset;

TG_Offset tg_offset_add  (TG_Offset a, TG_Offset b);
TG_Offset tg_offset_sub  (TG_Offset a, TG_Offset b);
TG_Offset tg_offset_scale(TG_Offset a, int factor);
bool      tg_offset_eq   (TG_Offset a, TG_Offset b);

// ---------------------------------------------------------------------------
// Size — dimensiones 2D en celdas de terminal
// ---------------------------------------------------------------------------

typedef struct {
    int width;
    int height;
} TG_Size;

TG_Size tg_size_add  (TG_Size a, TG_Size b);
TG_Size tg_size_scale(TG_Size a, int factor);
int     tg_size_area (TG_Size a);
bool    tg_size_eq   (TG_Size a, TG_Size b);

// ---------------------------------------------------------------------------
// Spacing — márgenes o padding en celdas de terminal
// ---------------------------------------------------------------------------

typedef struct {
    int top;
    int right;
    int bottom;
    int left;
} TG_Spacing;

TG_Spacing tg_spacing_uniform(int value);
TG_Spacing tg_spacing_add    (TG_Spacing a, TG_Spacing b);
bool       tg_spacing_eq     (TG_Spacing a, TG_Spacing b);

// ---------------------------------------------------------------------------
// Region — rectángulo posicionado en celdas de terminal
// ---------------------------------------------------------------------------

typedef struct {
    int x;
    int y;
    int width;
    int height;
} TG_Region;

bool      tg_region_contains (TG_Region r, TG_Offset o);
TG_Region tg_region_clip     (TG_Region a, TG_Region b);
TG_Region tg_region_union    (TG_Region a, TG_Region b);
TG_Region tg_region_translate(TG_Region r, TG_Offset o);
TG_Region tg_region_inflate  (TG_Region r, TG_Spacing s);
TG_Region tg_region_deflate  (TG_Region r, TG_Spacing s);
bool      tg_region_is_empty (TG_Region r);
TG_Offset tg_region_center   (TG_Region r);
TG_Size   tg_region_size     (TG_Region r);
bool      tg_region_eq       (TG_Region a, TG_Region b);

#ifdef __cplusplus
}
#endif

#endif // PLAGUE_GEOMETRY_H
