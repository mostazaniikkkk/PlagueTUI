#include "text_style.h"
#include <string.h>

PG_TextStyle pg_text_style(const char* font_name, float font_size,
                            PG_Color color, int bold, int italic) {
    PG_TextStyle s;
    strncpy(s.font_name, font_name, sizeof(s.font_name) - 1);
    s.font_name[sizeof(s.font_name) - 1] = '\0';
    s.font_size = font_size;
    s.color     = color;
    s.bold      = bold;
    s.italic    = italic;
    return s;
}
