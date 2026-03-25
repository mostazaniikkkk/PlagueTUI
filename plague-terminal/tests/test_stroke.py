import unittest
from conftest import lib, TG_Region, PG_Color


BOX_TL = b'\xe2\x94\x8c'  # ┌ U+250C
BOX_TR = b'\xe2\x94\x90'  # ┐ U+2510
BOX_BL = b'\xe2\x94\x94'  # └ U+2514
BOX_BR = b'\xe2\x94\x98'  # ┘ U+2518
BOX_H  = b'\xe2\x94\x80'  # ─ U+2500
BOX_V  = b'\xe2\x94\x82'  # │ U+2502


def cell(col, row):
    return lib.pt_get_cell(col, row)

def glyph(col, row):
    c = cell(col, row)
    return bytes(c.ch[:c.ch_len])

def white():
    return PG_Color(1.0, 1.0, 1.0, 1.0)


class TestStrokeRect(unittest.TestCase):

    def setUp(self):
        lib.pt_init_headless(40, 20)

    def tearDown(self):
        lib.pt_shutdown()

    # ---- Esquinas ----

    def test_corner_top_left(self):
        lib.pt_stroke_rect(TG_Region(2, 2, 8, 5), white(), 1.0)
        self.assertEqual(glyph(2, 2), BOX_TL)

    def test_corner_top_right(self):
        lib.pt_stroke_rect(TG_Region(2, 2, 8, 5), white(), 1.0)
        self.assertEqual(glyph(9, 2), BOX_TR)

    def test_corner_bottom_left(self):
        lib.pt_stroke_rect(TG_Region(2, 2, 8, 5), white(), 1.0)
        self.assertEqual(glyph(2, 6), BOX_BL)

    def test_corner_bottom_right(self):
        lib.pt_stroke_rect(TG_Region(2, 2, 8, 5), white(), 1.0)
        self.assertEqual(glyph(9, 6), BOX_BR)

    # ---- Aristas ----

    def test_top_edge(self):
        lib.pt_stroke_rect(TG_Region(0, 0, 6, 4), white(), 1.0)
        for c in range(1, 5):
            self.assertEqual(glyph(c, 0), BOX_H, f"top edge at col {c}")

    def test_bottom_edge(self):
        lib.pt_stroke_rect(TG_Region(0, 0, 6, 4), white(), 1.0)
        for c in range(1, 5):
            self.assertEqual(glyph(c, 3), BOX_H, f"bottom edge at col {c}")

    def test_left_edge(self):
        lib.pt_stroke_rect(TG_Region(0, 0, 6, 4), white(), 1.0)
        for r in range(1, 3):
            self.assertEqual(glyph(0, r), BOX_V, f"left edge at row {r}")

    def test_right_edge(self):
        lib.pt_stroke_rect(TG_Region(0, 0, 6, 4), white(), 1.0)
        for r in range(1, 3):
            self.assertEqual(glyph(5, r), BOX_V, f"right edge at row {r}")

    # ---- Interior no tocado ----

    def test_interior_not_drawn(self):
        lib.pt_stroke_rect(TG_Region(0, 0, 6, 4), white(), 1.0)
        self.assertEqual(cell(2, 1).ch[0:1], b' ')
        self.assertEqual(cell(3, 2).ch[0:1], b' ')

    # ---- Color del borde ----

    def test_border_color(self):
        lib.pt_stroke_rect(TG_Region(0, 0, 4, 4), PG_Color(0, 1, 0, 1), 1.0)
        c = cell(0, 0)
        self.assertEqual(c.r_fg, 0)
        self.assertEqual(c.g_fg, 255)
        self.assertEqual(c.b_fg, 0)

    # ---- ch_len de los glifos de borde ----

    def test_box_chars_are_3_bytes(self):
        lib.pt_stroke_rect(TG_Region(0, 0, 4, 4), white(), 1.0)
        self.assertEqual(cell(0, 0).ch_len, 3)  # esquina
        self.assertEqual(cell(1, 0).ch_len, 3)  # arista horizontal
        self.assertEqual(cell(0, 1).ch_len, 3)  # arista vertical

    # ---- Degenerate: 1×1 (solo esquinas, sin aristas) ----

    def test_1x1_all_corners_overlap(self):
        lib.pt_stroke_rect(TG_Region(5, 5, 1, 1), white(), 1.0)
        # Con width=1, height=1: x0==x1 y y0==y1 → solo la esquina BR sobreescribe
        g = glyph(5, 5)
        self.assertIn(g, [BOX_TL, BOX_TR, BOX_BL, BOX_BR])

    # ---- Rect de 2 celdas: solo esquinas, sin aristas intermedias ----

    def test_2x2_only_corners(self):
        lib.pt_stroke_rect(TG_Region(0, 0, 2, 2), white(), 1.0)
        self.assertEqual(glyph(0, 0), BOX_TL)
        self.assertEqual(glyph(1, 0), BOX_TR)
        self.assertEqual(glyph(0, 1), BOX_BL)
        self.assertEqual(glyph(1, 1), BOX_BR)


if __name__ == "__main__":
    unittest.main()
