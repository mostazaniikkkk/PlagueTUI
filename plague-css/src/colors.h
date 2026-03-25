#ifndef CSS_COLORS_H
#define CSS_COLORS_H

#include "../include/plague_css.h"

/* Resuelve un nombre de color a PC_Color. Devuelve 1 si encontrado, 0 si no. */
int css_named_color(const char *name, PC_Color *out);

/* Parsea "#rrggbb" o "#rgb". Devuelve 1 si ok, 0 si no. */
int css_hex_color(const char *hex, PC_Color *out);

/* Parsea cualquier expresión de color (hex o nombre). Devuelve 1 si ok. */
int css_parse_color(const char *s, PC_Color *out);

#endif /* CSS_COLORS_H */
