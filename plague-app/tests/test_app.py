import unittest
from conftest import (
    lib,
    PA_NO_PARENT,
    PL_LAYOUT_DOCK, PL_LAYOUT_VERTICAL, PL_LAYOUT_HORIZONTAL,
    PL_DOCK_TOP, PL_DOCK_BOTTOM, PL_DOCK_LEFT, PL_DOCK_RIGHT, PL_DOCK_CENTER,
    fixed, fr, no_spacing, spacing,
)


class TestInit(unittest.TestCase):

    def setUp(self):
        lib.pa_init_headless(80, 24)

    def tearDown(self):
        lib.pa_shutdown()

    def test_init_headless_returns_one(self):
        lib.pa_shutdown()
        result = lib.pa_init_headless(80, 24)
        self.assertEqual(result, 1)

    def test_get_cols_rows(self):
        self.assertEqual(lib.pa_get_cols(), 80)
        self.assertEqual(lib.pa_get_rows(), 24)

    def test_init_headless_different_size(self):
        lib.pa_shutdown()
        lib.pa_init_headless(120, 40)
        self.assertEqual(lib.pa_get_cols(), 120)
        self.assertEqual(lib.pa_get_rows(),  40)


class TestWidgets(unittest.TestCase):

    def setUp(self):
        lib.pa_init_headless(80, 24)

    def tearDown(self):
        lib.pa_shutdown()

    def test_add_root_widget(self):
        wid = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        self.assertEqual(wid, 0)

    def test_add_child_widget(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        child = lib.pa_widget_add(b"Header", b"header", b"", root)
        self.assertEqual(child, 1)

    def test_widget_ids_sequential(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        w1   = lib.pa_widget_add(b"Header", b"", b"", root)
        w2   = lib.pa_widget_add(b"Footer", b"", b"", root)
        w3   = lib.pa_widget_add(b"Main",   b"", b"", root)
        self.assertEqual(w1, 1)
        self.assertEqual(w2, 2)
        self.assertEqual(w3, 3)


class TestLayout(unittest.TestCase):

    def setUp(self):
        lib.pa_init_headless(80, 24)

    def tearDown(self):
        lib.pa_shutdown()

    def test_root_fills_screen(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_layout(root, PL_LAYOUT_DOCK)
        lib.pa_render()
        r = lib.pa_widget_region(root)
        self.assertEqual(r.x,      0)
        self.assertEqual(r.y,      0)
        self.assertEqual(r.width,  80)
        self.assertEqual(r.height, 24)

    def test_docked_header_top(self):
        root   = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_layout(root, PL_LAYOUT_DOCK)
        header = lib.pa_widget_add(b"Header", b"", b"", root)
        lib.pa_widget_set_dock(header, PL_DOCK_TOP)
        lib.pa_widget_set_size(header, fr(1), fixed(3))
        lib.pa_render()
        r = lib.pa_widget_region(header)
        self.assertEqual(r.y,      0)
        self.assertEqual(r.height, 3)
        self.assertEqual(r.width,  80)

    def test_docked_footer_bottom(self):
        root   = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_layout(root, PL_LAYOUT_DOCK)
        footer = lib.pa_widget_add(b"Footer", b"", b"", root)
        lib.pa_widget_set_dock(footer, PL_DOCK_BOTTOM)
        lib.pa_widget_set_size(footer, fr(1), fixed(1))
        lib.pa_render()
        r = lib.pa_widget_region(footer)
        self.assertEqual(r.y,      23)   # 24-1
        self.assertEqual(r.height, 1)

    def test_docked_sidebar_left(self):
        root    = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_layout(root, PL_LAYOUT_DOCK)
        sidebar = lib.pa_widget_add(b"Sidebar", b"", b"", root)
        lib.pa_widget_set_dock(sidebar, PL_DOCK_LEFT)
        lib.pa_widget_set_size(sidebar, fixed(20), fr(1))
        lib.pa_render()
        r = lib.pa_widget_region(sidebar)
        self.assertEqual(r.x,     0)
        self.assertEqual(r.width, 20)

    def test_center_fills_remaining(self):
        root   = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_layout(root, PL_LAYOUT_DOCK)
        header = lib.pa_widget_add(b"Header", b"", b"", root)
        lib.pa_widget_set_dock(header, PL_DOCK_TOP)
        lib.pa_widget_set_size(header, fr(1), fixed(3))
        footer = lib.pa_widget_add(b"Footer", b"", b"", root)
        lib.pa_widget_set_dock(footer, PL_DOCK_BOTTOM)
        lib.pa_widget_set_size(footer, fr(1), fixed(1))
        main   = lib.pa_widget_add(b"Main", b"", b"", root)
        lib.pa_widget_set_dock(main, PL_DOCK_CENTER)
        lib.pa_render()
        r = lib.pa_widget_region(main)
        self.assertEqual(r.y,      3)
        self.assertEqual(r.height, 20)   # 24-3-1
        self.assertEqual(r.width,  80)

    def test_full_layout(self):
        """Header + Footer + Sidebar + Main."""
        root    = lib.pa_widget_add(b"Screen",  b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_layout(root, PL_LAYOUT_DOCK)
        header  = lib.pa_widget_add(b"Header",  b"", b"", root)
        lib.pa_widget_set_dock(header,  PL_DOCK_TOP);    lib.pa_widget_set_size(header,  fr(1), fixed(3))
        footer  = lib.pa_widget_add(b"Footer",  b"", b"", root)
        lib.pa_widget_set_dock(footer,  PL_DOCK_BOTTOM); lib.pa_widget_set_size(footer,  fr(1), fixed(1))
        sidebar = lib.pa_widget_add(b"Sidebar", b"", b"", root)
        lib.pa_widget_set_dock(sidebar, PL_DOCK_LEFT);   lib.pa_widget_set_size(sidebar, fixed(20), fr(1))
        main    = lib.pa_widget_add(b"Main",    b"", b"", root)
        lib.pa_widget_set_dock(main,    PL_DOCK_CENTER)
        lib.pa_render()

        rs = lib.pa_widget_region(sidebar)
        self.assertEqual(rs.y,      3)   # debajo del header
        self.assertEqual(rs.height, 20)  # 24-3-1
        self.assertEqual(rs.width,  20)

        rm = lib.pa_widget_region(main)
        self.assertEqual(rm.x,      20)
        self.assertEqual(rm.width,  60)  # 80-20
        self.assertEqual(rm.height, 20)


class TestCSS(unittest.TestCase):

    def setUp(self):
        lib.pa_init_headless(80, 24)

    def tearDown(self):
        lib.pa_shutdown()

    def test_css_load_returns_handle(self):
        h = lib.pa_css_load(b"Screen { background: darkblue; }")
        self.assertGreater(h, 0)

    def test_render_with_css_does_not_crash(self):
        lib.pa_css_load(b"""
            Screen  { background: #1e1e2e; }
            Header  { background: #313244; }
        """)
        root   = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_layout(root, PL_LAYOUT_DOCK)
        header = lib.pa_widget_add(b"Header", b"", b"", root)
        lib.pa_widget_set_dock(header, PL_DOCK_TOP)
        lib.pa_widget_set_size(header, fr(1), fixed(3))
        lib.pa_widget_set_text(header, b"PlagueTUI")
        lib.pa_render()  # no debe crashear


class TestBindings(unittest.TestCase):

    def setUp(self):
        lib.pa_init_headless(80, 24)

    def tearDown(self):
        lib.pa_shutdown()

    def test_bind_key_returns_nonzero(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        bid  = lib.pa_bind_key(root, ord('q'), 0)
        self.assertGreater(bid, 0)

    def test_poll_no_event_returns_zero(self):
        lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        result = lib.pa_poll()
        self.assertEqual(result, 0)

    def test_quit_makes_poll_return_minus_one(self):
        lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_quit()
        result = lib.pa_poll()
        self.assertEqual(result, -1)


if __name__ == "__main__":
    unittest.main()
