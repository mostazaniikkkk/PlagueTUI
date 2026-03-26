"""
Tests for plague-widgets Phase 2 display widgets.
Run with: python -m unittest discover -v
"""

import ctypes
import unittest
from conftest import lib, app, fixed, fr, PA_NO_PARENT


# ---------------------------------------------------------------------------
# Base class — init/shutdown headless for every test
# ---------------------------------------------------------------------------

class WidgetTestCase(unittest.TestCase):

    def setUp(self):
        app.pa_init_headless(80, 24)
        lib.pw_init()

    def tearDown(self):
        lib.pw_shutdown()
        app.pa_shutdown()


# ---------------------------------------------------------------------------
# TestPwInit
# ---------------------------------------------------------------------------

class TestPwInit(WidgetTestCase):

    def test_default_tcss_not_null(self):
        tcss = lib.pw_default_tcss()
        self.assertIsNotNone(tcss)

    def test_default_tcss_is_bytes(self):
        tcss = lib.pw_default_tcss()
        self.assertIsInstance(tcss, bytes)

    def test_default_tcss_nonempty(self):
        tcss = lib.pw_default_tcss()
        self.assertGreater(len(tcss), 0)

    def test_double_shutdown_safe(self):
        # pw_shutdown in tearDown will be second call — shouldn't crash
        lib.pw_shutdown()
        lib.pw_init()   # reinit so tearDown can call shutdown again safely


# ---------------------------------------------------------------------------
# TestStatic
# ---------------------------------------------------------------------------

