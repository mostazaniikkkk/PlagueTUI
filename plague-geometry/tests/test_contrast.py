"""
Test de contraste: verifica que el DLL produce resultados equivalentes
a la implementación de referencia de Textual (geometry.py).

Notas de comportamiento:
- Coordenadas son i32 (celdas de terminal): comparación exacta con assertEqual.
- Region.contains en Textual es exclusivo en borde derecho/inferior.
  El DLL es inclusivo. Los tests de contains usan solo puntos interiores.
- Region.grow == tg_region_inflate
- Region.shrink == tg_region_deflate
- not bool(py_region) == tg_region_is_empty
"""

import ctypes
import sys
import unittest
from pathlib import Path

from geometry import Offset as PyOffset
from geometry import Size as PySize
from geometry import Region as PyRegion
from geometry import Spacing as PySpacing

# ---------------------------------------------------------------------------
# Cargar DLL
# ---------------------------------------------------------------------------

def load_lib():
    root = Path(__file__).parent.parent
    if sys.platform == "win32":
        lib_path = root / "zig-out" / "bin" / "plague_geometry.dll"
    elif sys.platform == "darwin":
        lib_path = root / "zig-out" / "lib" / "libplague_geometry.dylib"
    else:
        lib_path = root / "zig-out" / "lib" / "libplague_geometry.so"

    lib = ctypes.CDLL(str(lib_path))

    class Offset(ctypes.Structure):
        _fields_ = [("x", ctypes.c_int), ("y", ctypes.c_int)]

    class Size(ctypes.Structure):
        _fields_ = [("width", ctypes.c_int), ("height", ctypes.c_int)]

    class Region(ctypes.Structure):
        _fields_ = [
            ("x",      ctypes.c_int),
            ("y",      ctypes.c_int),
            ("width",  ctypes.c_int),
            ("height", ctypes.c_int),
        ]

    class Spacing(ctypes.Structure):
        _fields_ = [
            ("top",    ctypes.c_int),
            ("right",  ctypes.c_int),
            ("bottom", ctypes.c_int),
            ("left",   ctypes.c_int),
        ]

    lib.tg_offset_add.argtypes     = [Offset, Offset];         lib.tg_offset_add.restype     = Offset
    lib.tg_offset_sub.argtypes     = [Offset, Offset];         lib.tg_offset_sub.restype     = Offset
    lib.tg_offset_scale.argtypes   = [Offset, ctypes.c_int];   lib.tg_offset_scale.restype   = Offset
    lib.tg_size_add.argtypes       = [Size, Size];             lib.tg_size_add.restype       = Size
    lib.tg_size_scale.argtypes     = [Size, ctypes.c_int];     lib.tg_size_scale.restype     = Size
    lib.tg_size_area.argtypes      = [Size];                   lib.tg_size_area.restype      = ctypes.c_int
    lib.tg_region_contains.argtypes  = [Region, Offset];       lib.tg_region_contains.restype  = ctypes.c_bool
    lib.tg_region_clip.argtypes      = [Region, Region];       lib.tg_region_clip.restype      = Region
    lib.tg_region_union.argtypes     = [Region, Region];       lib.tg_region_union.restype     = Region
    lib.tg_region_translate.argtypes = [Region, Offset];       lib.tg_region_translate.restype = Region
    lib.tg_region_inflate.argtypes   = [Region, Spacing];      lib.tg_region_inflate.restype   = Region
    lib.tg_region_deflate.argtypes   = [Region, Spacing];      lib.tg_region_deflate.restype   = Region
    lib.tg_region_is_empty.argtypes  = [Region];               lib.tg_region_is_empty.restype  = ctypes.c_bool
    lib.tg_region_center.argtypes    = [Region];               lib.tg_region_center.restype    = Offset
    lib.tg_region_size.argtypes      = [Region];               lib.tg_region_size.restype      = Size
    lib.tg_spacing_uniform.argtypes  = [ctypes.c_int];         lib.tg_spacing_uniform.restype  = Spacing
    lib.tg_spacing_add.argtypes      = [Spacing, Spacing];     lib.tg_spacing_add.restype      = Spacing

    return lib, Offset, Size, Region, Spacing

lib, COffset, CSize, CRegion, CSpacing = load_lib()

# ---------------------------------------------------------------------------
# Contraste: Offset
# ---------------------------------------------------------------------------

class TestContrastOffset(unittest.TestCase):

    def test_add(self):
        py = PyOffset(3, 7) + PyOffset(2, 1)
        c  = lib.tg_offset_add(COffset(3, 7), COffset(2, 1))
        self.assertEqual(c.x, py.x)
        self.assertEqual(c.y, py.y)

    def test_sub(self):
        py = PyOffset(10, 5) - PyOffset(3, 2)
        c  = lib.tg_offset_sub(COffset(10, 5), COffset(3, 2))
        self.assertEqual(c.x, py.x)
        self.assertEqual(c.y, py.y)

    def test_sub_negative(self):
        py = PyOffset(1, 1) - PyOffset(5, 5)
        c  = lib.tg_offset_sub(COffset(1, 1), COffset(5, 5))
        self.assertEqual(c.x, py.x)
        self.assertEqual(c.y, py.y)

    def test_scale(self):
        py = PyOffset(4, 6) * 2
        c  = lib.tg_offset_scale(COffset(4, 6), 2)
        self.assertEqual(c.x, py.x)
        self.assertEqual(c.y, py.y)

# ---------------------------------------------------------------------------
# Contraste: Size
# ---------------------------------------------------------------------------

