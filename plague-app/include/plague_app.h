#ifndef PLAGUE_APP_H
#define PLAGUE_APP_H

#include "../../plague-geometry/include/plague_geometry.h"
#include "../../plague-layout/include/plague_layout.h"

#ifdef _WIN32
  #define PA_API __declspec(dllexport)
#else
  #define PA_API __attribute__((visibility("default")))
#endif

// ---------------------------------------------------------------------------
// Ciclo de vida
// ---------------------------------------------------------------------------

/* Inicializa todos los subsistemas (terminal real). */
PA_API int  pa_init(void);

/* Sin I/O real — para tests y CI. */
PA_API int  pa_init_headless(int cols, int rows);

PA_API void pa_shutdown(void);

PA_API int  pa_get_cols(void);
PA_API int  pa_get_rows(void);

// ---------------------------------------------------------------------------
// CSS
// ---------------------------------------------------------------------------

/* Carga una hoja de estilos TCSS. Devuelve handle (>=1) o 0 en error.
 * La app usa el último handle cargado como hoja activa. */
PA_API int pa_css_load(const char *tcss);

// ---------------------------------------------------------------------------
// Estados de widget (bitmask) — iguales a PC_STATE_* de plague-css
// ---------------------------------------------------------------------------

#define PA_STATE_HOVER    0x01
#define PA_STATE_FOCUSED  0x02
#define PA_STATE_PRESSED  0x04
#define PA_STATE_DISABLED 0x08

// ---------------------------------------------------------------------------
// Árbol de widgets
//
// Cada widget tiene:
//   - identidad CSS  (type, id, classes)
//   - propiedades de layout (width, height, layout_type, dock, padding)
//   - contenido  (text)
//   - relación padre/hijo
// ---------------------------------------------------------------------------

#define PA_NO_PARENT  -1
#define PA_MAX_WIDGETS 256

/*
 * Crea un widget y lo añade como hijo de parent_id.
 * Devuelve widget_id (>=0) o -1 en error.
 * El primer widget creado (sin padre) se convierte en la raíz.
 */
PA_API int  pa_widget_add(const char *type, const char *id,
                          const char *classes, int parent_id);

PA_API void pa_widget_set_text      (int wid, const char *text);
PA_API void pa_widget_set_size      (int wid, PL_SizeValue w, PL_SizeValue h);
PA_API void pa_widget_set_dock      (int wid, PL_DockEdge edge);
PA_API void pa_widget_set_layout    (int wid, PL_LayoutType layout_type);
PA_API void pa_widget_set_padding   (int wid, TG_Spacing padding);
PA_API void pa_widget_set_focusable (int wid, int focusable);
PA_API void pa_widget_set_disabled  (int wid, int disabled);
PA_API void pa_widget_set_overlay   (int wid, int overlay);
PA_API void pa_widget_set_visible   (int wid, int visible);

/* Scroll del viewport interno del widget. */
PA_API void pa_widget_scroll_to(int wid, int x, int y);

/* Región calculada en el último pa_render() (útil para tests). */
PA_API TG_Region pa_widget_region(int wid);

// ---------------------------------------------------------------------------
// Render
//
// Recalcula el layout, aplica CSS y dibuja todos los widgets en el buffer.
// Termina haciendo pt_flush().
// ---------------------------------------------------------------------------

PA_API void pa_render(void);

// ---------------------------------------------------------------------------
// Foco
// ---------------------------------------------------------------------------

/* Establece el widget con foco activo. */
PA_API void pa_focus_set(int wid);

/* Devuelve el widget con foco activo (-1 si ninguno). */
PA_API int  pa_focus_get(void);

/* Mueve el foco al siguiente/anterior widget focusable (Tab order). */
PA_API void pa_focus_next(void);
PA_API void pa_focus_prev(void);

// ---------------------------------------------------------------------------
// Eventos y bindings
// ---------------------------------------------------------------------------

/*
 * Registra una combinación de tecla en un widget (y sus ancestros vía bubbling).
 * Devuelve binding_id (>=1) o 0 en error.
 */
PA_API int pa_bind_key(int wid, int key, int mods);

/*
 * Registra un click de mouse en un widget.
 * Devuelve binding_id (>=1) o 0 en error.
 * Cuando el usuario hace click dentro de la región del widget, pa_poll/pa_wait_poll
 * devuelve ese binding_id.
 */
PA_API int pa_bind_click(int wid);

/*
 * Registra un evento de scroll de mouse en un widget.
 * Devuelve binding_id cuando la rueda gira sobre la región del widget.
 * Usa pa_scroll_dy() para obtener la dirección (-1=arriba, +1=abajo).
 */
PA_API int pa_bind_scroll(int wid);
PA_API int pa_scroll_dy(void);

/*
 * Devuelve las coordenadas del último evento de ratón recibido.
 * Útil para determinar qué fila de una lista fue clickeada:
 *   int mx, my; pa_mouse_pos(&mx, &my);
 *   int row = my - pa_widget_region(list_wid).y;
 */
PA_API void pa_mouse_pos(int *x, int *y);

/*
 * Procesa el siguiente evento de entrada (no bloquea).
 * Devuelve:
 *   -1  → pa_quit() fue llamado
 *    0  → sin eventos pendientes
 *   >0  → binding_id del binding que capturó el evento
 */
PA_API int pa_poll(void);

/*
 * Igual que pa_poll pero bloquea hasta que haya un evento.
 */
PA_API int pa_wait_poll(void);

/* Señaliza fin de la aplicación (pa_poll/pa_wait_poll devuelven -1). */
PA_API void pa_quit(void);

// ---------------------------------------------------------------------------
// Timers
// ---------------------------------------------------------------------------

/*
 * Crea un timer. interval_ms = período en ms. repeat = 1 para periódico, 0 para one-shot.
 * Devuelve timer_id (>=1) o 0 en error.
 *
 * Los timers se procesan al llamar pa_tick_timers(). Para apps con timers usa
 * pa_poll() en un loop en lugar de pa_wait_poll() (que bloquea indefinidamente).
 */
PA_API int  pa_timer_create(int interval_ms, int repeat);
PA_API void pa_timer_cancel(int timer_id);

/*
 * Avanza todos los timers elapsed_ms milisegundos.
 * Devuelve timer_id del primer timer que haya expirado, o 0 si ninguno.
 * Llamar repetidamente para drenar todos los timers expirados en un tick.
 */
PA_API int  pa_tick_timers(int elapsed_ms);

#endif /* PLAGUE_APP_H */
