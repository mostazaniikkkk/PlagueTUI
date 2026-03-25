import ctypes
import sys
import unittest
from pathlib import Path

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

    lib.tg_region_contains.argtypes  = [Region, Offset]
    lib.tg_region_contains.restype   = ctypes.c_bool

    lib.tg_region_clip.argtypes      = [Region, Region]
    lib.tg_region_clip.restype       = Region

    lib.tg_region_union.argtypes     = [Region, Region]
    lib.tg_region_union.restype      = Region

    lib.tg_region_translate.argtypes = [Region, Offset]
    lib.tg_region_translate.restype  = Region

    lib.tg_region_is_empty.argtypes  = [Region]
    lib.tg_region_is_empty.restype   = ctypes.c_bool

    lib.tg_region_center.argtypes    = [Region]
    lib.tg_region_center.restype     = Offset

    lib.tg_region_size.argtypes      = [Region]
    lib.tg_region_size.restype       = Size

    lib.tg_region_eq.argtypes        = [Region, Region]
    lib.tg_region_eq.restype         = ctypes.c_bool

    class Spacing(ctypes.Structure):
        _fields_ = [
            ("top",    ctypes.c_int),
            ("right",  ctypes.c_int),
            ("bottom", ctypes.c_int),
            ("left",   ctypes.c_int),
        ]

    lib.tg_region_inflate.argtypes   = [Region, Spacing]
    lib.tg_region_inflate.restype    = Region

    lib.tg_region_deflate.argtypes   = [Region, Spacing]
    lib.tg_region_deflate.restype    = Region

    return lib, Offset, Size, Region, Spacing

lib, Offset, Size, Region, Spacing = load_lib()


class TestRegion(unittest.TestCase):

    def test_contains_interior_point(self):
        self.assertTrue(lib.tg_region_contains(Region(0, 0, 100, 100), Offset(50, 50)))

    def test_contains_edge_point(self):
        self.assertTrue(lib.tg_region_contains(Region(0, 0, 100, 100), Offset(100, 100)))

    def test_contains_exterior_point(self):
        self.assertFalse(lib.tg_region_contains(Region(0, 0, 100, 100), Offset(101, 50)))

    def test_clip_partial_intersection(self):
        r = lib.tg_region_clip(Region(0, 0, 100, 100), Region(50, 50, 100, 100))
        self.assertEqual(r.x,      50)
        self.assertEqual(r.y,      50)
        self.assertEqual(r.width,  50)
        self.assertEqual(r.height, 50)

    def test_clip_no_intersection_returns_zero(self):
        r = lib.tg_region_clip(Region(0, 0, 50, 50), Region(100, 0, 50, 50))
        self.assertEqual(r.width,  0)
        self.assertEqual(r.height, 0)

    def test_clip_contained(self):
        r = lib.tg_region_clip(Region(0, 0, 200, 200), Region(50, 50, 50, 50))
        self.assertEqual(r.x,      50)
        self.assertEqual(r.y,      50)
        self.assertEqual(r.width,  50)
        self.assertEqual(r.height, 50)

    def test_union(self):
        r = lib.tg_region_union(Region(0, 0, 50, 50), Region(50, 50, 50, 50))
        self.assertEqual(r.x,      0)
        self.assertEqual(r.y,      0)
        self.assertEqual(r.width,  100)
        self.assertEqual(r.height, 100)

    def test_translate(self):
        t = lib.tg_region_translate(Region(10, 20, 50, 50), Offset(5, -10))
        self.assertEqual(t.x,      15)
        self.assertEqual(t.y,      10)
        self.assertEqual(t.width,  50)
        self.assertEqual(t.height, 50)

    def test_is_empty_zero_width(self):
        self.assertTrue(lib.tg_region_is_empty(Region(0, 0, 0, 100)))

    def test_is_empty_zero_height(self):
        self.assertTrue(lib.tg_region_is_empty(Region(0, 0, 100, 0)))

    def test_is_empty_false(self):
        self.assertFalse(lib.tg_region_is_empty(Region(0, 0, 1, 1)))

    def test_center(self):
        c = lib.tg_region_center(Region(0, 0, 100, 200))
        self.assertEqual(c.x, 50)
        self.assertEqual(c.y, 100)

    def test_size(self):
        s = lib.tg_region_size(Region(10, 10, 80, 40))
        self.assertEqual(s.width,  80)
        self.assertEqual(s.height, 40)

    def test_eq_equal(self):
        r = Region(1, 2, 3, 4)
        self.assertTrue(lib.tg_region_eq(r, r))

    def test_eq_different(self):
        self.assertFalse(lib.tg_region_eq(Region(1, 2, 3, 4), Region(1, 2, 3, 5)))

    def test_inflate(self):
        result = lib.tg_region_inflate(Region(10, 10, 100, 100), Spacing(5, 5, 5, 5))
        self.assertEqual(result.x,      5)
        self.assertEqual(result.y,      5)
        self.assertEqual(result.width,  110)
        self.assertEqual(result.height, 110)

    def test_inflate_asymmetric(self):
        result = lib.tg_region_inflate(Region(10, 10, 100, 100), Spacing(2, 8, 4, 6))
        self.assertEqual(result.x,      4)
        self.assertEqual(result.y,      8)
        self.assertEqual(result.width,  114)
        self.assertEqual(result.height, 106)

    def test_deflate(self):
        result = lib.tg_region_deflate(Region(10, 10, 100, 100), Spacing(5, 5, 5, 5))
        self.assertEqual(result.x,      15)
        self.assertEqual(result.y,      15)
        self.assertEqual(result.width,  90)
        self.assertEqual(result.height, 90)

    def test_deflate_exceeds_size_returns_zero(self):
        result = lib.tg_region_deflate(Region(0, 0, 10, 10), Spacing(20, 20, 20, 20))
        self.assertEqual(result.width,  0)
        self.assertEqual(result.height, 0)


if __name__ == "__main__":
    unittest.main()
