"""
Carga del DLL para los tests de plague-app.
Las DLLs dependientes deben estar en bin\ (copiadas por build.bat).
"""

import ctypes
import sys
import os
from pathlib import Path

root = Path(__file__).parent.parent
bin_dir = root / "bin"

# Añadir bin\ al search path de DLLs (necesario en Windows para las deps)
if sys.platform == "win32":
    os.add_dll_directory(str(bin_dir))

if sys.platform == "win32":
    lib_path = bin_dir / "plague_app.dll"
elif sys.platform == "darwin":
    lib_path = bin_dir / "libplague_app.dylib"
else:
    lib_path = bin_dir / "libplague_app.so"

lib = ctypes.CDLL(str(lib_path))

# ---------------------------------------------------------------------------
# Tipos de plague-geometry
# ---------------------------------------------------------------------------

class TG_Region(ctypes.Structure):
    _fields_ = [
        ("x",      ctypes.c_int),
        ("y",      ctypes.c_int),
        ("width",  ctypes.c_int),
        ("height", ctypes.c_int),
    ]

class TG_Spacing(ctypes.Structure):
    _fields_ = [
        ("top",    ctypes.c_int),
        ("right",  ctypes.c_int),
        ("bottom", ctypes.c_int),
        ("left",   ctypes.c_int),
    ]

# ---------------------------------------------------------------------------
# Tipos de plague-layout
# ---------------------------------------------------------------------------

PL_SIZE_FIXED    = 0
PL_SIZE_FRACTION = 1

PL_LAYOUT_VERTICAL   = 0
PL_LAYOUT_HORIZONTAL = 1
PL_LAYOUT_DOCK       = 2
PL_LAYOUT_GRID       = 3

PL_DOCK_TOP    = 0
PL_DOCK_BOTTOM = 1
PL_DOCK_LEFT   = 2
PL_DOCK_RIGHT  = 3
PL_DOCK_CENTER = 4

class PL_SizeValue(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int), ("value", ctypes.c_int)]

# ---------------------------------------------------------------------------
# Constantes de plague-app
# ---------------------------------------------------------------------------

PA_NO_PARENT = -1

# ---------------------------------------------------------------------------
# Firmas de funciones
# ---------------------------------------------------------------------------

lib.pa_init.argtypes          = []
lib.pa_init.restype           = ctypes.c_int

lib.pa_init_headless.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pa_init_headless.restype  = ctypes.c_int

lib.pa_shutdown.argtypes      = []
lib.pa_shutdown.restype       = None

lib.pa_get_cols.argtypes      = []
lib.pa_get_cols.restype       = ctypes.c_int

lib.pa_get_rows.argtypes      = []
lib.pa_get_rows.restype       = ctypes.c_int

lib.pa_css_load.argtypes      = [ctypes.c_char_p]
lib.pa_css_load.restype       = ctypes.c_int

lib.pa_widget_add.argtypes    = [ctypes.c_char_p, ctypes.c_char_p,
                                  ctypes.c_char_p, ctypes.c_int]
lib.pa_widget_add.restype     = ctypes.c_int

lib.pa_widget_set_text.argtypes    = [ctypes.c_int, ctypes.c_char_p]
lib.pa_widget_set_text.restype     = None

lib.pa_widget_set_size.argtypes    = [ctypes.c_int, PL_SizeValue, PL_SizeValue]
lib.pa_widget_set_size.restype     = None

lib.pa_widget_set_dock.argtypes    = [ctypes.c_int, ctypes.c_int]
lib.pa_widget_set_dock.restype     = None

lib.pa_widget_set_layout.argtypes  = [ctypes.c_int, ctypes.c_int]
lib.pa_widget_set_layout.restype   = None

lib.pa_widget_set_padding.argtypes = [ctypes.c_int, TG_Spacing]
lib.pa_widget_set_padding.restype  = None

lib.pa_widget_region.argtypes      = [ctypes.c_int]
lib.pa_widget_region.restype       = TG_Region

lib.pa_render.argtypes = []
lib.pa_render.restype  = None

lib.pa_bind_key.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
lib.pa_bind_key.restype  = ctypes.c_int

lib.pa_poll.argtypes      = []
lib.pa_poll.restype       = ctypes.c_int

lib.pa_wait_poll.argtypes = []
lib.pa_wait_poll.restype  = ctypes.c_int

lib.pa_quit.argtypes = []
lib.pa_quit.restype  = None

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def fixed(v):
    return PL_SizeValue(PL_SIZE_FIXED, v)

def fr(v=1):
    return PL_SizeValue(PL_SIZE_FRACTION, v)

def no_spacing():
    return TG_Spacing(0, 0, 0, 0)

def spacing(top=0, right=0, bottom=0, left=0):
    return TG_Spacing(top, right, bottom, left)
