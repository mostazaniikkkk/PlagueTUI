#ifndef PA_FOCUS_H
#define PA_FOCUS_H

/* Inicializa el estado de foco (llamado desde common_init). */
void pa_focus_init(void);

/* Establece/obtiene el widget con foco actual (-1 = sin foco). */
void pa_focus_set_wid(int wid);
int  pa_focus_get_wid(void);

/* Avanza el foco al siguiente/anterior widget focusable (Tab order).
 * direction: +1 = siguiente, -1 = anterior. */
void pa_focus_advance(int direction);

#endif /* PA_FOCUS_H */
