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

    class Size(ctypes.Structure):
        _fields_ = [("width", ctypes.c_int), ("height", ctypes.c_int)]

    lib.tg_size_add.argtypes   = [Size, Size]
    lib.tg_size_add.restype    = Size

    lib.tg_size_scale.argtypes = [Size, ctypes.c_int]
    lib.tg_size_scale.restype  = Size

    lib.tg_size_area.argtypes  = [Size]
    lib.tg_size_area.restype   = ctypes.c_int

    lib.tg_size_eq.argtypes    = [Size, Size]
    lib.tg_size_eq.restype     = ctypes.c_bool

    return lib, Size

lib, Size = load_lib()


class TestSize(unittest.TestCase):

    def test_size_add(self):
        r = lib.tg_size_add(Size(10, 20), Size(5, 15))
        self.assertEqual(r.width,  15)
        self.assertEqual(r.height, 35)

    def test_size_scale(self):
        r = lib.tg_size_scale(Size(4, 8), 2)
        self.assertEqual(r.width,  8)
        self.assertEqual(r.height, 16)

    def test_size_scale_by_zero(self):
        r = lib.tg_size_scale(Size(99, 42), 0)
        self.assertEqual(r.width,  0)
        self.assertEqual(r.height, 0)

    def test_size_area(self):
        self.assertEqual(lib.tg_size_area(Size(10, 5)), 50)

    def test_size_area_zero(self):
        self.assertEqual(lib.tg_size_area(Size(0, 100)), 0)

    def test_size_eq_equal(self):
        self.assertTrue(lib.tg_size_eq(Size(3, 7), Size(3, 7)))

    def test_size_eq_different(self):
        self.assertFalse(lib.tg_size_eq(Size(3, 7), Size(3, 8)))

    def test_size_zero(self):
        self.assertTrue(lib.tg_size_eq(Size(0, 0), Size(0, 0)))


if __name__ == "__main__":
    unittest.main()
