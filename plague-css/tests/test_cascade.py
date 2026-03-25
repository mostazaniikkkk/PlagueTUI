import unittest
from conftest import (
    load, free, compute,
    PC_HAS_COLOR, PC_HAS_BACKGROUND, PC_HAS_BORDER, PC_HAS_MARGIN,
    PC_HAS_PADDING, PC_HAS_WIDTH, PC_HAS_HEIGHT, PC_HAS_DOCK,
    PC_HAS_DISPLAY, PC_HAS_OVERFLOW, PC_HAS_BOLD, PC_HAS_ITALIC,
    PC_HAS_UNDERLINE,
    PC_BORDER_NONE, PC_BORDER_SOLID, PC_BORDER_ROUNDED,
    PC_STATE_HOVER, PC_STATE_FOCUS, PC_STATE_PRESSED,
    PL_SIZE_FIXED, PL_SIZE_FRACTION,
)


# ---------------------------------------------------------------------------
# Selector matching
# ---------------------------------------------------------------------------

class TestSelectorMatching(unittest.TestCase):

    def test_type_selector_matches(self):
        h = load("Button { color: white; }")
        s = compute(h, type_name="Button")
        self.assertTrue(s.set_flags & PC_HAS_COLOR)
        free(h)

    def test_type_selector_no_match(self):
        h = load("Button { color: white; }")
        s = compute(h, type_name="Label")
        self.assertFalse(s.set_flags & PC_HAS_COLOR)
        free(h)

    def test_class_selector_matches(self):
        h = load(".primary { background: blue; }")
        s = compute(h, classes="primary")
        self.assertTrue(s.set_flags & PC_HAS_BACKGROUND)
        free(h)

    def test_class_selector_no_match(self):
        h = load(".primary { background: blue; }")
        s = compute(h, classes="secondary")
        self.assertFalse(s.set_flags & PC_HAS_BACKGROUND)
        free(h)

    def test_class_among_multiple(self):
        h = load(".primary { background: blue; }")
        s = compute(h, classes="large primary active")
        self.assertTrue(s.set_flags & PC_HAS_BACKGROUND)
        free(h)

    def test_id_selector_matches(self):
        h = load("#my-btn { width: 20; }")
        s = compute(h, id_="my-btn")
        self.assertTrue(s.set_flags & PC_HAS_WIDTH)
        free(h)

    def test_id_selector_no_match(self):
        h = load("#my-btn { width: 20; }")
        s = compute(h, id_="other-btn")
        self.assertFalse(s.set_flags & PC_HAS_WIDTH)
        free(h)

    def test_pseudo_hover_matches_with_state(self):
        h = load("Button:hover { background: red; }")
        s = compute(h, type_name="Button", state=PC_STATE_HOVER)
        self.assertTrue(s.set_flags & PC_HAS_BACKGROUND)
        free(h)

    def test_pseudo_hover_no_match_without_state(self):
        h = load("Button:hover { background: red; }")
        s = compute(h, type_name="Button", state=0)
        self.assertFalse(s.set_flags & PC_HAS_BACKGROUND)
        free(h)

    def test_pseudo_focus(self):
        h = load("Input:focus { border: solid cyan; }")
        s = compute(h, type_name="Input", state=PC_STATE_FOCUS)
        self.assertTrue(s.set_flags & PC_HAS_BORDER)
        free(h)

    def test_universal_selector_matches_any_type(self):
        h = load("* { color: white; }")
        s1 = compute(h, type_name="Button")
        s2 = compute(h, type_name="Label")
        s3 = compute(h, type_name="")
        self.assertTrue(s1.set_flags & PC_HAS_COLOR)
        self.assertTrue(s2.set_flags & PC_HAS_COLOR)
        self.assertTrue(s3.set_flags & PC_HAS_COLOR)
        free(h)

    def test_compound_type_class(self):
        h = load("Button.primary { color: blue; }")
        # type=Button, class=primary → match
        s = compute(h, type_name="Button", classes="primary")
        self.assertTrue(s.set_flags & PC_HAS_COLOR)
        free(h)

    def test_compound_type_class_partial_no_match(self):
        h = load("Button.primary { color: blue; }")
        # type=Button pero sin clase primary → no coincide
        s = compute(h, type_name="Button", classes="secondary")
        self.assertFalse(s.set_flags & PC_HAS_COLOR)
        free(h)


# ---------------------------------------------------------------------------
# Valores de color
# ---------------------------------------------------------------------------

