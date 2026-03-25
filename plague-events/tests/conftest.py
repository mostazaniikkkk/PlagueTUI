"""
Carga compartida del DLL para los tests de plague-events.
"""

import ctypes
import sys
from pathlib import Path

root = Path(__file__).parent.parent

if sys.platform == "win32":
    lib_path = root / "bin" / "plague_events.dll"
elif sys.platform == "darwin":
    lib_path = root / "bin" / "libplague_events.dylib"
else:
    lib_path = root / "bin" / "libplague_events.so"

lib = ctypes.CDLL(str(lib_path))

# ---------------------------------------------------------------------------
# Tipos de evento
# ---------------------------------------------------------------------------

PE_EVENT_NONE       = 0
PE_EVENT_KEY        = 1
PE_EVENT_MOUSE_DOWN = 2
PE_EVENT_MOUSE_UP   = 3
PE_EVENT_MOUSE_MOVE = 4
PE_EVENT_SCROLL     = 5
PE_EVENT_RESIZE     = 6
PE_EVENT_FOCUS      = 7
PE_EVENT_BLUR       = 8
PE_EVENT_TIMER      = 9

# ---------------------------------------------------------------------------
# Teclas especiales
# ---------------------------------------------------------------------------

PE_KEY_ENTER     = 0x000D
PE_KEY_ESCAPE    = 0x001B
PE_KEY_BACKSPACE = 0x0008
PE_KEY_TAB       = 0x0009
PE_KEY_DELETE    = 0x007F
PE_KEY_UP        = 0x0100
PE_KEY_DOWN      = 0x0101
PE_KEY_LEFT      = 0x0102
PE_KEY_RIGHT     = 0x0103
PE_KEY_HOME      = 0x0104
PE_KEY_END       = 0x0105
PE_KEY_PAGE_UP   = 0x0106
PE_KEY_PAGE_DOWN = 0x0107
PE_KEY_F1        = 0x0110
PE_KEY_F5        = 0x0114
PE_KEY_F12       = 0x011B

PE_MOD_NONE  = 0x00
PE_MOD_SHIFT = 0x01
PE_MOD_CTRL  = 0x02
PE_MOD_ALT   = 0x04

PE_NO_PARENT = -1

# ---------------------------------------------------------------------------
# Estructuras de datos de evento
# ---------------------------------------------------------------------------

class PE_KeyData(ctypes.Structure):
    _fields_ = [
        ("key",       ctypes.c_int),
        ("modifiers", ctypes.c_int),
        ("ch",        ctypes.c_char * 4),
        ("ch_len",    ctypes.c_int),
    ]

class PE_MouseData(ctypes.Structure):
    _fields_ = [
        ("x",         ctypes.c_int),
        ("y",         ctypes.c_int),
        ("button",    ctypes.c_int),
        ("modifiers", ctypes.c_int),
    ]

class PE_ScrollData(ctypes.Structure):
    _fields_ = [
        ("dx",        ctypes.c_int),
        ("dy",        ctypes.c_int),
        ("x",         ctypes.c_int),
        ("y",         ctypes.c_int),
        ("modifiers", ctypes.c_int),
    ]

class PE_ResizeData(ctypes.Structure):
    _fields_ = [
        ("cols", ctypes.c_int),
        ("rows", ctypes.c_int),
    ]

class PE_FocusData(ctypes.Structure):
    _fields_ = [("node_id", ctypes.c_int)]

class PE_TimerData(ctypes.Structure):
    _fields_ = [("timer_id", ctypes.c_int)]

class PE_EventData(ctypes.Union):
    _fields_ = [
        ("key",    PE_KeyData),
        ("mouse",  PE_MouseData),
        ("scroll", PE_ScrollData),
        ("resize", PE_ResizeData),
        ("focus",  PE_FocusData),
        ("timer",  PE_TimerData),
    ]

class PE_Event(ctypes.Structure):
    _fields_ = [
        ("type",      ctypes.c_int),
        ("target",    ctypes.c_int),
        ("cancelled", ctypes.c_int),
        ("data",      PE_EventData),
    ]

# ---------------------------------------------------------------------------
# Firmas de funciones — Cola
# ---------------------------------------------------------------------------

lib.pe_queue_init.argtypes  = []
lib.pe_queue_init.restype   = None

lib.pe_queue_push.argtypes  = [PE_Event]
lib.pe_queue_push.restype   = ctypes.c_int

lib.pe_queue_pop.argtypes   = [ctypes.POINTER(PE_Event)]
lib.pe_queue_pop.restype    = ctypes.c_int

lib.pe_queue_size.argtypes  = []
lib.pe_queue_size.restype   = ctypes.c_int

lib.pe_queue_clear.argtypes = []
lib.pe_queue_clear.restype  = None

# ---------------------------------------------------------------------------
# Firmas de funciones — Árbol
# ---------------------------------------------------------------------------

lib.pe_tree_init.argtypes       = []
lib.pe_tree_init.restype        = None

lib.pe_tree_add.argtypes        = [ctypes.c_int]
lib.pe_tree_add.restype         = ctypes.c_int

lib.pe_tree_set_parent.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pe_tree_set_parent.restype  = None

lib.pe_tree_parent.argtypes     = [ctypes.c_int]
lib.pe_tree_parent.restype      = ctypes.c_int

# ---------------------------------------------------------------------------
# Firmas de funciones — Bindings
# ---------------------------------------------------------------------------

lib.pe_bind_key.argtypes    = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
lib.pe_bind_key.restype     = ctypes.c_int

lib.pe_unbind.argtypes      = [ctypes.c_int]
lib.pe_unbind.restype       = None

lib.pe_unbind_all.argtypes  = [ctypes.c_int]
lib.pe_unbind_all.restype   = None

lib.pe_dispatch_key.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
lib.pe_dispatch_key.restype  = ctypes.c_int

# ---------------------------------------------------------------------------
# Firmas de funciones — Foco
# ---------------------------------------------------------------------------

lib.pe_focus_set.argtypes = [ctypes.c_int]
lib.pe_focus_set.restype  = None

lib.pe_focus_get.argtypes = []
lib.pe_focus_get.restype  = ctypes.c_int

# ---------------------------------------------------------------------------
# Firmas de funciones — Timers
# ---------------------------------------------------------------------------

lib.pe_timer_create.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pe_timer_create.restype  = ctypes.c_int

lib.pe_timer_cancel.argtypes = [ctypes.c_int]
lib.pe_timer_cancel.restype  = None

lib.pe_timer_active.argtypes = [ctypes.c_int]
lib.pe_timer_active.restype  = ctypes.c_int

lib.pe_timer_tick.argtypes   = [ctypes.c_int]
lib.pe_timer_tick.restype    = None

lib.pe_reset_all.argtypes = []
lib.pe_reset_all.restype  = None

# ---------------------------------------------------------------------------
# Helpers Python
# ---------------------------------------------------------------------------

def make_key_event(key, modifiers=PE_MOD_NONE, target=0):
    ev = PE_Event()
    ev.type           = PE_EVENT_KEY
    ev.target         = target
    ev.cancelled      = 0
    ev.data.key.key   = key
    ev.data.key.modifiers = modifiers
    return ev

def make_resize_event(cols, rows):
    ev = PE_Event()
    ev.type             = PE_EVENT_RESIZE
    ev.data.resize.cols = cols
    ev.data.resize.rows = rows
    return ev

def pop_event():
    ev = PE_Event()
    got = lib.pe_queue_pop(ctypes.byref(ev))
    return (got, ev)
