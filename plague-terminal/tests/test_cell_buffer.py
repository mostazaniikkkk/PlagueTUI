import ctypes
import unittest
from conftest import lib, PT_Cell


class TestCellBuffer(unittest.TestCase):

    def setUp(self):
        lib.pt_init_headless(80, 24)

    def tearDown(self):
        lib.pt_shutdown()

    def test_init_size(self):
        cols, rows = ctypes.c_int(), ctypes.c_int()
        lib.pt_get_size(ctypes.byref(cols), ctypes.byref(rows))
        self.assertEqual(cols.value, 80)
        self.assertEqual(rows.value, 24)

    def test_default_cell_is_space(self):
        cell = lib.pt_get_cell(0, 0)
        self.assertEqual(cell.ch[0:1], b' ')
        self.assertEqual(cell.ch_len,  1)

    def test_default_cell_colors_are_zero(self):
        cell = lib.pt_get_cell(10, 5)
        self.assertEqual(cell.r_fg, 0)
        self.assertEqual(cell.r_bg, 0)

    def test_out_of_bounds_returns_space(self):
        cell = lib.pt_get_cell(999, 999)
        self.assertEqual(cell.ch[0:1], b' ')

    def test_clear_resets_to_space(self):
        from conftest import TG_Region, PG_Color
        lib.pt_fill_rect(TG_Region(0, 0, 10, 10), PG_Color(1, 0, 0, 1))
        lib.pt_clear()
        cell = lib.pt_get_cell(5, 5)
        self.assertEqual(cell.r_bg, 0)
        self.assertEqual(cell.ch[0:1], b' ')

    def test_resize_updates_size(self):
        lib.pt_resize(120, 40)
        cols, rows = ctypes.c_int(), ctypes.c_int()
        lib.pt_get_size(ctypes.byref(cols), ctypes.byref(rows))
        self.assertEqual(cols.value, 120)
        self.assertEqual(rows.value, 40)

    def test_flush_is_noop_in_headless(self):
        # No debe lanzar excepción ni crashear
        lib.pt_flush()

    def test_headless_poll_returns_no_event(self):
        from conftest import PT_Event, PT_EVENT_NONE
        ev = PT_Event()
        result = lib.pt_poll_event(ctypes.byref(ev))
        self.assertEqual(result, 0)
        self.assertEqual(ev.type, PT_EVENT_NONE)


if __name__ == "__main__":
    unittest.main()
