import unittest
from conftest import lib, TG_Region, TG_Offset, PG_Color, PG_TextStyle


def cell(col, row):
    return lib.pt_get_cell(col, row)

def red():
    return PG_Color(1.0, 0.0, 0.0, 1.0)

def blue():
    return PG_Color(0.0, 0.0, 1.0, 1.0)

def white():
    return PG_Color(1.0, 1.0, 1.0, 1.0)


class TestFillRect(unittest.TestCase):

    def setUp(self):
        lib.pt_init_headless(80, 24)

    def tearDown(self):
        lib.pt_shutdown()

    def test_fill_sets_bg_color(self):
        lib.pt_fill_rect(TG_Region(0, 0, 10, 5), red())
        c = cell(5, 2)
        self.assertEqual(c.r_bg, 255)
        self.assertEqual(c.g_bg, 0)
        self.assertEqual(c.b_bg, 0)

    def test_fill_sets_space_glyph(self):
        lib.pt_fill_rect(TG_Region(0, 0, 10, 5), red())
        c = cell(3, 3)
        self.assertEqual(c.ch[0:1], b' ')
        self.assertEqual(c.ch_len, 1)

    def test_fill_does_not_exceed_region(self):
        lib.pt_fill_rect(TG_Region(5, 5, 10, 10), red())
        # celda justo fuera del borde derecho/inferior
        outside_right  = cell(15, 7)
        outside_bottom = cell(7, 15)
        self.assertEqual(outside_right.r_bg,  0)
        self.assertEqual(outside_bottom.r_bg, 0)

    def test_fill_starts_at_origin(self):
        lib.pt_fill_rect(TG_Region(5, 5, 10, 10), red())
        # celda justo antes del origen
        before_left = cell(4, 7)
        before_top  = cell(7, 4)
        self.assertEqual(before_left.r_bg, 0)
        self.assertEqual(before_top.r_bg,  0)

    def test_transparent_fill_is_ignored(self):
        lib.pt_fill_rect(TG_Region(0, 0, 10, 10), PG_Color(1, 0, 0, 0.0))
        self.assertEqual(cell(5, 5).r_bg, 0)

    def test_color_conversion_blue(self):
        lib.pt_fill_rect(TG_Region(0, 0, 1, 1), blue())
        c = cell(0, 0)
        self.assertEqual(c.r_bg, 0)
        self.assertEqual(c.g_bg, 0)
        self.assertEqual(c.b_bg, 255)

    def test_two_fills_second_wins(self):
        lib.pt_fill_rect(TG_Region(0, 0, 5, 5), red())
        lib.pt_fill_rect(TG_Region(0, 0, 5, 5), blue())
        c = cell(2, 2)
        self.assertEqual(c.r_bg, 0)
        self.assertEqual(c.b_bg, 255)


class TestDrawText(unittest.TestCase):

    def setUp(self):
        lib.pt_init_headless(80, 24)

    def tearDown(self):
        lib.pt_shutdown()

    def _style(self, color=None, bold=0, italic=0):
        s = PG_TextStyle()
        s.font_name = b"mono"
        s.font_size = 12.0
        s.color     = color or white()
        s.bold      = bold
        s.italic    = italic
        return s

    def test_text_sets_characters(self):
        lib.pt_draw_text(TG_Offset(0, 0), b"Hello", self._style())
        self.assertEqual(cell(0, 0).ch[0:1], b'H')
        self.assertEqual(cell(1, 0).ch[0:1], b'e')
        self.assertEqual(cell(4, 0).ch[0:1], b'o')

    def test_text_sets_fg_color(self):
        lib.pt_draw_text(TG_Offset(0, 0), b"A", self._style(red()))
        c = cell(0, 0)
        self.assertEqual(c.r_fg, 255)
        self.assertEqual(c.g_fg, 0)
        self.assertEqual(c.b_fg, 0)

    def test_text_bold_flag(self):
        lib.pt_draw_text(TG_Offset(0, 0), b"B", self._style(bold=1))
        self.assertEqual(cell(0, 0).bold, 1)

    def test_text_italic_flag(self):
        lib.pt_draw_text(TG_Offset(0, 0), b"I", self._style(italic=1))
        self.assertEqual(cell(0, 0).italic, 1)

    def test_text_advances_columns(self):
        lib.pt_draw_text(TG_Offset(3, 2), b"AB", self._style())
        self.assertEqual(cell(3, 2).ch[0:1], b'A')
        self.assertEqual(cell(4, 2).ch[0:1], b'B')

    def test_text_utf8_multibyte(self):
        # "é" = U+00E9 = 0xC3 0xA9 (2 bytes UTF-8)
        lib.pt_draw_text(TG_Offset(0, 0), "é".encode("utf-8"), self._style())
        c = cell(0, 0)
        self.assertEqual(c.ch_len, 2)
        self.assertEqual(c.ch[0:2], "é".encode("utf-8"))

    def test_text_does_not_overflow_row(self):
        # Texto que empieza antes del límite del buffer
        lib.pt_draw_text(TG_Offset(78, 0), b"XYZ", self._style())
        self.assertEqual(cell(78, 0).ch[0:1], b'X')
        self.assertEqual(cell(79, 0).ch[0:1], b'Y')
        # col 80 está fuera del buffer (80 cols: 0-79)
        outside = cell(80, 0)
        self.assertEqual(outside.ch[0:1], b' ')


class TestClipTranslate(unittest.TestCase):

    def setUp(self):
        lib.pt_init_headless(80, 24)

    def tearDown(self):
        lib.pt_shutdown()

    def test_clip_restricts_fill(self):
        lib.pt_clip_push(TG_Region(5, 5, 10, 10))
        lib.pt_fill_rect(TG_Region(0, 0, 80, 24), red())
        lib.pt_clip_pop()
        # Dentro del clip → rojo
        self.assertEqual(cell(10, 10).r_bg, 255)
        # Fuera del clip → sin color
        self.assertEqual(cell(0, 0).r_bg, 0)
        self.assertEqual(cell(20, 20).r_bg, 0)

    def test_translate_shifts_fill(self):
        lib.pt_translate_push(TG_Offset(10, 5))
        lib.pt_fill_rect(TG_Region(0, 0, 5, 5), red())
        lib.pt_translate_pop()
        # (0,0) local + (10,5) translate = (10,5) screen
        self.assertEqual(cell(10, 5).r_bg, 255)
        # Sin translate, (0,0) no fue tocado
        self.assertEqual(cell(0, 0).r_bg, 0)

    def test_translate_stack(self):
        lib.pt_translate_push(TG_Offset(5, 0))
        lib.pt_translate_push(TG_Offset(3, 0))
        lib.pt_fill_rect(TG_Region(0, 0, 1, 1), red())
        lib.pt_translate_pop()
        lib.pt_translate_pop()
        # (5+3)=8 de desplazamiento
        self.assertEqual(cell(8, 0).r_bg, 255)
        self.assertEqual(cell(5, 0).r_bg, 0)

    def test_clip_pop_restores(self):
        lib.pt_clip_push(TG_Region(0, 0, 5, 5))
        lib.pt_clip_pop()
        lib.pt_fill_rect(TG_Region(10, 10, 5, 5), red())
        # Después del pop, la región fuera del clip anterior es alcanzable
        self.assertEqual(cell(12, 12).r_bg, 255)


if __name__ == "__main__":
    unittest.main()
