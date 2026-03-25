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

    lib.tg_offset_add.argtypes   = [Offset, Offset]
    lib.tg_offset_add.restype    = Offset

    lib.tg_offset_sub.argtypes   = [Offset, Offset]
    lib.tg_offset_sub.restype    = Offset

    lib.tg_offset_scale.argtypes = [Offset, ctypes.c_int]
    lib.tg_offset_scale.restype  = Offset

    lib.tg_offset_eq.argtypes    = [Offset, Offset]
    lib.tg_offset_eq.restype     = ctypes.c_bool

    return lib, Offset

lib, Offset = load_lib()


class TestOffset(unittest.TestCase):

    def test_offset_add(self):
        r = lib.tg_offset_add(Offset(1, 2), Offset(3, 4))
        self.assertEqual(r.x, 4)
        self.assertEqual(r.y, 6)

    def test_offset_sub(self):
        r = lib.tg_offset_sub(Offset(5, 7), Offset(2, 3))
        self.assertEqual(r.x, 3)
        self.assertEqual(r.y, 4)

    def test_offset_sub_negative_result(self):
        r = lib.tg_offset_sub(Offset(1, 1), Offset(5, 5))
        self.assertEqual(r.x, -4)
        self.assertEqual(r.y, -4)

    def test_offset_scale(self):
        r = lib.tg_offset_scale(Offset(2, 3), 4)
        self.assertEqual(r.x, 8)
        self.assertEqual(r.y, 12)

    def test_offset_scale_by_zero(self):
        r = lib.tg_offset_scale(Offset(99, 42), 0)
        self.assertEqual(r.x, 0)
        self.assertEqual(r.y, 0)

    def test_offset_eq_equal(self):
        self.assertTrue(lib.tg_offset_eq(Offset(1, 2), Offset(1, 2)))

    def test_offset_eq_different(self):
        self.assertFalse(lib.tg_offset_eq(Offset(1, 2), Offset(1, 3)))

    def test_offset_negative_values(self):
        r = lib.tg_offset_add(Offset(-10, -20), Offset(-5, -10))
        self.assertEqual(r.x, -15)
        self.assertEqual(r.y, -30)

    def test_offset_zero(self):
        self.assertTrue(lib.tg_offset_eq(Offset(0, 0), Offset(0, 0)))


if __name__ == "__main__":
    unittest.main()
