import unittest
from conftest import (lib, PE_EVENT_TIMER, pop_event)


class TestTimerCreate(unittest.TestCase):

    def setUp(self):
        lib.pe_reset_all()

    def test_create_returns_nonzero_id(self):
        tid = lib.pe_timer_create(100, 0)
        self.assertGreater(tid, 0)
        lib.pe_timer_cancel(tid)

    def test_two_timers_different_ids(self):
        t1 = lib.pe_timer_create(100, 0)
        t2 = lib.pe_timer_create(200, 0)
        self.assertNotEqual(t1, t2)
        lib.pe_timer_cancel(t1)
        lib.pe_timer_cancel(t2)

    def test_invalid_interval_returns_zero(self):
        tid = lib.pe_timer_create(0, 0)
        self.assertEqual(tid, 0)

    def test_created_timer_is_active(self):
        tid = lib.pe_timer_create(100, 0)
        self.assertEqual(lib.pe_timer_active(tid), 1)
        lib.pe_timer_cancel(tid)

    def test_cancelled_timer_is_inactive(self):
        tid = lib.pe_timer_create(100, 0)
        lib.pe_timer_cancel(tid)
        self.assertEqual(lib.pe_timer_active(tid), 0)


class TestTimerOneShot(unittest.TestCase):

    def setUp(self):
        lib.pe_reset_all()

    def test_no_fire_before_interval(self):
        tid = lib.pe_timer_create(100, 0)
        lib.pe_timer_tick(50)
        self.assertEqual(lib.pe_queue_size(), 0)
        lib.pe_timer_cancel(tid)

    def test_fires_at_interval(self):
        tid = lib.pe_timer_create(100, 0)
        lib.pe_timer_tick(100)
        self.assertEqual(lib.pe_queue_size(), 1)
        got, ev = pop_event()
        self.assertEqual(got, 1)
        self.assertEqual(ev.type, PE_EVENT_TIMER)
        self.assertEqual(ev.data.timer.timer_id, tid)

    def test_fires_when_elapsed_exceeds_interval(self):
        tid = lib.pe_timer_create(100, 0)
        lib.pe_timer_tick(150)
        self.assertEqual(lib.pe_queue_size(), 1)
        lib.pe_timer_cancel(tid)

    def test_one_shot_becomes_inactive_after_fire(self):
        tid = lib.pe_timer_create(100, 0)
        lib.pe_timer_tick(100)
        self.assertEqual(lib.pe_timer_active(tid), 0)

    def test_one_shot_does_not_fire_again(self):
        lib.pe_timer_create(100, 0)
        lib.pe_timer_tick(100)
        lib.pe_queue_clear()
        lib.pe_timer_tick(100)
        self.assertEqual(lib.pe_queue_size(), 0)

    def test_partial_ticks_accumulate(self):
        tid = lib.pe_timer_create(100, 0)
        lib.pe_timer_tick(40)
        lib.pe_timer_tick(40)
        self.assertEqual(lib.pe_queue_size(), 0)
        lib.pe_timer_tick(20)  # total=100 → dispara
        self.assertEqual(lib.pe_queue_size(), 1)
        lib.pe_timer_cancel(tid)


class TestTimerRepeat(unittest.TestCase):

    def setUp(self):
        lib.pe_reset_all()

    def test_repeat_fires_multiple_times(self):
        tid = lib.pe_timer_create(100, 1)
        lib.pe_timer_tick(100)
        lib.pe_timer_tick(100)
        lib.pe_timer_tick(100)
        self.assertEqual(lib.pe_queue_size(), 3)
        lib.pe_timer_cancel(tid)

    def test_repeat_remains_active_after_fire(self):
        tid = lib.pe_timer_create(100, 1)
        lib.pe_timer_tick(100)
        self.assertEqual(lib.pe_timer_active(tid), 1)
        lib.pe_timer_cancel(tid)

    def test_repeat_fires_multiple_in_single_tick(self):
        """Un tick grande puede disparar el timer varias veces."""
        lib.pe_timer_create(100, 1)
        lib.pe_timer_tick(350)  # debería disparar 3 veces
        self.assertEqual(lib.pe_queue_size(), 3)

    def test_cancel_stops_repeat(self):
        tid = lib.pe_timer_create(100, 1)
        lib.pe_timer_tick(100)
        lib.pe_queue_clear()
        lib.pe_timer_cancel(tid)
        lib.pe_timer_tick(100)
        self.assertEqual(lib.pe_queue_size(), 0)


class TestTimerMultiple(unittest.TestCase):

    def setUp(self):
        lib.pe_reset_all()

    def test_two_timers_fire_independently(self):
        t1 = lib.pe_timer_create(100, 0)
        t2 = lib.pe_timer_create(200, 0)
        lib.pe_timer_tick(100)
        self.assertEqual(lib.pe_queue_size(), 1)
        _, ev = pop_event()
        self.assertEqual(ev.data.timer.timer_id, t1)
        lib.pe_timer_tick(100)  # total=200 para t2
        self.assertEqual(lib.pe_queue_size(), 1)
        _, ev = pop_event()
        self.assertEqual(ev.data.timer.timer_id, t2)


if __name__ == "__main__":
    unittest.main()
