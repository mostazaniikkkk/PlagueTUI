import unittest
from conftest import lib, Color

class TestTextStyle(unittest.TestCase):

    def test_basic_fields(self):
        color = lib.pg_color_rgba(1.0, 1.0, 1.0, 1.0)
        s = lib.pg_text_style(b"Inter", 14.0, color, 0, 0)
        self.assertEqual(s.font_name, b"Inter")
        self.assertAlmostEqual(s.font_size, 14.0, places=5)
        self.assertEqual(s.bold,   0)
        self.assertEqual(s.italic, 0)

    def test_bold_italic(self):
        color = lib.pg_color_rgba(0.0, 0.0, 0.0, 1.0)
        s = lib.pg_text_style(b"Roboto", 16.0, color, 1, 1)
        self.assertEqual(s.bold,   1)
        self.assertEqual(s.italic, 1)

    def test_color_preserved(self):
        color = lib.pg_color_rgba(0.5, 0.3, 0.1, 0.9)
        s = lib.pg_text_style(b"Arial", 12.0, color, 0, 0)
        self.assertAlmostEqual(s.color.r, 0.5, places=5)
        self.assertAlmostEqual(s.color.g, 0.3, places=5)
        self.assertAlmostEqual(s.color.b, 0.1, places=5)
        self.assertAlmostEqual(s.color.a, 0.9, places=5)

    def test_font_name_truncated(self):
        # Names longer than 63 chars must be truncated without crash
        name = b"A" * 100
        color = lib.pg_color_rgba(1.0, 1.0, 1.0, 1.0)
        s = lib.pg_text_style(name, 10.0, color, 0, 0)
        self.assertEqual(len(s.font_name), 63)

if __name__ == "__main__":
    unittest.main()
