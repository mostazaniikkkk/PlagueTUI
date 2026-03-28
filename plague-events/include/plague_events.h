#ifndef PLAGUE_EVENTS_H
#define PLAGUE_EVENTS_H

#include <stdint.h>

#ifdef _WIN32
  #define PE_API __declspec(dllexport)
#else
  #define PE_API __attribute__((visibility("default")))
#endif

// ---------------------------------------------------------------------------
// Tipos de evento
// ---------------------------------------------------------------------------

typedef enum {
    PE_EVENT_NONE       = 0,
    PE_EVENT_KEY        = 1,
    PE_EVENT_MOUSE_DOWN = 2,
    PE_EVENT_MOUSE_UP   = 3,
    PE_EVENT_MOUSE_MOVE = 4,
    PE_EVENT_SCROLL     = 5,
    PE_EVENT_RESIZE     = 6,
    PE_EVENT_FOCUS      = 7,
    PE_EVENT_BLUR       = 8,
    PE_EVENT_TIMER      = 9,
} PE_EventType;

// ---------------------------------------------------------------------------
// Teclas especiales  (0x00-0xFF: ASCII/Unicode; 0x100+: teclas especiales)
// ---------------------------------------------------------------------------

#define PE_KEY_ENTER        0x000D
#define PE_KEY_ESCAPE       0x001B
#define PE_KEY_BACKSPACE    0x0008
#define PE_KEY_TAB          0x0009
#define PE_KEY_DELETE       0x007F
#define PE_KEY_UP           0x0100
#define PE_KEY_DOWN         0x0101
#define PE_KEY_LEFT         0x0102
#define PE_KEY_RIGHT        0x0103
#define PE_KEY_HOME         0x0104
#define PE_KEY_END          0x0105
#define PE_KEY_PAGE_UP      0x0106
#define PE_KEY_PAGE_DOWN    0x0107
#define PE_KEY_INSERT       0x0108
#define PE_KEY_F1           0x0110
#define PE_KEY_F2           0x0111
#define PE_KEY_F3           0x0112
#define PE_KEY_F4           0x0113
#define PE_KEY_F5           0x0114
#define PE_KEY_F6           0x0115
#define PE_KEY_F7           0x0116
#define PE_KEY_F8           0x0117
#define PE_KEY_F9           0x0118
#define PE_KEY_F10          0x0119
#define PE_KEY_F11          0x011A
#define PE_KEY_F12          0x011B

// Modificadores de teclado
#define PE_MOD_NONE  0x00
#define PE_MOD_SHIFT 0x01
#define PE_MOD_CTRL  0x02
#define PE_MOD_ALT   0x04
#define PE_MOD_META  0x08

// ---------------------------------------------------------------------------
// Estructuras de datos de evento
// ---------------------------------------------------------------------------

typedef struct {
    int  key;
    int  modifiers;
    char ch[4];     /* carácter UTF-8 si es imprimible */
    int  ch_len;
} PE_KeyData;

typedef struct {
    int x, y;
    int button;     /* 1=izq, 2=medio, 3=der */
    int modifiers;
} PE_MouseData;

typedef struct {
    int dx, dy;
    int x, y;
    int modifiers;
} PE_ScrollData;

typedef struct {
    int cols, rows;
} PE_ResizeData;

typedef struct {
    int node_id;
} PE_FocusData;

typedef struct {
    int timer_id;
} PE_TimerData;

typedef union {
    PE_KeyData    key;
    PE_MouseData  mouse;
    PE_ScrollData scroll;
    PE_ResizeData resize;
    PE_FocusData  focus;
    PE_TimerData  timer;
} PE_EventData;

typedef struct {
    PE_EventType type;
    int          target;     /* node_id del destino original */
    int          cancelled;  /* 1 = detener el bubbling */
    PE_EventData data;
} PE_Event;

// ---------------------------------------------------------------------------
// Cola de eventos — ring buffer FIFO
// ---------------------------------------------------------------------------

PE_API void pe_queue_init (void);
PE_API int  pe_queue_push (PE_Event ev);   /* 1=ok, 0=llena */
PE_API int  pe_queue_pop  (PE_Event *out); /* 1=evento obtenido, 0=vacía */
PE_API int  pe_queue_size (void);
PE_API void pe_queue_clear(void);

// ---------------------------------------------------------------------------
// Árbol de nodos — relaciones padre/hijo para bubbling
// ---------------------------------------------------------------------------

#define PE_NO_PARENT -1
#define PE_MAX_NODES  256

PE_API void pe_tree_init      (void);
PE_API int  pe_tree_add       (int parent_id);           /* devuelve node_id */
PE_API void pe_tree_set_parent(int node_id, int parent_id);
PE_API int  pe_tree_parent    (int node_id);

// ---------------------------------------------------------------------------
// Bindings de teclado
// ---------------------------------------------------------------------------

#define PE_MAX_BINDINGS 512

/* Registra una combinación de tecla+modificadores en un nodo.
 * Devuelve binding_id (>=1) o 0 en error. */
PE_API int  pe_bind_key  (int node_id, int key, int modifiers);
PE_API void pe_unbind    (int binding_id);
PE_API void pe_unbind_all(int node_id);   /* elimina todos los bindings del nodo */

/* Despacha un evento de teclado subiendo desde target_node.
 * Devuelve el binding_id que lo capturó, o 0 si ninguno. */
PE_API int  pe_dispatch_key(int target_node, int key, int modifiers);

// ---------------------------------------------------------------------------
// Foco
// ---------------------------------------------------------------------------

PE_API void pe_focus_set(int node_id);
PE_API int  pe_focus_get(void);   /* devuelve -1 si no hay foco */

// ---------------------------------------------------------------------------
// Timers
// ---------------------------------------------------------------------------

#define PE_MAX_TIMERS 32

/* Crea un timer que dispara cada interval_ms.
 * repeat=1: se repite indefinidamente; repeat=0: one-shot.
 * Devuelve timer_id (>=1) o 0 en error. */
PE_API int  pe_timer_create(int interval_ms, int repeat);
PE_API void pe_timer_cancel(int timer_id);
PE_API int  pe_timer_active(int timer_id);

/* Avanza todos los timers activos. Empuja PE_EVENT_TIMER en la cola
 * por cada timer que haya expirado. */
PE_API void pe_timer_tick(int elapsed_ms);

// ---------------------------------------------------------------------------
// Reset global — reinicia todo el estado del módulo (útil en tests)
// ---------------------------------------------------------------------------

PE_API void pe_reset_all(void);

#endif /* PLAGUE_EVENTS_H */
