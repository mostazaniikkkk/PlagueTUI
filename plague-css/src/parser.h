#ifndef CSS_PARSER_H
#define CSS_PARSER_H

#include "css_types.h"

/*
 * Parsea un string TCSS y rellena `ss`.
 * Devuelve el número de reglas parseadas (>=0), o -1 en error grave.
 */
int css_parse(CSSStylesheet *ss, const char *tcss);

#endif /* CSS_PARSER_H */
