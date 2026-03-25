#include "color.h"

const PG_Color PG_COLOR_BLACK       = { 0.0f, 0.0f, 0.0f, 1.0f };
const PG_Color PG_COLOR_WHITE       = { 1.0f, 1.0f, 1.0f, 1.0f };
const PG_Color PG_COLOR_TRANSPARENT = { 0.0f, 0.0f, 0.0f, 0.0f };

PG_Color pg_color_rgba(float r, float g, float b, float a) {
    return (PG_Color){ r, g, b, a };
}

PG_Color pg_color_from_hex(uint32_t hex) {
    return (PG_Color){
        ((hex >> 24) & 0xFF) / 255.0f,
        ((hex >> 16) & 0xFF) / 255.0f,
        ((hex >>  8) & 0xFF) / 255.0f,
        ((hex >>  0) & 0xFF) / 255.0f,
    };
}

PG_Color pg_color_mix(PG_Color a, PG_Color b, float t) {
    return (PG_Color){
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t,
    };
}

PG_Color pg_color_with_alpha(PG_Color c, float a) {
    return (PG_Color){ c.r, c.g, c.b, a };
}

int pg_color_eq(PG_Color a, PG_Color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}
