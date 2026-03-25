# plague-events

Sistema de eventos para PlagueTUI. Gestiona una cola FIFO de eventos, un árbol de nodos para bubbling, bindings de teclado con despacho por el árbol, foco, y timers.

## Responsabilidad

Desacopla la generación de eventos (input de terminal, timers) de su procesamiento (widgets). Los eventos suben por el árbol de nodos hasta que un binding los captura.

## API pública

### Cola de eventos

```c
void pe_queue_init(void);
int  pe_queue_push(PE_Event ev);   // 1=ok, 0=llena
int  pe_queue_pop(PE_Event *out);  // 1=obtenido, 0=vacía
int  pe_queue_size(void);
void pe_queue_clear(void);
```

### Árbol de nodos (bubbling)

```c
void pe_tree_init(void);
int  pe_tree_add(int parent_id);           // devuelve node_id
void pe_tree_set_parent(int id, int parent);
int  pe_tree_parent(int node_id);          // PE_NO_PARENT si es raíz
```

### Bindings de teclado

```c
int  pe_bind_key(int node_id, int key, int modifiers); // devuelve binding_id
void pe_unbind(int binding_id);
void pe_unbind_all(int node_id);

// Sube desde target_node buscando un binding que coincida.
// Devuelve binding_id o 0 si ninguno lo captura.
int pe_dispatch_key(int target_node, int key, int modifiers);
```

### Foco

```c
void pe_focus_set(int node_id);
int  pe_focus_get(void);   // -1 si no hay foco
```

### Timers

```c
int  pe_timer_create(int interval_ms, int repeat);  // devuelve timer_id
void pe_timer_cancel(int timer_id);
int  pe_timer_active(int timer_id);
void pe_timer_tick(int elapsed_ms);  // avanza timers; empuja PE_EVENT_TIMER si expiran
```

### Reset

```c
void pe_reset_all(void);  // reinicia toda la state global (útil en tests)
```

## Tipos de evento

```c
typedef enum {
    PE_EVENT_NONE, PE_EVENT_KEY, PE_EVENT_MOUSE_DOWN, PE_EVENT_MOUSE_UP,
    PE_EVENT_MOUSE_MOVE, PE_EVENT_SCROLL, PE_EVENT_RESIZE,
    PE_EVENT_FOCUS, PE_EVENT_BLUR, PE_EVENT_TIMER,
} PE_EventType;

typedef struct {
    PE_EventType type;
    int          target;     // node_id del destino original
    int          cancelled;
    union {
        PE_KeyData    key;
        PE_MouseData  mouse;
        PE_ScrollData scroll;
        PE_ResizeData resize;
        PE_FocusData  focus;
        PE_TimerData  timer;
    } data;
} PE_Event;
```

## Teclas especiales

```c
PE_KEY_ENTER, PE_KEY_ESCAPE, PE_KEY_BACKSPACE, PE_KEY_TAB, PE_KEY_DELETE
PE_KEY_UP, PE_KEY_DOWN, PE_KEY_LEFT, PE_KEY_RIGHT
PE_KEY_HOME, PE_KEY_END, PE_KEY_PAGE_UP, PE_KEY_PAGE_DOWN
PE_KEY_F1 … PE_KEY_F12
```

Modificadores: `PE_MOD_SHIFT`, `PE_MOD_CTRL`, `PE_MOD_ALT`, `PE_MOD_META`

## Bubbling

Al llamar `pe_dispatch_key(target, key, mods)`:
1. Busca en `target` un binding que coincida con `(key, mods)`.
2. Si no hay, sube al padre (`pe_tree_parent`).
3. Repite hasta la raíz o hasta encontrar un binding.
4. Devuelve `binding_id` si encontró, `0` si ninguno capturó.

## Dependencias

Ninguna (standalone — no depende de otros módulos de PlagueTUI).

## Compilar y testear

```bat
build.bat   # → bin\plague_events.dll
test.bat    # compila + ejecuta tests con unittest
```

## Tests

| Archivo | Cubre |
|---------|-------|
| `test_queue.py` | Push/pop, FIFO, capacidad (256), wrap-around del ring buffer |
| `test_bindings.py` | Árbol de nodos, bind/unbind, dispatch directo, bubbling, foco |
| `test_timers.py` | One-shot, repeat, acumulación de ticks, múltiples timers |
