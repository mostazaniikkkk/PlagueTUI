"""
Carga compartida del DLL para todos los tests de plague-layout.
"""

import ctypes
import sys
from pathlib import Path

root = Path(__file__).parent.parent

if sys.platform == "win32":
    lib_path = root / "bin" / "plague_layout.dll"
elif sys.platform == "darwin":
    lib_path = root / "bin" / "libplague_layout.dylib"
else:
    lib_path = root / "bin" / "libplague_layout.so"

lib = ctypes.CDLL(str(lib_path))

# ---------------------------------------------------------------------------
# Tipos de plague-geometry
# ---------------------------------------------------------------------------

class TG_Spacing(ctypes.Structure):
    _fields_ = [
        ("top",    ctypes.c_int),
        ("right",  ctypes.c_int),
        ("bottom", ctypes.c_int),
        ("left",   ctypes.c_int),
    ]

class TG_Region(ctypes.Structure):
    _fields_ = [
        ("x",      ctypes.c_int),
        ("y",      ctypes.c_int),
        ("width",  ctypes.c_int),
        ("height", ctypes.c_int),
    ]

# ---------------------------------------------------------------------------
# PL_SizeValue
# ---------------------------------------------------------------------------

PL_SIZE_FIXED    = 0
PL_SIZE_FRACTION = 1

class PL_SizeValue(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int), ("value", ctypes.c_int)]

# ---------------------------------------------------------------------------
# Enums
# ---------------------------------------------------------------------------

PL_LAYOUT_VERTICAL   = 0
PL_LAYOUT_HORIZONTAL = 1
PL_LAYOUT_DOCK       = 2
PL_LAYOUT_GRID       = 3

PL_DOCK_TOP    = 0
PL_DOCK_BOTTOM = 1
PL_DOCK_LEFT   = 2
PL_DOCK_RIGHT  = 3
PL_DOCK_CENTER = 4

PL_OVERFLOW_HIDDEN = 0
PL_OVERFLOW_SCROLL = 1
PL_OVERFLOW_AUTO   = 2

# ---------------------------------------------------------------------------
# PL_LayoutNode
# ---------------------------------------------------------------------------

class PL_LayoutNode(ctypes.Structure):
    _fields_ = [
        ("layout",      ctypes.c_int),
        ("width",       PL_SizeValue),
        ("height",      PL_SizeValue),
        ("margin",      TG_Spacing),
        ("padding",     TG_Spacing),
        ("dock",        ctypes.c_int),
        ("overflow",    ctypes.c_int),
        ("grid_cols",   ctypes.c_int),
        ("grid_rows",   ctypes.c_int),
        ("children",    ctypes.c_int * 32),
        ("child_count", ctypes.c_int),
        ("parent",      ctypes.c_int),
        ("visible",     ctypes.c_int),
    ]

# ---------------------------------------------------------------------------
# PL_LayoutResult
# ---------------------------------------------------------------------------

class PL_LayoutResult(ctypes.Structure):
    _fields_ = [
        ("region",         TG_Region),
        ("content_region", TG_Region),
    ]

# ---------------------------------------------------------------------------
# Firmas de funciones
# ---------------------------------------------------------------------------

lib.pl_tree_init.argtypes = []
lib.pl_tree_init.restype  = None

lib.pl_tree_add.argtypes  = [PL_LayoutNode]
lib.pl_tree_add.restype   = ctypes.c_int

lib.pl_tree_set_child.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pl_tree_set_child.restype  = None

lib.pl_tree_count.argtypes = []
lib.pl_tree_count.restype  = ctypes.c_int

lib.pl_tree_get.argtypes   = [ctypes.c_int]
lib.pl_tree_get.restype    = PL_LayoutNode

lib.pl_tree_compute.argtypes = [ctypes.c_int, TG_Region]
lib.pl_tree_compute.restype  = None

lib.pl_tree_result.argtypes = [ctypes.c_int]
lib.pl_tree_result.restype  = PL_LayoutResult

lib.pl_size_fixed.argtypes    = [ctypes.c_int]
lib.pl_size_fixed.restype     = PL_SizeValue

lib.pl_size_fraction.argtypes = [ctypes.c_int]
lib.pl_size_fraction.restype  = PL_SizeValue

lib.pl_node_vertical.argtypes   = [PL_SizeValue, PL_SizeValue, TG_Spacing]
lib.pl_node_vertical.restype    = PL_LayoutNode

lib.pl_node_horizontal.argtypes = [PL_SizeValue, PL_SizeValue, TG_Spacing]
lib.pl_node_horizontal.restype  = PL_LayoutNode

lib.pl_node_dock_root.argtypes = [TG_Spacing]
lib.pl_node_dock_root.restype  = PL_LayoutNode

lib.pl_node_docked.argtypes = [ctypes.c_int, PL_SizeValue, PL_SizeValue]
lib.pl_node_docked.restype  = PL_LayoutNode

lib.pl_node_grid.argtypes = [PL_SizeValue, PL_SizeValue, ctypes.c_int, ctypes.c_int]
lib.pl_node_grid.restype  = PL_LayoutNode

# ---------------------------------------------------------------------------
# Helpers de Python
# ---------------------------------------------------------------------------

def no_spacing():
    return TG_Spacing(0, 0, 0, 0)

def fixed(v):
    return lib.pl_size_fixed(v)

def fr(v=1):
    return lib.pl_size_fraction(v)

def region(x=0, y=0, w=80, h=24):
    return TG_Region(x, y, w, h)

def result(idx):
    return lib.pl_tree_result(idx)

def compute(root_idx=0, x=0, y=0, w=80, h=24):
    lib.pl_tree_compute(root_idx, TG_Region(x, y, w, h))
