import ctypes
import unittest
from conftest import lib, Color, TextStyle, TG_Offset, TG_Region, DrawCommand

# Tipos de comando (deben coincidir con el enum en C)
PG_CMD_FILL_RECT      = 0
PG_CMD_STROKE_RECT    = 1
PG_CMD_DRAW_TEXT      = 2
PG_CMD_CLIP_PUSH      = 3
PG_CMD_CLIP_POP       = 4
PG_CMD_TRANSLATE_PUSH = 5
PG_CMD_TRANSLATE_POP  = 6

def reset():
    lib.pg_stub_reset()

class TestStubContext(unittest.TestCase):

    def setUp(self):
        reset()

    def test_reset_clears_commands(self):
        lib.pg_stub_fill_rect(TG_Region(0, 0, 100, 100), lib.pg_color_rgba(1, 0, 0, 1))
        lib.pg_stub_reset()
        self.assertEqual(lib.pg_stub_count(), 0)

    def test_fill_rect_recorded(self):
        lib.pg_stub_fill_rect(TG_Region(10, 20, 80, 40), lib.pg_color_rgba(1, 0, 0, 1))
        self.assertEqual(lib.pg_stub_count(), 1)

    def test_stroke_rect_recorded(self):
        lib.pg_stub_stroke_rect(TG_Region(0, 0, 50, 50), lib.pg_color_rgba(0, 1, 0, 1), 2.0)
        self.assertEqual(lib.pg_stub_count(), 1)

    def test_draw_text_recorded(self):
        color = lib.pg_color_rgba(1, 1, 1, 1)
        style = lib.pg_text_style(b"Inter", 14.0, color, 0, 0)
        lib.pg_stub_draw_text(TG_Offset(10, 20), b"Hello", style)
        self.assertEqual(lib.pg_stub_count(), 1)

    def test_clip_push_pop(self):
        lib.pg_stub_clip_push(TG_Region(0, 0, 200, 200))
        lib.pg_stub_clip_pop()
        self.assertEqual(lib.pg_stub_count(), 2)

    def test_translate_push_pop(self):
        lib.pg_stub_translate_push(TG_Offset(10, 20))
        lib.pg_stub_translate_pop()
        self.assertEqual(lib.pg_stub_count(), 2)

    def test_multiple_commands(self):
        lib.pg_stub_fill_rect(TG_Region(0, 0, 100, 100), lib.pg_color_rgba(1, 0, 0, 1))
        lib.pg_stub_stroke_rect(TG_Region(0, 0, 100, 100), lib.pg_color_rgba(0, 1, 0, 1), 1.0)
        lib.pg_stub_clip_push(TG_Region(0, 0, 200, 200))
        lib.pg_stub_clip_pop()
        self.assertEqual(lib.pg_stub_count(), 4)

    def test_get_fill_rect_content(self):
        region = TG_Region(10, 20, 80, 40)
        color  = lib.pg_color_rgba(1.0, 0.0, 0.0, 1.0)
        lib.pg_stub_fill_rect(region, color)
        cmd = lib.pg_stub_get(0)
        self.assertEqual(cmd.type, PG_CMD_FILL_RECT)
        self.assertEqual(cmd.data.fill_rect.region.x,     10)
        self.assertEqual(cmd.data.fill_rect.region.width, 80)
        self.assertAlmostEqual(cmd.data.fill_rect.color.r, 1.0, places=5)

    def test_get_draw_text_content(self):
        color = lib.pg_color_rgba(1.0, 1.0, 1.0, 1.0)
        style = lib.pg_text_style(b"Inter", 14.0, color, 0, 0)
        lib.pg_stub_draw_text(TG_Offset(5, 10), b"Hello", style)
        cmd = lib.pg_stub_get(0)
        self.assertEqual(cmd.type, PG_CMD_DRAW_TEXT)
        self.assertEqual(cmd.data.draw_text.text, b"Hello")

    def test_get_command_order(self):
        lib.pg_stub_clip_push(TG_Region(0, 0, 100, 100))
        lib.pg_stub_clip_pop()
        self.assertEqual(lib.pg_stub_get(0).type, PG_CMD_CLIP_PUSH)
        self.assertEqual(lib.pg_stub_get(1).type, PG_CMD_CLIP_POP)

if __name__ == "__main__":
    unittest.main()
