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

PA_API void pa_widget_set_text   (int wid, const char *text);
PA_API void pa_widget_set_size   (int wid, PL_SizeValue w, PL_SizeValue h);
PA_API void pa_widget_set_dock   (int wid, PL_DockEdge edge);
PA_API void pa_widget_set_layout (int wid, PL_LayoutType layout_type);
PA_API void pa_widget_set_padding(int wid, TG_Spacing padding);

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
// Eventos y bindings
// ---------------------------------------------------------------------------

/*
 * Registra una combinación de tecla en un widget (y sus ancestros vía bubbling).
 * Devuelve binding_id (>=1) o 0 en error.
 */
PA_API int pa_bind_key(int wid, int key, int mods);

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

#endif /* PLAGUE_APP_H */
