"""
Carga compartida del DLL para los tests de plague-css.
"""

import ctypes
import sys
from pathlib import Path

root = Path(__file__).parent.parent

if sys.platform == "win32":
    lib_path = root / "bin" / "plague_css.dll"
elif sys.platform == "darwin":
    lib_path = root / "bin" / "libplague_css.dylib"
else:
    lib_path = root / "bin" / "libplague_css.so"

lib = ctypes.CDLL(str(lib_path))

# ---------------------------------------------------------------------------
# Tipos de plague-geometry (replicados aquí sin dep. del DLL de geometría)
# ---------------------------------------------------------------------------

class TG_Spacing(ctypes.Structure):
    _fields_ = [
        ("top",    ctypes.c_int),
        ("right",  ctypes.c_int),
        ("bottom", ctypes.c_int),
        ("left",   ctypes.c_int),
    ]

# ---------------------------------------------------------------------------
# PL_SizeValue (de plague-layout)
# ---------------------------------------------------------------------------

PL_SIZE_FIXED    = 0
PL_SIZE_FRACTION = 1

class PL_SizeValue(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int), ("value", ctypes.c_int)]

# ---------------------------------------------------------------------------
# PC_Color
# ---------------------------------------------------------------------------

class PC_Color(ctypes.Structure):
    _fields_ = [
        ("r", ctypes.c_uint8),
        ("g", ctypes.c_uint8),
        ("b", ctypes.c_uint8),
        ("a", ctypes.c_uint8),
    ]

# ---------------------------------------------------------------------------
# PC_BorderStyle
# ---------------------------------------------------------------------------

PC_BORDER_NONE    = 0
PC_BORDER_SOLID   = 1
PC_BORDER_DASHED  = 2
PC_BORDER_ROUNDED = 3
PC_BORDER_THICK   = 4
PC_BORDER_DOUBLE  = 5

# ---------------------------------------------------------------------------
# PC_ComputedStyle
# ---------------------------------------------------------------------------

PC_HAS_COLOR      = 0x0001
PC_HAS_BACKGROUND = 0x0002
PC_HAS_BORDER     = 0x0004
PC_HAS_MARGIN     = 0x0008
PC_HAS_PADDING    = 0x0010
PC_HAS_WIDTH      = 0x0020
PC_HAS_HEIGHT     = 0x0040
PC_HAS_DOCK       = 0x0080
PC_HAS_DISPLAY    = 0x0100
PC_HAS_OVERFLOW   = 0x0200
PC_HAS_BOLD       = 0x0400
PC_HAS_ITALIC     = 0x0800
PC_HAS_UNDERLINE  = 0x1000

class PC_ComputedStyle(ctypes.Structure):
    _fields_ = [
        ("set_flags",    ctypes.c_uint32),
        ("color",        PC_Color),
        ("background",   PC_Color),
        ("border_color", PC_Color),
        ("border_style", ctypes.c_int),
        ("margin",       TG_Spacing),
        ("padding",      TG_Spacing),
        ("width",        PL_SizeValue),
        ("height",       PL_SizeValue),
        ("dock",         ctypes.c_int),
        ("display",      ctypes.c_int),
        ("overflow",     ctypes.c_int),
        ("bold",         ctypes.c_uint8),
        ("italic",       ctypes.c_uint8),
        ("underline",    ctypes.c_uint8),
    ]

# ---------------------------------------------------------------------------
# PC_STATE flags
# ---------------------------------------------------------------------------

PC_STATE_HOVER    = 0x01
PC_STATE_FOCUS    = 0x02
PC_STATE_PRESSED  = 0x04
PC_STATE_DISABLED = 0x08

# ---------------------------------------------------------------------------
# Firmas de funciones
# ---------------------------------------------------------------------------

lib.pc_stylesheet_load.argtypes  = [ctypes.c_char_p]
lib.pc_stylesheet_load.restype   = ctypes.c_int

lib.pc_stylesheet_free.argtypes  = [ctypes.c_int]
lib.pc_stylesheet_free.restype   = None

lib.pc_stylesheet_rule_count.argtypes = [ctypes.c_int]
lib.pc_stylesheet_rule_count.restype  = ctypes.c_int

lib.pc_compute_style.argtypes = [
    ctypes.c_int,    # ss_handle
    ctypes.c_char_p, # type_name
    ctypes.c_char_p, # id
    ctypes.c_char_p, # classes
    ctypes.c_int,    # state
]
lib.pc_compute_style.restype = PC_ComputedStyle

lib.pc_default_style.argtypes = []
lib.pc_default_style.restype  = PC_ComputedStyle

# ---------------------------------------------------------------------------
# Helpers Python
# ---------------------------------------------------------------------------

def load(tcss: str) -> int:
    return lib.pc_stylesheet_load(tcss.encode())

def free(handle: int):
    lib.pc_stylesheet_free(handle)

def rule_count(handle: int) -> int:
    return lib.pc_stylesheet_rule_count(handle)

def compute(handle: int, type_name="", id_="", classes="", state=0) -> PC_ComputedStyle:
    return lib.pc_compute_style(
        handle,
        type_name.encode(),
        id_.encode(),
        classes.encode(),
        state,
    )
