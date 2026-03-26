import unittest
from conftest import (
    lib,
    PA_NO_PARENT,
    PA_STATE_HOVER, PA_STATE_FOCUSED, PA_STATE_PRESSED, PA_STATE_DISABLED,
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


class TestFocus(unittest.TestCase):

    def setUp(self):
        lib.pa_init_headless(80, 24)

    def tearDown(self):
        lib.pa_shutdown()

    def test_focus_get_initial_is_minus_one(self):
        self.assertEqual(lib.pa_focus_get(), -1)

    def test_focus_set_and_get(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_focusable(root, 1)
        lib.pa_focus_set(root)
        self.assertEqual(lib.pa_focus_get(), root)

    def test_focus_next_cycles_focusable_widgets(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        w1   = lib.pa_widget_add(b"A", b"", b"", root)
        w2   = lib.pa_widget_add(b"B", b"", b"", root)
        lib.pa_widget_set_focusable(w1, 1)
        lib.pa_widget_set_focusable(w2, 1)
        lib.pa_focus_next()
        self.assertEqual(lib.pa_focus_get(), w1)
        lib.pa_focus_next()
        self.assertEqual(lib.pa_focus_get(), w2)
        lib.pa_focus_next()
        self.assertEqual(lib.pa_focus_get(), w1)   # wrap-around

    def test_focus_prev_cycles_backwards(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        w1   = lib.pa_widget_add(b"A", b"", b"", root)
        w2   = lib.pa_widget_add(b"B", b"", b"", root)
        lib.pa_widget_set_focusable(w1, 1)
        lib.pa_widget_set_focusable(w2, 1)
        lib.pa_focus_prev()
        self.assertEqual(lib.pa_focus_get(), w2)   # desde -1, prev → último

    def test_disabled_widget_skipped_by_focus(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        w1   = lib.pa_widget_add(b"A", b"", b"", root)
        w2   = lib.pa_widget_add(b"B", b"", b"", root)
        lib.pa_widget_set_focusable(w1, 1)
        lib.pa_widget_set_focusable(w2, 1)
        lib.pa_widget_set_disabled(w1, 1)
        lib.pa_focus_next()
        self.assertEqual(lib.pa_focus_get(), w2)   # w1 deshabilitado, salta

    def test_no_focusable_widgets_focus_stays_minus_one(self):
        lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_focus_next()
        self.assertEqual(lib.pa_focus_get(), -1)


class TestClickBinding(unittest.TestCase):

    def setUp(self):
        lib.pa_init_headless(80, 24)

    def tearDown(self):
        lib.pa_shutdown()

    def test_bind_click_returns_nonzero(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        bid  = lib.pa_bind_click(root)
        self.assertGreater(bid, 0)

    def test_bind_click_ids_are_unique(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        w1   = lib.pa_widget_add(b"A", b"", b"", root)
        w2   = lib.pa_widget_add(b"B", b"", b"", root)
        b1   = lib.pa_bind_click(w1)
        b2   = lib.pa_bind_click(w2)
        self.assertNotEqual(b1, b2)

    def test_bind_click_range_different_from_key_bindings(self):
        root  = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        kbid  = lib.pa_bind_key(root, ord('q'), 0)
        cbid  = lib.pa_bind_click(root)
        self.assertNotEqual(kbid, cbid)


class TestWidgetProperties(unittest.TestCase):

    def setUp(self):
        lib.pa_init_headless(80, 24)

    def tearDown(self):
        lib.pa_shutdown()

    def test_set_focusable_does_not_crash(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_focusable(root, 1)
        lib.pa_widget_set_focusable(root, 0)

    def test_set_disabled_does_not_crash(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_disabled(root, 1)
        lib.pa_widget_set_disabled(root, 0)

    def test_set_overlay_does_not_crash(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_overlay(root, 1)

    def test_scroll_to_does_not_crash(self):
        root = lib.pa_widget_add(b"Screen", b"", b"", PA_NO_PARENT)
        lib.pa_widget_scroll_to(root, 5, 10)

    def test_render_with_overlay_does_not_crash(self):
        root    = lib.pa_widget_add(b"Screen",  b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_layout(root, PL_LAYOUT_DOCK)
        main    = lib.pa_widget_add(b"Main",    b"", b"", root)
        lib.pa_widget_set_dock(main, PL_DOCK_CENTER)
        toast   = lib.pa_widget_add(b"Toast",   b"", b"", root)
        lib.pa_widget_set_overlay(toast, 1)
        lib.pa_widget_set_size(toast, fixed(20), fixed(3))
        lib.pa_render()


class TestTimers(unittest.TestCase):

    def setUp(self):
        lib.pa_init_headless(80, 24)

    def tearDown(self):
        lib.pa_shutdown()

    def test_timer_create_returns_nonzero(self):
        tid = lib.pa_timer_create(100, 0)
        self.assertGreater(tid, 0)

    def test_timer_ids_are_unique(self):
        t1 = lib.pa_timer_create(100, 0)
        t2 = lib.pa_timer_create(200, 0)
        self.assertNotEqual(t1, t2)

    def test_one_shot_timer_fires_once(self):
        tid = lib.pa_timer_create(100, 0)   # one-shot, 100 ms
        result = lib.pa_tick_timers(50)
        self.assertEqual(result, 0)          # no ha expirado
        result = lib.pa_tick_timers(60)      # total 110 ms → expira
        self.assertEqual(result, tid)

    def test_repeat_timer_fires_multiple_times(self):
        tid = lib.pa_timer_create(50, 1)    # periódico, 50 ms
        lib.pa_tick_timers(60)              # primera vez
        fired = lib.pa_tick_timers(60)     # segunda vez
        self.assertEqual(fired, tid)

    def test_cancel_timer_stops_firing(self):
        tid = lib.pa_timer_create(50, 1)
        lib.pa_timer_cancel(tid)
        result = lib.pa_tick_timers(100)
        self.assertEqual(result, 0)         # cancelado, no dispara


class TestVisibility(unittest.TestCase):

    def setUp(self):
        lib.pa_init_headless(80, 24)

    def tearDown(self):
        lib.pa_shutdown()

    def test_widget_visible_by_default(self):
        wid = lib.pa_widget_add(b"box", b"", b"", PA_NO_PARENT)
        # Default visible=1 — render must not crash
        lib.pa_render()

    def test_set_invisible(self):
        wid = lib.pa_widget_add(b"box", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_visible(wid, 0)
        lib.pa_render()

    def test_set_visible_again(self):
        wid = lib.pa_widget_add(b"box", b"", b"", PA_NO_PARENT)
        lib.pa_widget_set_visible(wid, 0)
        lib.pa_widget_set_visible(wid, 1)
        lib.pa_render()

    def test_invalid_wid_safe(self):
        lib.pa_widget_set_visible(-1,  0)
        lib.pa_widget_set_visible(999, 0)


if __name__ == "__main__":
    unittest.main()
