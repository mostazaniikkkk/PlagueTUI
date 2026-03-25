#ifndef PLAGUE_CSS_H
#define PLAGUE_CSS_H

#include <stdint.h>
#include "../../plague-geometry/include/plague_geometry.h"
#include "../../plague-layout/include/plague_layout.h"

#ifdef _WIN32
  #define PC_CSS_API __declspec(dllexport)
#else
  #define PC_CSS_API __attribute__((visibility("default")))
#endif

// ---------------------------------------------------------------------------
// Color RGBA (8 bits por canal)
// ---------------------------------------------------------------------------

typedef struct {
    uint8_t r, g, b, a;
} PC_Color;

// ---------------------------------------------------------------------------
// Estilo de borde
// ---------------------------------------------------------------------------

typedef enum {
    PC_BORDER_NONE    = 0,
    PC_BORDER_SOLID   = 1,
    PC_BORDER_DASHED  = 2,
    PC_BORDER_ROUNDED = 3,
    PC_BORDER_THICK   = 4,
    PC_BORDER_DOUBLE  = 5,
} PC_BorderStyle;

// ---------------------------------------------------------------------------
// Estilo computado — resultado de la cascada CSS aplicada a un widget
// ---------------------------------------------------------------------------

/* Flags para saber qué propiedades fueron explícitamente establecidas */
#define PC_HAS_COLOR        0x0001u
#define PC_HAS_BACKGROUND   0x0002u
#define PC_HAS_BORDER       0x0004u
#define PC_HAS_MARGIN       0x0008u
#define PC_HAS_PADDING      0x0010u
#define PC_HAS_WIDTH        0x0020u
#define PC_HAS_HEIGHT       0x0040u
#define PC_HAS_DOCK         0x0080u
#define PC_HAS_DISPLAY      0x0100u
#define PC_HAS_OVERFLOW     0x0200u
#define PC_HAS_BOLD         0x0400u
#define PC_HAS_ITALIC       0x0800u
#define PC_HAS_UNDERLINE    0x1000u

typedef struct {
    uint32_t       set_flags;
    PC_Color       color;
    PC_Color       background;
    PC_Color       border_color;
    PC_BorderStyle border_style;
    TG_Spacing     margin;
    TG_Spacing     padding;
    PL_SizeValue   width;
    PL_SizeValue   height;
    int            dock;        /* PL_DockEdge, o -1 si no está docked */
    int            display;     /* 1 = block, 0 = none */
    int            overflow;    /* PL_Overflow */
    uint8_t        bold;
    uint8_t        italic;
    uint8_t        underline;
} PC_ComputedStyle;

// ---------------------------------------------------------------------------
// Estado del widget — para pseudo-clases (:hover, :focus, etc.)
// ---------------------------------------------------------------------------

#define PC_STATE_HOVER    0x01
#define PC_STATE_FOCUS    0x02
#define PC_STATE_PRESSED  0x04
#define PC_STATE_DISABLED 0x08

// ---------------------------------------------------------------------------
// API pública
// ---------------------------------------------------------------------------

/* Parsea una hoja de estilos TCSS. Devuelve handle (>=1) o 0 en error. */
PC_CSS_API int  pc_stylesheet_load(const char *tcss);
PC_CSS_API void pc_stylesheet_free(int handle);

/* Número de reglas parseadas (útil para tests). */
PC_CSS_API int  pc_stylesheet_rule_count(int handle);

/*
 * Computa el estilo final para un widget.
 *   type_name : tipo del widget, e.g. "Button"  (cadena vacía = sin tipo)
 *   id        : identificador,   e.g. "my-btn"  (cadena vacía = sin id)
 *   classes   : clases separadas por espacios, e.g. "primary large"
 *   state     : bitmask de PC_STATE_*
 */
PC_CSS_API PC_ComputedStyle pc_compute_style(
    int         ss_handle,
    const char *type_name,
    const char *id,
    const char *classes,
    int         state
);

/* Estilo por defecto (todos los campos en valores neutros). */
PC_CSS_API PC_ComputedStyle pc_default_style(void);

#endif /* PLAGUE_CSS_H */