class TestColorValues(unittest.TestCase):

    def test_named_color_white(self):
        h = load("Button { color: white; }")
        s = compute(h, type_name="Button")
        self.assertEqual(s.color.r, 255)
        self.assertEqual(s.color.g, 255)
        self.assertEqual(s.color.b, 255)
        self.assertEqual(s.color.a, 255)
        free(h)

    def test_named_color_black(self):
        h = load("Button { background: black; }")
        s = compute(h, type_name="Button")
        self.assertEqual(s.background.r, 0)
        self.assertEqual(s.background.g, 0)
        self.assertEqual(s.background.b, 0)
        free(h)

    def test_hex_color_rrggbb(self):
        h = load("Screen { background: #336699; }")
        s = compute(h, type_name="Screen")
        self.assertEqual(s.background.r, 0x33)
        self.assertEqual(s.background.g, 0x66)
        self.assertEqual(s.background.b, 0x99)
        self.assertEqual(s.background.a, 255)
        free(h)

    def test_hex_color_rgb_short(self):
        h = load("Screen { background: #f0a; }")
        s = compute(h, type_name="Screen")
        self.assertEqual(s.background.r, 0xff)
        self.assertEqual(s.background.g, 0x00)
        self.assertEqual(s.background.b, 0xaa)
        free(h)

    def test_transparent_color(self):
        h = load("Button { background: transparent; }")
        s = compute(h, type_name="Button")
        self.assertEqual(s.background.a, 0)
        free(h)


# ---------------------------------------------------------------------------
# Tamaños
# ---------------------------------------------------------------------------

class TestSizeValues(unittest.TestCase):

    def test_width_fixed(self):
        h = load("Button { width: 20; }")
        s = compute(h, type_name="Button")
        self.assertTrue(s.set_flags & PC_HAS_WIDTH)
        self.assertEqual(s.width.type,  PL_SIZE_FIXED)
        self.assertEqual(s.width.value, 20)
        free(h)

    def test_height_fixed(self):
        h = load("Button { height: 3; }")
        s = compute(h, type_name="Button")
        self.assertEqual(s.height.type,  PL_SIZE_FIXED)
        self.assertEqual(s.height.value, 3)
        free(h)

    def test_width_fraction(self):
        h = load("Button { width: 1fr; }")
        s = compute(h, type_name="Button")
        self.assertEqual(s.width.type,  PL_SIZE_FRACTION)
        self.assertEqual(s.width.value, 1)
        free(h)

    def test_width_fraction_2(self):
        h = load("Button { width: 2fr; }")
        s = compute(h, type_name="Button")
        self.assertEqual(s.width.type,  PL_SIZE_FRACTION)
        self.assertEqual(s.width.value, 2)
        free(h)

    def test_width_auto(self):
        h = load("Button { width: auto; }")
        s = compute(h, type_name="Button")
        self.assertEqual(s.width.type, PL_SIZE_FRACTION)
        free(h)


# ---------------------------------------------------------------------------
# Spacing (margin / padding)
# ---------------------------------------------------------------------------

class TestSpacingValues(unittest.TestCase):

    def test_padding_uniform(self):
        h = load("Button { padding: 2; }")
        s = compute(h, type_name="Button")
        self.assertTrue(s.set_flags & PC_HAS_PADDING)
        self.assertEqual(s.padding.top,    2)
        self.assertEqual(s.padding.right,  2)
        self.assertEqual(s.padding.bottom, 2)
        self.assertEqual(s.padding.left,   2)
        free(h)

    def test_padding_two_values(self):
        h = load("Button { padding: 1 2; }")
        s = compute(h, type_name="Button")
        self.assertEqual(s.padding.top,    1)
        self.assertEqual(s.padding.bottom, 1)
        self.assertEqual(s.padding.right,  2)
        self.assertEqual(s.padding.left,   2)
        free(h)

    def test_margin_four_values(self):
        h = load("Button { margin: 1 2 3 4; }")
        s = compute(h, type_name="Button")
        self.assertTrue(s.set_flags & PC_HAS_MARGIN)
        self.assertEqual(s.margin.top,    1)
        self.assertEqual(s.margin.right,  2)
        self.assertEqual(s.margin.bottom, 3)
        self.assertEqual(s.margin.left,   4)
        free(h)


# ---------------------------------------------------------------------------
# Dock
# ---------------------------------------------------------------------------

class TestDockValue(unittest.TestCase):

    def test_dock_top(self):
        h = load("Header { dock: top; }")
        s = compute(h, type_name="Header")
        self.assertTrue(s.set_flags & PC_HAS_DOCK)
        self.assertEqual(s.dock, 0)  # PL_DOCK_TOP
        free(h)

    def test_dock_bottom(self):
        h = load("Footer { dock: bottom; }")
        s = compute(h, type_name="Footer")
        self.assertEqual(s.dock, 1)  # PL_DOCK_BOTTOM
        free(h)

    def test_dock_left(self):
        h = load("Sidebar { dock: left; }")
        s = compute(h, type_name="Sidebar")
        self.assertEqual(s.dock, 2)
        free(h)

    def test_dock_right(self):
        h = load("Panel { dock: right; }")
        s = compute(h, type_name="Panel")
        self.assertEqual(s.dock, 3)
        free(h)


# ---------------------------------------------------------------------------
# Display / overflow
# ---------------------------------------------------------------------------

