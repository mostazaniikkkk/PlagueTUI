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

    class Spacing(ctypes.Structure):
        _fields_ = [
            ("top",    ctypes.c_int),
            ("right",  ctypes.c_int),
            ("bottom", ctypes.c_int),
            ("left",   ctypes.c_int),
        ]

    lib.tg_spacing_uniform.argtypes = [ctypes.c_int]
    lib.tg_spacing_uniform.restype  = Spacing

    lib.tg_spacing_add.argtypes     = [Spacing, Spacing]
    lib.tg_spacing_add.restype      = Spacing

    lib.tg_spacing_eq.argtypes      = [Spacing, Spacing]
    lib.tg_spacing_eq.restype       = ctypes.c_bool

    return lib, Spacing

lib, Spacing = load_lib()


class TestSpacing(unittest.TestCase):

    def test_spacing_uniform(self):
        s = lib.tg_spacing_uniform(8)
        self.assertEqual(s.top,    8)
        self.assertEqual(s.right,  8)
        self.assertEqual(s.bottom, 8)
        self.assertEqual(s.left,   8)

    def test_spacing_uniform_zero(self):
        s = lib.tg_spacing_uniform(0)
        self.assertEqual(s.top,    0)
        self.assertEqual(s.right,  0)
        self.assertEqual(s.bottom, 0)
        self.assertEqual(s.left,   0)

    def test_spacing_add(self):
        a = Spacing(1, 2, 3, 4)
        b = Spacing(4, 3, 2, 1)
        r = lib.tg_spacing_add(a, b)
        self.assertEqual(r.top,    5)
        self.assertEqual(r.right,  5)
        self.assertEqual(r.bottom, 5)
        self.assertEqual(r.left,   5)

    def test_spacing_eq_equal(self):
        self.assertTrue(lib.tg_spacing_eq(Spacing(1, 2, 3, 4), Spacing(1, 2, 3, 4)))

    def test_spacing_eq_different(self):
        self.assertFalse(lib.tg_spacing_eq(Spacing(1, 2, 3, 4), Spacing(1, 2, 3, 5)))

    def test_spacing_zero(self):
        self.assertTrue(lib.tg_spacing_eq(Spacing(0, 0, 0, 0), Spacing(0, 0, 0, 0)))


if __name__ == "__main__":
    unittest.main()
