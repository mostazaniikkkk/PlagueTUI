import unittest
from conftest import lib, Color

class TestColor(unittest.TestCase):

    def test_rgba(self):
        c = lib.pg_color_rgba(1.0, 0.5, 0.25, 1.0)
        self.assertAlmostEqual(c.r, 1.0,  places=5)
        self.assertAlmostEqual(c.g, 0.5,  places=5)
        self.assertAlmostEqual(c.b, 0.25, places=5)
        self.assertAlmostEqual(c.a, 1.0,  places=5)

    def test_from_hex(self):
        # 0xFF8040FF = r=255, g=128, b=64, a=255
        c = lib.pg_color_from_hex(0xFF8040FF)
        self.assertAlmostEqual(c.r, 1.0,        places=3)
        self.assertAlmostEqual(c.g, 128/255.0,  places=3)
        self.assertAlmostEqual(c.b, 64/255.0,   places=3)
        self.assertAlmostEqual(c.a, 1.0,        places=3)

    def test_from_hex_transparent(self):
        c = lib.pg_color_from_hex(0x00000000)
        self.assertAlmostEqual(c.r, 0.0, places=5)
        self.assertAlmostEqual(c.a, 0.0, places=5)

    def test_mix_half(self):
        a = lib.pg_color_rgba(0.0, 0.0, 0.0, 1.0)
        b = lib.pg_color_rgba(1.0, 1.0, 1.0, 1.0)
        m = lib.pg_color_mix(a, b, 0.5)
        self.assertAlmostEqual(m.r, 0.5, places=5)
        self.assertAlmostEqual(m.g, 0.5, places=5)
        self.assertAlmostEqual(m.b, 0.5, places=5)

    def test_mix_t0_returns_a(self):
        a = lib.pg_color_rgba(1.0, 0.0, 0.0, 1.0)
        b = lib.pg_color_rgba(0.0, 1.0, 0.0, 1.0)
        m = lib.pg_color_mix(a, b, 0.0)
        self.assertTrue(lib.pg_color_eq(m, a))

    def test_mix_t1_returns_b(self):
        a = lib.pg_color_rgba(1.0, 0.0, 0.0, 1.0)
        b = lib.pg_color_rgba(0.0, 1.0, 0.0, 1.0)
        m = lib.pg_color_mix(a, b, 1.0)
        self.assertTrue(lib.pg_color_eq(m, b))

    def test_with_alpha(self):
        c = lib.pg_color_rgba(1.0, 0.0, 0.0, 1.0)
        t = lib.pg_color_with_alpha(c, 0.5)
        self.assertAlmostEqual(t.r, 1.0, places=5)
        self.assertAlmostEqual(t.a, 0.5, places=5)

    def test_eq_equal(self):
        a = lib.pg_color_rgba(0.1, 0.2, 0.3, 0.4)
        b = lib.pg_color_rgba(0.1, 0.2, 0.3, 0.4)
        self.assertTrue(lib.pg_color_eq(a, b))

    def test_eq_different(self):
        a = lib.pg_color_rgba(1.0, 0.0, 0.0, 1.0)
        b = lib.pg_color_rgba(0.0, 1.0, 0.0, 1.0)
        self.assertFalse(lib.pg_color_eq(a, b))

if __name__ == "__main__":
    unittest.main()