class TestDisplayOverflow(unittest.TestCase):

    def test_display_none(self):
        h = load("Button { display: none; }")
        s = compute(h, type_name="Button")
        self.assertTrue(s.set_flags & PC_HAS_DISPLAY)
        self.assertEqual(s.display, 0)
        free(h)

    def test_display_block(self):
        h = load("Button { display: block; }")
        s = compute(h, type_name="Button")
        self.assertEqual(s.display, 1)
        free(h)

    def test_overflow_scroll(self):
        h = load("ListView { overflow: scroll; }")
        s = compute(h, type_name="ListView")
        self.assertTrue(s.set_flags & PC_HAS_OVERFLOW)
        self.assertEqual(s.overflow, 1)
        free(h)

    def test_overflow_hidden(self):
        h = load("Widget { overflow: hidden; }")
        s = compute(h, type_name="Widget")
        self.assertEqual(s.overflow, 0)
        free(h)


# ---------------------------------------------------------------------------
# Borde
# ---------------------------------------------------------------------------

class TestBorderValue(unittest.TestCase):

    def test_border_none(self):
        h = load("Button { border: none; }")
        s = compute(h, type_name="Button")
        self.assertTrue(s.set_flags & PC_HAS_BORDER)
        self.assertEqual(s.border_style, PC_BORDER_NONE)
        free(h)

    def test_border_solid_white(self):
        h = load("Button { border: solid white; }")
        s = compute(h, type_name="Button")
        self.assertEqual(s.border_style,   PC_BORDER_SOLID)
        self.assertEqual(s.border_color.r, 255)
        self.assertEqual(s.border_color.g, 255)
        self.assertEqual(s.border_color.b, 255)
        free(h)

    def test_border_rounded_hex(self):
        h = load("Panel { border: rounded #336699; }")
        s = compute(h, type_name="Panel")
        self.assertEqual(s.border_style,   PC_BORDER_ROUNDED)
        self.assertEqual(s.border_color.r, 0x33)
        free(h)


# ---------------------------------------------------------------------------
# Estilo de texto
# ---------------------------------------------------------------------------

class TestTextStyle(unittest.TestCase):

    def test_text_style_bold(self):
        h = load("Button { text-style: bold; }")
        s = compute(h, type_name="Button")
        self.assertTrue(s.set_flags & PC_HAS_BOLD)
        self.assertEqual(s.bold, 1)
        free(h)

    def test_text_style_italic(self):
        h = load("Label { text-style: italic; }")
        s = compute(h, type_name="Label")
        self.assertEqual(s.italic, 1)
        free(h)

    def test_text_style_underline(self):
        h = load("Label { text-style: underline; }")
        s = compute(h, type_name="Label")
        self.assertEqual(s.underline, 1)
        free(h)

    def test_text_style_combined(self):
        h = load("Label { text-style: bold italic; }")
        s = compute(h, type_name="Label")
        self.assertEqual(s.bold,   1)
        self.assertEqual(s.italic, 1)
        free(h)

    def test_text_style_none(self):
        h = load("Label { text-style: none; }")
        s = compute(h, type_name="Label")
        self.assertEqual(s.bold,      0)
        self.assertEqual(s.italic,    0)
        self.assertEqual(s.underline, 0)
        free(h)


# ---------------------------------------------------------------------------
# Especificidad y cascada
# ---------------------------------------------------------------------------

class TestSpecificity(unittest.TestCase):

    def test_id_overrides_type(self):
        """#id (especificidad 100) vence a tipo (especificidad 1)."""
        tcss = "Button { color: white; } #my-btn { color: red; }"
        h = load(tcss)
        s = compute(h, type_name="Button", id_="my-btn")
        # color rojo = (255,0,0)
        self.assertEqual(s.color.r, 255)
        self.assertEqual(s.color.g,   0)
        self.assertEqual(s.color.b,   0)
        free(h)

    def test_class_overrides_type(self):
        tcss = "Button { background: black; } .primary { background: blue; }"
        h = load(tcss)
        s = compute(h, type_name="Button", classes="primary")
        self.assertEqual(s.background.b, 255)  # azul gana
        free(h)

    def test_later_rule_same_specificity_wins(self):
        """Misma especificidad: la última regla en la hoja gana."""
        tcss = "Button { color: white; } Button { color: red; }"
        h = load(tcss)
        s = compute(h, type_name="Button")
        self.assertEqual(s.color.r, 255)
        self.assertEqual(s.color.g,   0)
        free(h)

    def test_no_match_returns_defaults(self):
        h = load("Button { color: white; }")
        s = compute(h, type_name="Label")
        self.assertEqual(s.set_flags, 0)
        self.assertEqual(s.display,   1)   # display por defecto = block
        self.assertEqual(s.dock,     -1)   # sin dock por defecto
        free(h)


if __name__ == "__main__":
    unittest.main()