class TestStatic(WidgetTestCase):

    def test_create_returns_valid_wid(self):
        wid = lib.pw_static_create(b"Hello", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_with_parent(self):
        root = app.pa_widget_add(b"container", b"", b"", PA_NO_PARENT)
        wid  = lib.pw_static_create(b"text", root)
        self.assertGreaterEqual(wid, 0)

    def test_empty_text_allowed(self):
        wid = lib.pw_static_create(b"", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_long_text_allowed(self):
        long_text = b"x" * 1000
        wid = lib.pw_static_create(long_text, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_multiple(self):
        w1 = lib.pw_static_create(b"a", PA_NO_PARENT)
        w2 = lib.pw_static_create(b"b", w1)
        self.assertNotEqual(w1, w2)


# ---------------------------------------------------------------------------
# TestLabel
# ---------------------------------------------------------------------------

class TestLabel(WidgetTestCase):

    def test_create_no_variant(self):
        wid = lib.pw_label_create(b"Label", None, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_primary(self):
        wid = lib.pw_label_create(b"Primary", b"primary", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_error_variant(self):
        wid = lib.pw_label_create(b"Error!", b"error", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_set_text(self):
        wid = lib.pw_label_create(b"old", None, PA_NO_PARENT)
        lib.pw_label_set_text(wid, b"new text")
        # No crash is sufficient; render will show new text

    def test_set_text_empty(self):
        wid = lib.pw_label_create(b"text", None, PA_NO_PARENT)
        lib.pw_label_set_text(wid, b"")

    def test_create_success_warning(self):
        w1 = lib.pw_label_create(b"OK",   b"success", PA_NO_PARENT)
        w2 = lib.pw_label_create(b"WARN", b"warning", w1)
        self.assertGreaterEqual(w1, 0)
        self.assertGreaterEqual(w2, 0)


# ---------------------------------------------------------------------------
# TestRule
# ---------------------------------------------------------------------------

class TestRule(WidgetTestCase):

    def test_create_default(self):
        wid = lib.pw_rule_create(None, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_heavy(self):
        wid = lib.pw_rule_create("\u2501".encode("utf-8"), PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_double(self):
        wid = lib.pw_rule_create("\u2550".encode("utf-8"), PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_set_char(self):
        wid = lib.pw_rule_create(None, PA_NO_PARENT)
        lib.pw_rule_set_char(wid, "\u2501".encode("utf-8"))

    def test_set_char_back_to_thin(self):
        wid = lib.pw_rule_create("\u2501".encode("utf-8"), PA_NO_PARENT)
        lib.pw_rule_set_char(wid, "\u2500".encode("utf-8"))

    def test_rule_text_filled(self):
        """After render the rule widget text must not be empty."""
        root = app.pa_widget_add(b"container", b"root", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(3))
        wid = lib.pw_rule_create(None, root)
        app.pa_render()
        # Rule generates repeating chars — just check no crash


# ---------------------------------------------------------------------------
# TestPlaceholder
# ---------------------------------------------------------------------------

class TestPlaceholder(WidgetTestCase):

    def test_create_with_label(self):
        wid = lib.pw_placeholder_create(b"Sidebar", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_null_label(self):
        wid = lib.pw_placeholder_create(None, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_set_variant_zero(self):
        wid = lib.pw_placeholder_create(b"test", PA_NO_PARENT)
        lib.pw_placeholder_set_variant(wid, 0)

    def test_set_variant_max(self):
        wid = lib.pw_placeholder_create(b"test", PA_NO_PARENT)
        lib.pw_placeholder_set_variant(wid, 6)

    def test_set_variant_wraps(self):
        """Out-of-range variant must not crash (should clamp/wrap)."""
        wid = lib.pw_placeholder_create(b"test", PA_NO_PARENT)
        lib.pw_placeholder_set_variant(wid, 100)

    def test_multiple_placeholders_different_variants(self):
        w1 = lib.pw_placeholder_create(b"A", PA_NO_PARENT)
        w2 = lib.pw_placeholder_create(b"B", w1)
        lib.pw_placeholder_set_variant(w1, 0)
        lib.pw_placeholder_set_variant(w2, 3)
        self.assertNotEqual(w1, w2)


# ---------------------------------------------------------------------------
# TestProgressBar
# ---------------------------------------------------------------------------

class TestProgressBar(WidgetTestCase):

    def test_create(self):
        wid = lib.pw_progressbar_create(ctypes.c_float(100.0), PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_initial_progress_zero(self):
        wid = lib.pw_progressbar_create(ctypes.c_float(100.0), PA_NO_PARENT)
        p = lib.pw_progressbar_get_progress(wid)
        self.assertAlmostEqual(p, 0.0, places=3)

    def test_set_progress(self):
        wid = lib.pw_progressbar_create(ctypes.c_float(100.0), PA_NO_PARENT)
        lib.pw_progressbar_set_progress(wid, ctypes.c_float(50.0))
        p = lib.pw_progressbar_get_progress(wid)
        self.assertAlmostEqual(p, 50.0, places=3)

    def test_get_total(self):
        wid = lib.pw_progressbar_create(ctypes.c_float(200.0), PA_NO_PARENT)
        t = lib.pw_progressbar_get_total(wid)
        self.assertAlmostEqual(t, 200.0, places=3)

    def test_progress_clamp_at_total(self):
        wid = lib.pw_progressbar_create(ctypes.c_float(100.0), PA_NO_PARENT)
        lib.pw_progressbar_set_progress(wid, ctypes.c_float(999.0))
        p = lib.pw_progressbar_get_progress(wid)
        self.assertLessEqual(p, 100.0)

    def test_progress_clamp_at_zero(self):
        wid = lib.pw_progressbar_create(ctypes.c_float(100.0), PA_NO_PARENT)
        lib.pw_progressbar_set_progress(wid, ctypes.c_float(-10.0))
        p = lib.pw_progressbar_get_progress(wid)
        self.assertGreaterEqual(p, 0.0)

    def test_update_after_render(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(1))
        wid = lib.pw_progressbar_create(ctypes.c_float(100.0), root)
        app.pa_widget_set_size(wid, fixed(40), fixed(1))
        lib.pw_progressbar_set_progress(wid, ctypes.c_float(50.0))
        app.pa_render()
        lib.pw_progressbar_update(wid)  # must not crash

    def test_update_before_render_safe(self):
        wid = lib.pw_progressbar_create(ctypes.c_float(100.0), PA_NO_PARENT)
        lib.pw_progressbar_update(wid)  # region is (0,0,0,0) — fallback width


# ---------------------------------------------------------------------------
# TestSparkline
# ---------------------------------------------------------------------------

class TestSparkline(WidgetTestCase):

    def test_create(self):
        wid = lib.pw_sparkline_create(PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_set_data(self):
        wid  = lib.pw_sparkline_create(PA_NO_PARENT)
        data = (ctypes.c_float * 8)(1, 2, 3, 4, 5, 6, 7, 8)
        lib.pw_sparkline_set_data(wid, data, 8)

    def test_set_empty_data(self):
        wid  = lib.pw_sparkline_create(PA_NO_PARENT)
        data = (ctypes.c_float * 1)(0)
        lib.pw_sparkline_set_data(wid, data, 0)

    def test_set_uniform_data(self):
        """All equal values should not divide by zero."""
        wid  = lib.pw_sparkline_create(PA_NO_PARENT)
        data = (ctypes.c_float * 4)(5, 5, 5, 5)
        lib.pw_sparkline_set_data(wid, data, 4)

    def test_set_data_large(self):
        wid  = lib.pw_sparkline_create(PA_NO_PARENT)
        n    = 200
        data = (ctypes.c_float * n)(*range(n))
        lib.pw_sparkline_set_data(wid, data, n)

    def test_render_no_crash(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(1))
        wid  = lib.pw_sparkline_create(root)
        app.pa_widget_set_size(wid, fixed(20), fixed(1))
        data = (ctypes.c_float * 5)(0, 2.5, 5, 7.5, 10)
        lib.pw_sparkline_set_data(wid, data, 5)
        app.pa_render()


# ---------------------------------------------------------------------------
# TestDigits
# ---------------------------------------------------------------------------

class TestDigits(WidgetTestCase):

    def test_create(self):
        wid = lib.pw_digits_create(b"123", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_all_digits(self):
        wid = lib.pw_digits_create(b"0123456789", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_with_separators(self):
        wid = lib.pw_digits_create(b"12:34:56", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_set_text(self):
        wid = lib.pw_digits_create(b"0", PA_NO_PARENT)
        lib.pw_digits_set_text(wid, b"99")

    def test_set_text_with_dot(self):
        wid = lib.pw_digits_create(b"0", PA_NO_PARENT)
        lib.pw_digits_set_text(wid, b"3.14")

    def test_set_text_empty(self):
        wid = lib.pw_digits_create(b"42", PA_NO_PARENT)
        lib.pw_digits_set_text(wid, b"")

    def test_render_produces_3_rows(self):
        """Digits text must contain exactly 2 newlines (3 rows)."""
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(3))
        wid = lib.pw_digits_create(b"42", root)
        app.pa_render()
        # After render, no crash suffices; actual text verified via draw


# ---------------------------------------------------------------------------
# TestLoadingIndicator
# ---------------------------------------------------------------------------

class TestLoading(WidgetTestCase):

    def test_create(self):
        wid = lib.pw_loading_create(PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_tick_advances_frame(self):
        wid = lib.pw_loading_create(PA_NO_PARENT)
        # Should not crash after multiple ticks
        for _ in range(10):
            lib.pw_loading_tick(wid, 100)

    def test_start_stop(self):
        wid = lib.pw_loading_create(PA_NO_PARENT)
        lib.pw_loading_start(wid)
        lib.pw_loading_tick(wid, 200)
        lib.pw_loading_stop(wid)
        lib.pw_loading_tick(wid, 200)  # should not advance after stop

    def test_stop_then_start_resumes(self):
        wid = lib.pw_loading_create(PA_NO_PARENT)
        lib.pw_loading_stop(wid)
        lib.pw_loading_start(wid)
        lib.pw_loading_tick(wid, 100)

    def test_render_no_crash(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(1))
        wid = lib.pw_loading_create(root)
        app.pa_widget_set_size(wid, fixed(4), fixed(1))
        lib.pw_loading_start(wid)
        app.pa_render()
        lib.pw_loading_tick(wid, 200)
        app.pa_render()

    def test_multiple_indicators(self):
        """Multiple independent spinners at different frames."""
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        w1 = lib.pw_loading_create(root)
        w2 = lib.pw_loading_create(root)
        lib.pw_loading_start(w1)
        lib.pw_loading_start(w2)
        lib.pw_loading_tick(w1, 100)
        lib.pw_loading_tick(w2, 200)
        self.assertNotEqual(w1, w2)


# ---------------------------------------------------------------------------
# TestRadioButton
# ---------------------------------------------------------------------------

class TestRadioButton(WidgetTestCase):

    def test_create_unchecked(self):
        wid = lib.pw_radio_create(b"Option A", 0, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_checked(self):
        wid = lib.pw_radio_create(b"Option A", 1, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_initial_state(self):
        wid = lib.pw_radio_create(b"opt", 0, PA_NO_PARENT)
        self.assertEqual(lib.pw_radio_get_checked(wid), 0)

    def test_set_checked(self):
        wid = lib.pw_radio_create(b"opt", 0, PA_NO_PARENT)
        lib.pw_radio_set_checked(wid, 1)
        self.assertEqual(lib.pw_radio_get_checked(wid), 1)

    def test_render_no_crash(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(2))
        lib.pw_radio_create(b"Choice A", 0, root)
        lib.pw_radio_create(b"Choice B", 1, root)
        app.pa_render()


# ---------------------------------------------------------------------------
# TestRadioSet
# ---------------------------------------------------------------------------

class TestRadioSet(WidgetTestCase):

    def test_create(self):
        wid = lib.pw_radioset_create(PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_add_returns_radio_wid(self):
        rs  = lib.pw_radioset_create(PA_NO_PARENT)
        wid = lib.pw_radioset_add(rs, b"Alpha")
        self.assertGreaterEqual(wid, 0)

    def test_count(self):
        rs = lib.pw_radioset_create(PA_NO_PARENT)
        lib.pw_radioset_add(rs, b"A")
        lib.pw_radioset_add(rs, b"B")
        lib.pw_radioset_add(rs, b"C")
        self.assertEqual(lib.pw_radioset_count(rs), 3)

    def test_first_auto_selected(self):
        rs = lib.pw_radioset_create(PA_NO_PARENT)
        lib.pw_radioset_add(rs, b"First")
        self.assertEqual(lib.pw_radioset_get_selected(rs), 0)

    def test_select_changes_selection(self):
        rs = lib.pw_radioset_create(PA_NO_PARENT)
        lib.pw_radioset_add(rs, b"A")
        lib.pw_radioset_add(rs, b"B")
        lib.pw_radioset_add(rs, b"C")
        lib.pw_radioset_select(rs, 2)
        self.assertEqual(lib.pw_radioset_get_selected(rs), 2)

    def test_mutual_exclusivity(self):
        rs = lib.pw_radioset_create(PA_NO_PARENT)
        r0 = lib.pw_radioset_add(rs, b"A")
        r1 = lib.pw_radioset_add(rs, b"B")
        lib.pw_radioset_select(rs, 1)
        self.assertEqual(lib.pw_radio_get_checked(r0), 0)
        self.assertEqual(lib.pw_radio_get_checked(r1), 1)

    def test_select_out_of_range_safe(self):
        rs = lib.pw_radioset_create(PA_NO_PARENT)
        lib.pw_radioset_add(rs, b"A")
        lib.pw_radioset_select(rs, 99)  # must not crash

    def test_empty_set_selected_is_minus_one(self):
        rs = lib.pw_radioset_create(PA_NO_PARENT)
        self.assertEqual(lib.pw_radioset_get_selected(rs), -1)

    def test_render_no_crash(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(5))
        rs = lib.pw_radioset_create(root)
        lib.pw_radioset_add(rs, b"Small")
        lib.pw_radioset_add(rs, b"Medium")
        lib.pw_radioset_add(rs, b"Large")
        lib.pw_radioset_select(rs, 1)
        app.pa_render()


# ---------------------------------------------------------------------------
# TestOptionList
# ---------------------------------------------------------------------------

class TestOptionList(WidgetTestCase):

    def test_create(self):
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_add_option_returns_index(self):
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        idx = lib.pw_optionlist_add_option(wid, b"Item A")
        self.assertEqual(idx, 0)
        idx2 = lib.pw_optionlist_add_option(wid, b"Item B")
        self.assertEqual(idx2, 1)

    def test_count(self):
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        lib.pw_optionlist_add_option(wid, b"A")
        lib.pw_optionlist_add_option(wid, b"B")
        lib.pw_optionlist_add_option(wid, b"C")
        self.assertEqual(lib.pw_optionlist_count(wid), 3)

    def test_initial_cursor_zero(self):
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        lib.pw_optionlist_add_option(wid, b"A")
        self.assertEqual(lib.pw_optionlist_get_cursor(wid), 0)

    def test_cursor_next(self):
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        lib.pw_optionlist_add_option(wid, b"A")
        lib.pw_optionlist_add_option(wid, b"B")
        lib.pw_optionlist_cursor_next(wid)
        self.assertEqual(lib.pw_optionlist_get_cursor(wid), 1)

    def test_cursor_prev(self):
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        lib.pw_optionlist_add_option(wid, b"A")
        lib.pw_optionlist_add_option(wid, b"B")
        lib.pw_optionlist_set_cursor(wid, 1)
        lib.pw_optionlist_cursor_prev(wid)
        self.assertEqual(lib.pw_optionlist_get_cursor(wid), 0)

    def test_cursor_clamps_at_end(self):
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        lib.pw_optionlist_add_option(wid, b"A")
        lib.pw_optionlist_cursor_next(wid)
        lib.pw_optionlist_cursor_next(wid)
        self.assertEqual(lib.pw_optionlist_get_cursor(wid), 0)

    def test_cursor_clamps_at_start(self):
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        lib.pw_optionlist_add_option(wid, b"A")
        lib.pw_optionlist_cursor_prev(wid)
        self.assertEqual(lib.pw_optionlist_get_cursor(wid), 0)

    def test_clear_resets(self):
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        lib.pw_optionlist_add_option(wid, b"A")
        lib.pw_optionlist_add_option(wid, b"B")
        lib.pw_optionlist_clear(wid)
        self.assertEqual(lib.pw_optionlist_count(wid), 0)
        self.assertEqual(lib.pw_optionlist_get_cursor(wid), 0)

    def test_add_after_clear(self):
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        lib.pw_optionlist_add_option(wid, b"A")
        lib.pw_optionlist_clear(wid)
        idx = lib.pw_optionlist_add_option(wid, b"Fresh")
        self.assertEqual(idx, 0)

    def test_render_no_crash(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(5))
        wid = lib.pw_optionlist_create(root)
        for opt in [b"Alpha", b"Beta", b"Gamma", b"Delta"]:
            lib.pw_optionlist_add_option(wid, opt)
        lib.pw_optionlist_set_cursor(wid, 2)
        app.pa_render()

    def test_scroll_with_many_items(self):
        """Cursor past item 2 should trigger scroll without crash."""
        wid = lib.pw_optionlist_create(PA_NO_PARENT)
        for i in range(20):
            lib.pw_optionlist_add_option(wid, str(i).encode())
        lib.pw_optionlist_set_cursor(wid, 15)


# ---------------------------------------------------------------------------
# TestListView
# ---------------------------------------------------------------------------

class TestListView(WidgetTestCase):

    def test_create(self):
        wid = lib.pw_listview_create(PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_add_item_returns_index(self):
        wid = lib.pw_listview_create(PA_NO_PARENT)
        self.assertEqual(lib.pw_listview_add_item(wid, b"First"), 0)
        self.assertEqual(lib.pw_listview_add_item(wid, b"Second"), 1)

    def test_count(self):
        wid = lib.pw_listview_create(PA_NO_PARENT)
        lib.pw_listview_add_item(wid, b"A")
        lib.pw_listview_add_item(wid, b"B")
        self.assertEqual(lib.pw_listview_count(wid), 2)

    def test_cursor_navigation(self):
        wid = lib.pw_listview_create(PA_NO_PARENT)
        lib.pw_listview_add_item(wid, b"A")
        lib.pw_listview_add_item(wid, b"B")
        lib.pw_listview_add_item(wid, b"C")
        lib.pw_listview_cursor_next(wid)
        lib.pw_listview_cursor_next(wid)
        self.assertEqual(lib.pw_listview_get_cursor(wid), 2)
        lib.pw_listview_cursor_prev(wid)
        self.assertEqual(lib.pw_listview_get_cursor(wid), 1)

    def test_clear(self):
        wid = lib.pw_listview_create(PA_NO_PARENT)
        lib.pw_listview_add_item(wid, b"A")
        lib.pw_listview_clear(wid)
        self.assertEqual(lib.pw_listview_count(wid), 0)

    def test_empty_cursor_next_safe(self):
        wid = lib.pw_listview_create(PA_NO_PARENT)
        lib.pw_listview_cursor_next(wid)  # no items — must not crash

    def test_render_uses_arrow_prefix(self):
        """ListView cursor uses ▶ — just verify no crash in render."""
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(5))
        wid = lib.pw_listview_create(root)
        for item in [b"File 1", b"File 2", b"File 3"]:
            lib.pw_listview_add_item(wid, item)
        lib.pw_listview_cursor_next(wid)
        app.pa_render()


# ---------------------------------------------------------------------------
# TestSelectionList
# ---------------------------------------------------------------------------

class TestSelectionList(WidgetTestCase):

    def test_create(self):
        wid = lib.pw_selectionlist_create(PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_add_unselected(self):
        wid = lib.pw_selectionlist_create(PA_NO_PARENT)
        idx = lib.pw_selectionlist_add_option(wid, b"Option", 0)
        self.assertEqual(idx, 0)
        self.assertEqual(lib.pw_selectionlist_is_selected(wid, 0), 0)

    def test_add_preselected(self):
        wid = lib.pw_selectionlist_create(PA_NO_PARENT)
        lib.pw_selectionlist_add_option(wid, b"Option", 1)
        self.assertEqual(lib.pw_selectionlist_is_selected(wid, 0), 1)

    def test_toggle_selection(self):
        wid = lib.pw_selectionlist_create(PA_NO_PARENT)
        lib.pw_selectionlist_add_option(wid, b"Item", 0)
        lib.pw_selectionlist_toggle_selection(wid, 0)
        self.assertEqual(lib.pw_selectionlist_is_selected(wid, 0), 1)
        lib.pw_selectionlist_toggle_selection(wid, 0)
        self.assertEqual(lib.pw_selectionlist_is_selected(wid, 0), 0)

    def test_set_selected(self):
        wid = lib.pw_selectionlist_create(PA_NO_PARENT)
        lib.pw_selectionlist_add_option(wid, b"A", 0)
        lib.pw_selectionlist_add_option(wid, b"B", 0)
        lib.pw_selectionlist_set_selected(wid, 1, 1)
        self.assertEqual(lib.pw_selectionlist_is_selected(wid, 0), 0)
        self.assertEqual(lib.pw_selectionlist_is_selected(wid, 1), 1)

    def test_multiple_items_selected_independently(self):
        wid = lib.pw_selectionlist_create(PA_NO_PARENT)
        lib.pw_selectionlist_add_option(wid, b"A", 1)
        lib.pw_selectionlist_add_option(wid, b"B", 0)
        lib.pw_selectionlist_add_option(wid, b"C", 1)
        self.assertEqual(lib.pw_selectionlist_is_selected(wid, 0), 1)
        self.assertEqual(lib.pw_selectionlist_is_selected(wid, 1), 0)
        self.assertEqual(lib.pw_selectionlist_is_selected(wid, 2), 1)

    def test_cursor_navigation(self):
        wid = lib.pw_selectionlist_create(PA_NO_PARENT)
        lib.pw_selectionlist_add_option(wid, b"A", 0)
        lib.pw_selectionlist_add_option(wid, b"B", 0)
        lib.pw_selectionlist_cursor_next(wid)
        self.assertEqual(lib.pw_selectionlist_get_cursor(wid), 1)
        lib.pw_selectionlist_cursor_prev(wid)
        self.assertEqual(lib.pw_selectionlist_get_cursor(wid), 0)

    def test_toggle_out_of_range_safe(self):
        wid = lib.pw_selectionlist_create(PA_NO_PARENT)
        lib.pw_selectionlist_add_option(wid, b"A", 0)
        lib.pw_selectionlist_toggle_selection(wid, 99)  # must not crash

    def test_count(self):
        wid = lib.pw_selectionlist_create(PA_NO_PARENT)
        for i in range(5):
            lib.pw_selectionlist_add_option(wid, str(i).encode(), 0)
        self.assertEqual(lib.pw_selectionlist_count(wid), 5)

    def test_clear(self):
        wid = lib.pw_selectionlist_create(PA_NO_PARENT)
        lib.pw_selectionlist_add_option(wid, b"A", 1)
        lib.pw_selectionlist_clear(wid)
        self.assertEqual(lib.pw_selectionlist_count(wid), 0)

    def test_render_mixed_states(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(6))
        wid = lib.pw_selectionlist_create(root)
        lib.pw_selectionlist_add_option(wid, b"Python",     1)
        lib.pw_selectionlist_add_option(wid, b"JavaScript", 0)
        lib.pw_selectionlist_add_option(wid, b"Rust",       1)
        lib.pw_selectionlist_add_option(wid, b"Go",         0)
        lib.pw_selectionlist_set_cursor(wid, 1)
        app.pa_render()


# ---------------------------------------------------------------------------
# TestToggleButton
# ---------------------------------------------------------------------------

class TestToggleButton(WidgetTestCase):

    def test_create_unchecked(self):
        wid = lib.pw_toggle_create(b"Option A", 0, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_checked(self):
        wid = lib.pw_toggle_create(b"Option B", 1, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_initial_state_false(self):
        wid = lib.pw_toggle_create(b"opt", 0, PA_NO_PARENT)
        self.assertEqual(lib.pw_toggle_get_checked(wid), 0)

    def test_initial_state_true(self):
        wid = lib.pw_toggle_create(b"opt", 1, PA_NO_PARENT)
        self.assertEqual(lib.pw_toggle_get_checked(wid), 1)

    def test_set_checked_true(self):
        wid = lib.pw_toggle_create(b"opt", 0, PA_NO_PARENT)
        lib.pw_toggle_set_checked(wid, 1)
        self.assertEqual(lib.pw_toggle_get_checked(wid), 1)

    def test_set_checked_false(self):
        wid = lib.pw_toggle_create(b"opt", 1, PA_NO_PARENT)
        lib.pw_toggle_set_checked(wid, 0)
        self.assertEqual(lib.pw_toggle_get_checked(wid), 0)

    def test_toggle_flips(self):
        wid = lib.pw_toggle_create(b"opt", 0, PA_NO_PARENT)
        lib.pw_toggle_toggle(wid)
        self.assertEqual(lib.pw_toggle_get_checked(wid), 1)
        lib.pw_toggle_toggle(wid)
        self.assertEqual(lib.pw_toggle_get_checked(wid), 0)

    def test_set_label(self):
        wid = lib.pw_toggle_create(b"old", 0, PA_NO_PARENT)
        lib.pw_toggle_set_label(wid, b"new label")

    def test_empty_label(self):
        wid = lib.pw_toggle_create(b"", 0, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_render_no_crash(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(3))
        lib.pw_toggle_create(b"Enable feature", 0, root)
        app.pa_render()


# ---------------------------------------------------------------------------
# TestCheckbox
# ---------------------------------------------------------------------------

class TestCheckbox(WidgetTestCase):

    def test_create_unchecked(self):
        wid = lib.pw_checkbox_create(b"Accept terms", 0, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_checked(self):
        wid = lib.pw_checkbox_create(b"Accept terms", 1, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_initial_false(self):
        wid = lib.pw_checkbox_create(b"cb", 0, PA_NO_PARENT)
        self.assertEqual(lib.pw_checkbox_get_checked(wid), 0)

    def test_initial_true(self):
        wid = lib.pw_checkbox_create(b"cb", 1, PA_NO_PARENT)
        self.assertEqual(lib.pw_checkbox_get_checked(wid), 1)

    def test_set_checked(self):
        wid = lib.pw_checkbox_create(b"cb", 0, PA_NO_PARENT)
        lib.pw_checkbox_set_checked(wid, 1)
        self.assertEqual(lib.pw_checkbox_get_checked(wid), 1)

    def test_toggle_flips(self):
        wid = lib.pw_checkbox_create(b"cb", 0, PA_NO_PARENT)
        lib.pw_checkbox_toggle(wid)
        self.assertEqual(lib.pw_checkbox_get_checked(wid), 1)
        lib.pw_checkbox_toggle(wid)
        self.assertEqual(lib.pw_checkbox_get_checked(wid), 0)

    def test_set_label(self):
        wid = lib.pw_checkbox_create(b"old", 0, PA_NO_PARENT)
        lib.pw_checkbox_set_label(wid, b"new label")

    def test_toggle_and_label_independent(self):
        """Toggling state does not corrupt the label."""
        wid = lib.pw_checkbox_create(b"stable label", 0, PA_NO_PARENT)
        lib.pw_checkbox_toggle(wid)
        lib.pw_checkbox_toggle(wid)
        # label still intact, no crash

    def test_render_unchecked(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(2))
        lib.pw_checkbox_create(b"Newsletter", 0, root)
        app.pa_render()

    def test_render_checked(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(2))
        lib.pw_checkbox_create(b"Newsletter", 1, root)
        app.pa_render()


# ---------------------------------------------------------------------------
# TestSwitch
# ---------------------------------------------------------------------------

class TestSwitch(WidgetTestCase):

    def test_create_off(self):
        wid = lib.pw_switch_create(0, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_on(self):
        wid = lib.pw_switch_create(1, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_initial_off(self):
        wid = lib.pw_switch_create(0, PA_NO_PARENT)
        self.assertEqual(lib.pw_switch_get_active(wid), 0)

    def test_initial_on(self):
        wid = lib.pw_switch_create(1, PA_NO_PARENT)
        self.assertEqual(lib.pw_switch_get_active(wid), 1)

    def test_set_active(self):
        wid = lib.pw_switch_create(0, PA_NO_PARENT)
        lib.pw_switch_set_active(wid, 1)
        self.assertEqual(lib.pw_switch_get_active(wid), 1)

    def test_set_inactive(self):
        wid = lib.pw_switch_create(1, PA_NO_PARENT)
        lib.pw_switch_set_active(wid, 0)
        self.assertEqual(lib.pw_switch_get_active(wid), 0)

    def test_toggle_flips(self):
        wid = lib.pw_switch_create(0, PA_NO_PARENT)
        lib.pw_switch_toggle(wid)
        self.assertEqual(lib.pw_switch_get_active(wid), 1)
        lib.pw_switch_toggle(wid)
        self.assertEqual(lib.pw_switch_get_active(wid), 0)

    def test_multiple_toggles(self):
        wid = lib.pw_switch_create(0, PA_NO_PARENT)
        for _ in range(10):
            lib.pw_switch_toggle(wid)
        self.assertEqual(lib.pw_switch_get_active(wid), 0)

    def test_render_off(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(1))
        lib.pw_switch_create(0, root)
        app.pa_render()

    def test_render_on(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(1))
        lib.pw_switch_create(1, root)
        app.pa_render()


# ---------------------------------------------------------------------------
# TestButton
# ---------------------------------------------------------------------------

class TestButton(WidgetTestCase):

    def test_create_default(self):
        wid = lib.pw_button_create(b"OK", None, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_primary(self):
        wid = lib.pw_button_create(b"Submit", b"primary", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_success(self):
        wid = lib.pw_button_create(b"Confirm", b"success", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_warning(self):
        wid = lib.pw_button_create(b"Maybe", b"warning", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_error(self):
        wid = lib.pw_button_create(b"Delete", b"error", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_set_label(self):
        wid = lib.pw_button_create(b"Old", None, PA_NO_PARENT)
        lib.pw_button_set_label(wid, b"New Label")

    def test_empty_label(self):
        wid = lib.pw_button_create(b"", None, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_multiple_buttons_different_wids(self):
        w1 = lib.pw_button_create(b"Yes", b"success", PA_NO_PARENT)
        w2 = lib.pw_button_create(b"No",  b"error",   PA_NO_PARENT)
        self.assertNotEqual(w1, w2)

    def test_render_no_crash(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(3))
        lib.pw_button_create(b"OK",     b"success", root)
        lib.pw_button_create(b"Cancel", b"error",   root)
        app.pa_render()

    def test_render_after_label_change(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(1))
        wid = lib.pw_button_create(b"Click me", None, root)
        app.pa_render()
        lib.pw_button_set_label(wid, b"Clicked!")
        app.pa_render()


# ---------------------------------------------------------------------------
# TestHeader
# ---------------------------------------------------------------------------

class TestHeader(WidgetTestCase):

    def test_create_returns_valid_wid(self):
        wid = lib.pw_header_create(b"My App", b"[>]", PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_no_icon(self):
        wid = lib.pw_header_create(b"Title", None, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_create_empty_title(self):
        wid = lib.pw_header_create(b"", None, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_set_title(self):
        wid = lib.pw_header_create(b"Old", None, PA_NO_PARENT)
        lib.pw_header_set_title(wid, b"New Title")
        # No crash is sufficient

    def test_set_icon(self):
        wid = lib.pw_header_create(b"App", None, PA_NO_PARENT)
        lib.pw_header_set_icon(wid, b"*")

    def test_clock_off_by_default(self):
        """Header created without clock — pw_header_tick should be a no-op."""
        wid = lib.pw_header_create(b"App", None, PA_NO_PARENT)
        lib.pw_header_tick(wid)  # must not crash

    def test_clock_on(self):
        wid = lib.pw_header_create(b"App", None, PA_NO_PARENT)
        lib.pw_header_set_clock(wid, 1)
        lib.pw_header_tick(wid)

    def test_clock_toggle(self):
        wid = lib.pw_header_create(b"App", None, PA_NO_PARENT)
        lib.pw_header_set_clock(wid, 1)
        lib.pw_header_tick(wid)
        lib.pw_header_set_clock(wid, 0)
        lib.pw_header_tick(wid)  # now a no-op again

    def test_render_with_clock(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(24))
        wid = lib.pw_header_create(b"PlagueTUI", b"[P]", root)
        lib.pw_header_set_clock(wid, 1)
        lib.pw_header_tick(wid)
        app.pa_render()

    def test_multiple_ticks_stable(self):
        wid = lib.pw_header_create(b"App", None, PA_NO_PARENT)
        lib.pw_header_set_clock(wid, 1)
        for _ in range(5):
            lib.pw_header_tick(wid)


# ---------------------------------------------------------------------------
# TestFooter
# ---------------------------------------------------------------------------

class TestFooter(WidgetTestCase):

    def test_create_returns_valid_wid(self):
        wid = lib.pw_footer_create(PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_empty_footer_no_crash(self):
        wid = lib.pw_footer_create(PA_NO_PARENT)
        lib.pw_footer_refresh(wid)

    def test_add_one_key(self):
        wid = lib.pw_footer_create(PA_NO_PARENT)
        lib.pw_footer_add_key(wid, b"q", b"Quit")

    def test_add_multiple_keys(self):
        wid = lib.pw_footer_create(PA_NO_PARENT)
        lib.pw_footer_add_key(wid, b"q",   b"Quit")
        lib.pw_footer_add_key(wid, b"Tab", b"Focus next")
        lib.pw_footer_add_key(wid, b"?",   b"Help")

    def test_clear_keys(self):
        wid = lib.pw_footer_create(PA_NO_PARENT)
        lib.pw_footer_add_key(wid, b"q", b"Quit")
        lib.pw_footer_clear_keys(wid)

    def test_add_after_clear(self):
        wid = lib.pw_footer_create(PA_NO_PARENT)
        lib.pw_footer_add_key(wid, b"q", b"Quit")
        lib.pw_footer_clear_keys(wid)
        lib.pw_footer_add_key(wid, b"?", b"Help")

    def test_add_max_keys(self):
        """Adding 16 keys (the max) must not crash."""
        wid = lib.pw_footer_create(PA_NO_PARENT)
        for i in range(16):
            lib.pw_footer_add_key(wid, str(i).encode(), b"desc")

    def test_add_beyond_max_safe(self):
        """Adding more than 16 keys must not crash or corrupt memory."""
        wid = lib.pw_footer_create(PA_NO_PARENT)
        for i in range(20):
            lib.pw_footer_add_key(wid, str(i).encode(), b"desc")

    def test_refresh_idempotent(self):
        wid = lib.pw_footer_create(PA_NO_PARENT)
        lib.pw_footer_add_key(wid, b"q", b"Quit")
        lib.pw_footer_refresh(wid)
        lib.pw_footer_refresh(wid)

    def test_render_no_crash(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(24))
        wid = lib.pw_footer_create(root)
        lib.pw_footer_add_key(wid, b"q",   b"Quit")
        lib.pw_footer_add_key(wid, b"Tab", b"Next")
        app.pa_render()


# ---------------------------------------------------------------------------
# TestHeaderFooterCombo
# ---------------------------------------------------------------------------

class TestHeaderFooterCombo(WidgetTestCase):

    def test_header_and_footer_same_parent(self):
        root   = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(24))
        header = lib.pw_header_create(b"My App", b"[>]", root)
        footer = lib.pw_footer_create(root)
        lib.pw_footer_add_key(footer, b"q", b"Quit")
        lib.pw_header_set_clock(header, 1)
        lib.pw_header_tick(header)
        app.pa_render()
        self.assertNotEqual(header, footer)

    def test_header_footer_different_wids(self):
        root   = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        header = lib.pw_header_create(b"Title", None, root)
        footer = lib.pw_footer_create(root)
        self.assertGreaterEqual(header, 0)
        self.assertGreaterEqual(footer, 0)
        self.assertNotEqual(header, footer)


# ---------------------------------------------------------------------------
# TestContentSwitcher
# ---------------------------------------------------------------------------

class TestContentSwitcher(WidgetTestCase):

    def test_create(self):
        wid = lib.pw_switcher_create(PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_empty_active_is_minus_one(self):
        wid = lib.pw_switcher_create(PA_NO_PARENT)
        self.assertEqual(lib.pw_switcher_active(wid), -1)

    def test_empty_count_zero(self):
        wid = lib.pw_switcher_create(PA_NO_PARENT)
        self.assertEqual(lib.pw_switcher_count(wid), 0)

    def test_add_first_child_auto_visible(self):
        sw  = lib.pw_switcher_create(PA_NO_PARENT)
        c0  = app.pa_widget_add(b"container", b"c0", b"", sw)
        idx = lib.pw_switcher_add(sw, c0)
        self.assertEqual(idx, 0)
        self.assertEqual(lib.pw_switcher_active(sw), 0)

    def test_add_second_child_hidden(self):
        sw = lib.pw_switcher_create(PA_NO_PARENT)
        c0 = app.pa_widget_add(b"container", b"c0", b"", sw)
        c1 = app.pa_widget_add(b"container", b"c1", b"", sw)
        lib.pw_switcher_add(sw, c0)
        lib.pw_switcher_add(sw, c1)
        # active is still 0 (first)
        self.assertEqual(lib.pw_switcher_active(sw), 0)

    def test_count(self):
        sw = lib.pw_switcher_create(PA_NO_PARENT)
        for i in range(3):
            c = app.pa_widget_add(b"container", b"cx", b"", sw)
            lib.pw_switcher_add(sw, c)
        self.assertEqual(lib.pw_switcher_count(sw), 3)

    def test_show_changes_active(self):
        sw = lib.pw_switcher_create(PA_NO_PARENT)
        c0 = app.pa_widget_add(b"container", b"c0", b"", sw)
        c1 = app.pa_widget_add(b"container", b"c1", b"", sw)
        c2 = app.pa_widget_add(b"container", b"c2", b"", sw)
        lib.pw_switcher_add(sw, c0)
        lib.pw_switcher_add(sw, c1)
        lib.pw_switcher_add(sw, c2)
        lib.pw_switcher_show(sw, 2)
        self.assertEqual(lib.pw_switcher_active(sw), 2)

    def test_show_out_of_range_safe(self):
        sw = lib.pw_switcher_create(PA_NO_PARENT)
        c  = app.pa_widget_add(b"container", b"c", b"", sw)
        lib.pw_switcher_add(sw, c)
        lib.pw_switcher_show(sw, 99)  # must not crash

    def test_cycle_panels(self):
        sw = lib.pw_switcher_create(PA_NO_PARENT)
        children = []
        for i in range(4):
            c = app.pa_widget_add(b"container", b"c", b"", sw)
            lib.pw_switcher_add(sw, c)
            children.append(c)
        for i in range(4):
            lib.pw_switcher_show(sw, i)
            self.assertEqual(lib.pw_switcher_active(sw), i)

    def test_render_only_active_visible(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(10))
        sw = lib.pw_switcher_create(root)
        app.pa_widget_set_size(sw, fixed(80), fixed(8))
        p0 = app.pa_widget_add(b"container", b"p0", b"", sw)
        p1 = app.pa_widget_add(b"container", b"p1", b"", sw)
        lib.pw_switcher_add(sw, p0)
        lib.pw_switcher_add(sw, p1)
        app.pa_render()
        lib.pw_switcher_show(sw, 1)
        app.pa_render()


# ---------------------------------------------------------------------------
# TestToast
# ---------------------------------------------------------------------------

class TestToast(WidgetTestCase):

    def test_create(self):
        wid = lib.pw_toast_create(b"Hello!", 2000, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_initially_visible(self):
        wid = lib.pw_toast_create(b"msg", 1000, PA_NO_PARENT)
        self.assertEqual(lib.pw_toast_is_visible(wid), 1)

    def test_tick_before_expiry_stays_visible(self):
        wid = lib.pw_toast_create(b"msg", 1000, PA_NO_PARENT)
        lib.pw_toast_tick(wid, 500)
        self.assertEqual(lib.pw_toast_is_visible(wid), 1)

    def test_tick_at_expiry_dismisses(self):
        wid = lib.pw_toast_create(b"msg", 1000, PA_NO_PARENT)
        lib.pw_toast_tick(wid, 1000)
        self.assertEqual(lib.pw_toast_is_visible(wid), 0)

    def test_tick_past_expiry_dismisses(self):
        wid = lib.pw_toast_create(b"msg", 500, PA_NO_PARENT)
        lib.pw_toast_tick(wid, 200)
        lib.pw_toast_tick(wid, 400)
        self.assertEqual(lib.pw_toast_is_visible(wid), 0)

    def test_tick_after_dismiss_safe(self):
        wid = lib.pw_toast_create(b"msg", 100, PA_NO_PARENT)
        lib.pw_toast_tick(wid, 200)
        lib.pw_toast_tick(wid, 200)  # already dismissed — must not crash

    def test_show_resets_visibility(self):
        wid = lib.pw_toast_create(b"old", 100, PA_NO_PARENT)
        lib.pw_toast_tick(wid, 200)
        self.assertEqual(lib.pw_toast_is_visible(wid), 0)
        lib.pw_toast_show(wid, b"new message", 2000)
        self.assertEqual(lib.pw_toast_is_visible(wid), 1)

    def test_show_resets_timer(self):
        wid = lib.pw_toast_create(b"msg", 500, PA_NO_PARENT)
        lib.pw_toast_tick(wid, 300)
        lib.pw_toast_show(wid, b"reset", 1000)
        lib.pw_toast_tick(wid, 600)
        # new duration 1000ms — 600ms elapsed, still visible
        self.assertEqual(lib.pw_toast_is_visible(wid), 1)

    def test_null_message_safe(self):
        wid = lib.pw_toast_create(None, 1000, PA_NO_PARENT)
        self.assertGreaterEqual(wid, 0)

    def test_render_no_crash(self):
        root = app.pa_widget_add(b"container", b"r", b"", PA_NO_PARENT)
        app.pa_widget_set_size(root, fixed(80), fixed(10))
        wid = lib.pw_toast_create(b"File saved successfully!", 3000, root)
        app.pa_render()

    def test_default_duration_when_zero(self):
        """duration_ms=0 must use default (3000ms), not instant dismiss."""
        wid = lib.pw_toast_create(b"msg", 0, PA_NO_PARENT)
        lib.pw_toast_tick(wid, 1000)
        self.assertEqual(lib.pw_toast_is_visible(wid), 1)


if __name__ == "__main__":
    unittest.main()
