"""
Carga compartida del DLL para todos los tests de plague-terminal.
Importar: from conftest import lib, TG_Offset, TG_Region, PG_Color, PG_TextStyle, PT_Cell, PT_Event, ...
"""

import ctypes
import sys
from pathlib import Path

root = Path(__file__).parent.parent

if sys.platform == "win32":
    lib_path = root / "bin" / "plague_terminal.dll"
elif sys.platform == "darwin":
    lib_path = root / "bin" / "libplague_terminal.dylib"
else:
    lib_path = root / "bin" / "libplague_terminal.so"

lib = ctypes.CDLL(str(lib_path))

# ---------------------------------------------------------------------------
# Tipos de plague-geometry
# ---------------------------------------------------------------------------

class TG_Offset(ctypes.Structure):
    _fields_ = [("x", ctypes.c_int), ("y", ctypes.c_int)]

class TG_Region(ctypes.Structure):
    _fields_ = [
        ("x",      ctypes.c_int),
        ("y",      ctypes.c_int),
        ("width",  ctypes.c_int),
        ("height", ctypes.c_int),
    ]

# ---------------------------------------------------------------------------
# Tipos de plague-drawcontext
# ---------------------------------------------------------------------------

class PG_Color(ctypes.Structure):
    _fields_ = [
        ("r", ctypes.c_float),
        ("g", ctypes.c_float),
        ("b", ctypes.c_float),
        ("a", ctypes.c_float),
    ]

class PG_TextStyle(ctypes.Structure):
    _fields_ = [
        ("font_name", ctypes.c_char * 64),
        ("font_size", ctypes.c_float),
        ("color",     PG_Color),
        ("bold",      ctypes.c_int),
        ("italic",    ctypes.c_int),
    ]

# ---------------------------------------------------------------------------
# PT_Cell — 16 bytes
# ---------------------------------------------------------------------------

class PT_Cell(ctypes.Structure):
    _fields_ = [
        ("ch",        ctypes.c_char * 4),
        ("ch_len",    ctypes.c_uint8),
        ("r_fg",      ctypes.c_uint8),
        ("g_fg",      ctypes.c_uint8),
        ("b_fg",      ctypes.c_uint8),
        ("r_bg",      ctypes.c_uint8),
        ("g_bg",      ctypes.c_uint8),
        ("b_bg",      ctypes.c_uint8),
        ("bold",      ctypes.c_uint8),
        ("italic",    ctypes.c_uint8),
        ("underline", ctypes.c_uint8),
        ("_pad",      ctypes.c_uint8 * 2),
    ]

# ---------------------------------------------------------------------------
# PT_Event
# ---------------------------------------------------------------------------

class _KeyData(ctypes.Structure):
    _fields_ = [
        ("keycode", ctypes.c_int),
        ("mods",    ctypes.c_int),
        ("ch",      ctypes.c_char * 4),
        ("ch_len",  ctypes.c_int),
    ]

class _MouseData(ctypes.Structure):
    _fields_ = [
        ("x",      ctypes.c_int),
        ("y",      ctypes.c_int),
        ("button", ctypes.c_int),
        ("mods",   ctypes.c_int),
    ]

class _ResizeData(ctypes.Structure):
    _fields_ = [
        ("cols", ctypes.c_int),
        ("rows", ctypes.c_int),
    ]

class _EventData(ctypes.Union):
    _fields_ = [
        ("key",    _KeyData),
        ("mouse",  _MouseData),
        ("resize", _ResizeData),
    ]

class PT_Event(ctypes.Structure):
    _fields_ = [
        ("type", ctypes.c_int),
        ("data", _EventData),
    ]

PT_EVENT_NONE   = 0
PT_EVENT_KEY    = 1
PT_EVENT_MOUSE  = 2
PT_EVENT_RESIZE = 3

PT_MOD_SHIFT = 0x01
PT_MOD_CTRL  = 0x02
PT_MOD_ALT   = 0x04

# ---------------------------------------------------------------------------
# Firmas de funciones
# ---------------------------------------------------------------------------

lib.pt_init_headless.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pt_init_headless.restype  = ctypes.c_int

lib.pt_shutdown.argtypes = []
lib.pt_shutdown.restype  = None

lib.pt_get_size.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
lib.pt_get_size.restype  = None

lib.pt_resize.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pt_resize.restype  = None

lib.pt_get_cell.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pt_get_cell.restype  = PT_Cell

lib.pt_clear.argtypes = []
lib.pt_clear.restype  = None

lib.pt_flush.argtypes = []
lib.pt_flush.restype  = None

lib.pt_fill_rect.argtypes   = [TG_Region, PG_Color]
lib.pt_fill_rect.restype    = None

lib.pt_stroke_rect.argtypes = [TG_Region, PG_Color, ctypes.c_float]
lib.pt_stroke_rect.restype  = None

lib.pt_draw_text.argtypes   = [TG_Offset, ctypes.c_char_p, PG_TextStyle]
lib.pt_draw_text.restype    = None

lib.pt_clip_push.argtypes   = [TG_Region]
lib.pt_clip_push.restype    = None

lib.pt_clip_pop.argtypes    = []
lib.pt_clip_pop.restype     = None

lib.pt_translate_push.argtypes = [TG_Offset]
lib.pt_translate_push.restype  = None

lib.pt_translate_pop.argtypes  = []
lib.pt_translate_pop.restype   = None

lib.pt_poll_event.argtypes = [ctypes.POINTER(PT_Event)]
lib.pt_poll_event.restype  = ctypes.c_int

lib.pt_wait_event.argtypes = [ctypes.POINTER(PT_Event)]
lib.pt_wait_event.restype  = None
