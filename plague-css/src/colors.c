#include "colors.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* ---------------------------------------------------------------------------
 * Tabla de colores con nombre (subconjunto útil para TUI)
 * --------------------------------------------------------------------------- */

typedef struct { const char *name; uint8_t r, g, b; } NamedColor;

static const NamedColor NAMED[] = {
    /* básicos */
    { "black",        0,   0,   0 },
    { "white",      255, 255, 255 },
    { "red",        255,   0,   0 },
    { "green",        0, 128,   0 },
    { "blue",         0,   0, 255 },
    { "yellow",     255, 255,   0 },
    { "cyan",         0, 255, 255 },
    { "magenta",    255,   0, 255 },
    /* oscuros */
    { "darkblue",     0,   0, 139 },
    { "darkgreen",    0, 100,   0 },
    { "darkred",    139,   0,   0 },
    { "darkcyan",     0, 139, 139 },
    { "darkmagenta",139,   0, 139 },
    { "darkyellow", 139, 139,   0 },
    /* grises */
    { "gray",       128, 128, 128 },
    { "grey",       128, 128, 128 },
    { "lightgray",  211, 211, 211 },
    { "lightgrey",  211, 211, 211 },
    { "darkgray",   169, 169, 169 },
    { "darkgrey",   169, 169, 169 },
    { "silver",     192, 192, 192 },
    /* otros */
    { "orange",     255, 165,   0 },
    { "pink",       255, 192, 203 },
    { "purple",     128,   0, 128 },
    { "brown",      139,  69,  19 },
    { "lime",         0, 255,   0 },
    { "navy",         0,   0, 128 },
    { "teal",         0, 128, 128 },
    { "maroon",     128,   0,   0 },
    { "olive",      128, 128,   0 },
    { "aqua",         0, 255, 255 },
    { "fuchsia",    255,   0, 255 },
    { "coral",      255, 127,  80 },
    { "salmon",     250, 128, 114 },
    { "gold",       255, 215,   0 },
    { "indigo",      75,   0, 130 },
    { "violet",     238, 130, 238 },
    { "transparent",  0,   0,   0 },   /* a=0 manejado en css_named_color */
};
#define NAMED_COUNT ((int)(sizeof(NAMED)/sizeof(NAMED[0])))

int css_named_color(const char *name, PC_Color *out)
{
    /* comparación case-insensitive */
    for (int i = 0; i < NAMED_COUNT; i++) {
        const char *n = NAMED[i].name;
        int j = 0;
        while (n[j] && tolower((unsigned char)name[j]) == n[j]) j++;
        if (!n[j] && !name[j]) {
            out->r = NAMED[i].r;
            out->g = NAMED[i].g;
            out->b = NAMED[i].b;
            out->a = (strcmp(n, "transparent") == 0) ? 0 : 255;
            return 1;
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * Hex color  "#rrggbb"  o  "#rgb"
 * --------------------------------------------------------------------------- */

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    return -1;
}

int css_hex_color(const char *hex, PC_Color *out)
{
    if (hex[0] != '#') return 0;
    const char *s = hex + 1;
    int len = (int)strlen(s);

    if (len == 6) {
        int r0 = hex_digit(s[0]), r1 = hex_digit(s[1]);
        int g0 = hex_digit(s[2]), g1 = hex_digit(s[3]);
        int b0 = hex_digit(s[4]), b1 = hex_digit(s[5]);
        if (r0<0||r1<0||g0<0||g1<0||b0<0||b1<0) return 0;
        out->r = (uint8_t)(r0*16 + r1);
        out->g = (uint8_t)(g0*16 + g1);
        out->b = (uint8_t)(b0*16 + b1);
        out->a = 255;
        return 1;
    }
    if (len == 3) {
        int r = hex_digit(s[0]), g = hex_digit(s[1]), b = hex_digit(s[2]);
        if (r<0||g<0||b<0) return 0;
        out->r = (uint8_t)(r*17);
        out->g = (uint8_t)(g*17);
        out->b = (uint8_t)(b*17);
        out->a = 255;
        return 1;
    }
    return 0;
}

int css_parse_color(const char *s, PC_Color *out)
{
    if (s[0] == '#') return css_hex_color(s, out);
    return css_named_color(s, out);
}