class TestContrastSize(unittest.TestCase):

    def test_add(self):
        py = PySize(100, 200) + PySize(50, 75)
        c  = lib.tg_size_add(CSize(100, 200), CSize(50, 75))
        self.assertEqual(c.width,  py.width)
        self.assertEqual(c.height, py.height)

    def test_area(self):
        py = PySize(12, 8)
        c  = lib.tg_size_area(CSize(12, 8))
        self.assertEqual(c, py.width * py.height)

# ---------------------------------------------------------------------------
# Contraste: Region
# ---------------------------------------------------------------------------

class TestContrastRegion(unittest.TestCase):

    def test_contains_interior(self):
        # Solo punto interior — evitar diferencia inclusivo vs exclusivo en borde
        py = PyRegion(0, 0, 100, 100).contains(50, 50)
        c  = lib.tg_region_contains(CRegion(0, 0, 100, 100), COffset(50, 50))
        self.assertEqual(c, py)

    def test_contains_exterior(self):
        py = PyRegion(0, 0, 100, 100).contains(150, 50)
        c  = lib.tg_region_contains(CRegion(0, 0, 100, 100), COffset(150, 50))
        self.assertEqual(c, py)

    def test_clip(self):
        py = PyRegion(0, 0, 100, 100).intersection(PyRegion(50, 50, 100, 100))
        c  = lib.tg_region_clip(CRegion(0, 0, 100, 100), CRegion(50, 50, 100, 100))
        self.assertEqual(c.x,      py.x)
        self.assertEqual(c.y,      py.y)
        self.assertEqual(c.width,  py.width)
        self.assertEqual(c.height, py.height)

    def test_clip_no_intersection(self):
        # Regiones separadas en ambos ejes — ambas implementaciones retornan dimensiones cero
        py = PyRegion(0, 0, 50, 50).intersection(PyRegion(100, 100, 50, 50))
        c  = lib.tg_region_clip(CRegion(0, 0, 50, 50), CRegion(100, 100, 50, 50))
        self.assertEqual(c.width,  py.width)
        self.assertEqual(c.height, py.height)

    def test_union(self):
        py = PyRegion(0, 0, 50, 50).union(PyRegion(50, 50, 50, 50))
        c  = lib.tg_region_union(CRegion(0, 0, 50, 50), CRegion(50, 50, 50, 50))
        self.assertEqual(c.x,      py.x)
        self.assertEqual(c.y,      py.y)
        self.assertEqual(c.width,  py.width)
        self.assertEqual(c.height, py.height)

    def test_translate(self):
        py = PyRegion(10, 20, 50, 50).translate(PyOffset(5, -10))
        c  = lib.tg_region_translate(CRegion(10, 20, 50, 50), COffset(5, -10))
        self.assertEqual(c.x,      py.x)
        self.assertEqual(c.y,      py.y)
        self.assertEqual(c.width,  py.width)
        self.assertEqual(c.height, py.height)

    def test_inflate(self):
        # Textual: Region.grow((top, right, bottom, left))
        py = PyRegion(10, 10, 100, 100).grow((5, 5, 5, 5))
        c  = lib.tg_region_inflate(CRegion(10, 10, 100, 100), CSpacing(5, 5, 5, 5))
        self.assertEqual(c.x,      py.x)
        self.assertEqual(c.y,      py.y)
        self.assertEqual(c.width,  py.width)
        self.assertEqual(c.height, py.height)

    def test_deflate(self):
        # Textual: Region.shrink((top, right, bottom, left))
        py = PyRegion(10, 10, 100, 100).shrink((5, 5, 5, 5))
        c  = lib.tg_region_deflate(CRegion(10, 10, 100, 100), CSpacing(5, 5, 5, 5))
        self.assertEqual(c.x,      py.x)
        self.assertEqual(c.y,      py.y)
        self.assertEqual(c.width,  py.width)
        self.assertEqual(c.height, py.height)

    def test_is_empty(self):
        # not bool(py_region) == tg_region_is_empty
        py = PyRegion(0, 0, 0, 100)
        c  = lib.tg_region_is_empty(CRegion(0, 0, 0, 100))
        self.assertEqual(c, not bool(py))

    def test_center(self):
        py_cx, py_cy = PyRegion(0, 0, 100, 200).center
        c = lib.tg_region_center(CRegion(0, 0, 100, 200))
        self.assertEqual(c.x, py_cx)
        self.assertEqual(c.y, py_cy)

    def test_size(self):
        py = PyRegion(10, 10, 80, 40)
        c  = lib.tg_region_size(CRegion(10, 10, 80, 40))
        self.assertEqual(c.width,  py.width)
        self.assertEqual(c.height, py.height)

# ---------------------------------------------------------------------------
# Contraste: Spacing
# ---------------------------------------------------------------------------

class TestContrastSpacing(unittest.TestCase):

    def test_uniform(self):
        py = PySpacing(8, 8, 8, 8)
        c  = lib.tg_spacing_uniform(8)
        self.assertEqual(c.top,    py.top)
        self.assertEqual(c.right,  py.right)
        self.assertEqual(c.bottom, py.bottom)
        self.assertEqual(c.left,   py.left)

    def test_add(self):
        py = PySpacing(1, 2, 3, 4) + PySpacing(4, 3, 2, 1)
        c  = lib.tg_spacing_add(CSpacing(1, 2, 3, 4), CSpacing(4, 3, 2, 1))
        self.assertEqual(c.top,    py.top)
        self.assertEqual(c.right,  py.right)
        self.assertEqual(c.bottom, py.bottom)
        self.assertEqual(c.left,   py.left)


if __name__ == "__main__":
    unittest.main()
