#ifndef CSS_CASCADE_H
#define CSS_CASCADE_H

#include "css_types.h"

/*
 * Aplica la cascada CSS para obtener el estilo computado de un widget.
 *   type_name : tipo, e.g. "Button"   (cadena vacía = sin tipo)
 *   id        : id,   e.g. "my-btn"  (cadena vacía = sin id)
 *   classes   : clases separadas por espacios, e.g. "primary large"
 *   state     : bitmask de PC_STATE_*
 */
PC_ComputedStyle css_cascade(
    const CSSStylesheet *ss,
    const char          *type_name,
    const char          *id,
    const char          *classes,
    int                  state
);

#endif /* CSS_CASCADE_H */
