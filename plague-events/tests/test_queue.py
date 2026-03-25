import unittest
from conftest import (
    lib, PE_Event, PE_EventData,
    PE_EVENT_KEY, PE_EVENT_RESIZE, PE_EVENT_NONE,
    PE_KEY_ESCAPE, PE_MOD_CTRL,
    make_key_event, make_resize_event, pop_event,
)


class TestQueueInit(unittest.TestCase):

    def setUp(self):
        lib.pe_queue_init()

    def test_empty_on_init(self):
        self.assertEqual(lib.pe_queue_size(), 0)

    def test_pop_empty_returns_zero(self):
        got, _ = pop_event()
        self.assertEqual(got, 0)


class TestQueuePushPop(unittest.TestCase):

    def setUp(self):
        lib.pe_queue_init()

    def test_push_increases_size(self):
        lib.pe_queue_push(make_key_event(PE_KEY_ESCAPE))
        self.assertEqual(lib.pe_queue_size(), 1)

    def test_pop_decreases_size(self):
        lib.pe_queue_push(make_key_event(PE_KEY_ESCAPE))
        pop_event()
        self.assertEqual(lib.pe_queue_size(), 0)

    def test_pop_returns_pushed_event(self):
        lib.pe_queue_push(make_key_event(PE_KEY_ESCAPE))
        got, ev = pop_event()
        self.assertEqual(got, 1)
        self.assertEqual(ev.type, PE_EVENT_KEY)
        self.assertEqual(ev.data.key.key, PE_KEY_ESCAPE)

    def test_fifo_order(self):
        lib.pe_queue_push(make_key_event(ord('a')))
        lib.pe_queue_push(make_key_event(ord('b')))
        lib.pe_queue_push(make_key_event(ord('c')))
        _, e1 = pop_event()
        _, e2 = pop_event()
        _, e3 = pop_event()
        self.assertEqual(e1.data.key.key, ord('a'))
        self.assertEqual(e2.data.key.key, ord('b'))
        self.assertEqual(e3.data.key.key, ord('c'))

    def test_key_modifiers_preserved(self):
        lib.pe_queue_push(make_key_event(ord('c'), PE_MOD_CTRL))
        _, ev = pop_event()
        self.assertEqual(ev.data.key.modifiers, PE_MOD_CTRL)

    def test_resize_event_fields(self):
        lib.pe_queue_push(make_resize_event(120, 40))
        _, ev = pop_event()
        self.assertEqual(ev.type,             PE_EVENT_RESIZE)
        self.assertEqual(ev.data.resize.cols, 120)
        self.assertEqual(ev.data.resize.rows,  40)

    def test_target_preserved(self):
        lib.pe_queue_push(make_key_event(PE_KEY_ESCAPE, target=7))
        _, ev = pop_event()
        self.assertEqual(ev.target, 7)


class TestQueueFull(unittest.TestCase):

    def setUp(self):
        lib.pe_queue_init()

    def test_push_256_events(self):
        for i in range(256):
            result = lib.pe_queue_push(make_key_event(ord('x')))
            self.assertEqual(result, 1, f"push {i} should succeed")
        self.assertEqual(lib.pe_queue_size(), 256)

    def test_push_257th_fails(self):
        for _ in range(256):
            lib.pe_queue_push(make_key_event(ord('x')))
        result = lib.pe_queue_push(make_key_event(ord('y')))
        self.assertEqual(result, 0)

    def test_size_does_not_exceed_capacity(self):
        for _ in range(300):
            lib.pe_queue_push(make_key_event(ord('x')))
        self.assertLessEqual(lib.pe_queue_size(), 256)


class TestQueueClear(unittest.TestCase):

    def setUp(self):
        lib.pe_queue_init()

    def test_clear_empties_queue(self):
        for _ in range(10):
            lib.pe_queue_push(make_key_event(ord('x')))
        lib.pe_queue_clear()
        self.assertEqual(lib.pe_queue_size(), 0)

    def test_pop_after_clear_returns_empty(self):
        lib.pe_queue_push(make_key_event(ord('x')))
        lib.pe_queue_clear()
        got, _ = pop_event()
        self.assertEqual(got, 0)


class TestQueueWrapAround(unittest.TestCase):
    """Verifica que el ring buffer funciona correctamente tras el wrap."""

    def setUp(self):
        lib.pe_queue_init()

    def test_wrap_around(self):
        # Llenar y vaciar parcialmente para forzar wrap del índice
        for i in range(200):
            lib.pe_queue_push(make_key_event(i % 256))
        for _ in range(200):
            pop_event()
        # Ahora push más allá del final del array
        for i in range(100):
            lib.pe_queue_push(make_key_event(i % 256))
        self.assertEqual(lib.pe_queue_size(), 100)
        _, first = pop_event()
        self.assertEqual(first.data.key.key, 0)


if __name__ == "__main__":
    unittest.main()
