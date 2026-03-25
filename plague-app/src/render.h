#ifndef PA_RENDER_H
#define PA_RENDER_H

#include "../include/plague_app.h"

/* Reconstruye el árbol de layout, computa posiciones y dibuja todos los
 * widgets usando plague-terminal. Finaliza con pt_flush(). */
void pa_do_render(int cols, int rows, int css_handle);

/* Devuelve la región calculada en el último render para el widget dado. */
TG_Region pa_last_region(int wid);

#endif /* PA_RENDER_H */
