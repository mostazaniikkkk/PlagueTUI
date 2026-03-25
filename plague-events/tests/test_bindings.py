import unittest
from conftest import (
    lib,
    PE_KEY_ESCAPE, PE_KEY_UP, PE_KEY_ENTER, PE_KEY_F5,
    PE_MOD_NONE, PE_MOD_CTRL, PE_MOD_SHIFT, PE_MOD_ALT,
    PE_NO_PARENT,
)


class TestTreeBasic(unittest.TestCase):

    def setUp(self):
        lib.pe_reset_all()

    def test_add_root_node(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        self.assertGreaterEqual(nid, 0)

    def test_root_has_no_parent(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        self.assertEqual(lib.pe_tree_parent(nid), PE_NO_PARENT)

    def test_child_has_correct_parent(self):
        root  = lib.pe_tree_add(PE_NO_PARENT)
        child = lib.pe_tree_add(root)
        self.assertEqual(lib.pe_tree_parent(child), root)

    def test_set_parent(self):
        root  = lib.pe_tree_add(PE_NO_PARENT)
        child = lib.pe_tree_add(PE_NO_PARENT)
        lib.pe_tree_set_parent(child, root)
        self.assertEqual(lib.pe_tree_parent(child), root)

    def test_invalid_node_returns_no_parent(self):
        self.assertEqual(lib.pe_tree_parent(999), PE_NO_PARENT)


class TestBindKey(unittest.TestCase):

    def setUp(self):
        lib.pe_reset_all()

    def test_bind_returns_nonzero_id(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        bid = lib.pe_bind_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE)
        self.assertGreater(bid, 0)

    def test_two_bindings_different_ids(self):
        nid  = lib.pe_tree_add(PE_NO_PARENT)
        bid1 = lib.pe_bind_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE)
        bid2 = lib.pe_bind_key(nid, PE_KEY_ENTER,  PE_MOD_NONE)
        self.assertNotEqual(bid1, bid2)

    def test_unbind_removes_binding(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        bid = lib.pe_bind_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE)
        lib.pe_unbind(bid)
        result = lib.pe_dispatch_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE)
        self.assertEqual(result, 0)

    def test_unbind_all_removes_all_node_bindings(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        lib.pe_bind_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE)
        lib.pe_bind_key(nid, PE_KEY_ENTER,  PE_MOD_NONE)
        lib.pe_bind_key(nid, PE_KEY_UP,     PE_MOD_NONE)
        lib.pe_unbind_all(nid)
        self.assertEqual(lib.pe_dispatch_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE), 0)
        self.assertEqual(lib.pe_dispatch_key(nid, PE_KEY_ENTER,  PE_MOD_NONE), 0)
        self.assertEqual(lib.pe_dispatch_key(nid, PE_KEY_UP,     PE_MOD_NONE), 0)

    def test_unbind_all_does_not_affect_other_node(self):
        nid1 = lib.pe_tree_add(PE_NO_PARENT)
        nid2 = lib.pe_tree_add(PE_NO_PARENT)
        bid  = lib.pe_bind_key(nid2, PE_KEY_ESCAPE, PE_MOD_NONE)
        lib.pe_unbind_all(nid1)
        result = lib.pe_dispatch_key(nid2, PE_KEY_ESCAPE, PE_MOD_NONE)
        self.assertEqual(result, bid)


class TestDispatchDirect(unittest.TestCase):

    def setUp(self):
        lib.pe_reset_all()

    def test_dispatch_match_returns_binding_id(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        bid = lib.pe_bind_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE)
        result = lib.pe_dispatch_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE)
        self.assertEqual(result, bid)

    def test_dispatch_wrong_key_returns_zero(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        lib.pe_bind_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE)
        result = lib.pe_dispatch_key(nid, PE_KEY_ENTER, PE_MOD_NONE)
        self.assertEqual(result, 0)

    def test_dispatch_wrong_modifiers_returns_zero(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        lib.pe_bind_key(nid, PE_KEY_ESCAPE, PE_MOD_CTRL)
        result = lib.pe_dispatch_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE)
        self.assertEqual(result, 0)

    def test_dispatch_exact_modifiers_match(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        bid = lib.pe_bind_key(nid, ord('q'), PE_MOD_CTRL)
        result = lib.pe_dispatch_key(nid, ord('q'), PE_MOD_CTRL)
        self.assertEqual(result, bid)

    def test_dispatch_no_binding_returns_zero(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        result = lib.pe_dispatch_key(nid, PE_KEY_ESCAPE, PE_MOD_NONE)
        self.assertEqual(result, 0)


class TestDispatchBubbling(unittest.TestCase):

    def setUp(self):
        lib.pe_reset_all()

    def test_event_bubbles_to_parent(self):
        parent = lib.pe_tree_add(PE_NO_PARENT)
        child  = lib.pe_tree_add(parent)
        bid    = lib.pe_bind_key(parent, PE_KEY_ESCAPE, PE_MOD_NONE)
        result = lib.pe_dispatch_key(child, PE_KEY_ESCAPE, PE_MOD_NONE)
        self.assertEqual(result, bid)

    def test_child_binding_takes_priority(self):
        parent = lib.pe_tree_add(PE_NO_PARENT)
        child  = lib.pe_tree_add(parent)
        lib.pe_bind_key(parent, PE_KEY_ESCAPE, PE_MOD_NONE)
        bid_child = lib.pe_bind_key(child, PE_KEY_ESCAPE, PE_MOD_NONE)
        result = lib.pe_dispatch_key(child, PE_KEY_ESCAPE, PE_MOD_NONE)
        self.assertEqual(result, bid_child)

    def test_bubbles_through_multiple_levels(self):
        root      = lib.pe_tree_add(PE_NO_PARENT)
        mid       = lib.pe_tree_add(root)
        leaf      = lib.pe_tree_add(mid)
        bid_root  = lib.pe_bind_key(root, PE_KEY_F5, PE_MOD_NONE)
        result    = lib.pe_dispatch_key(leaf, PE_KEY_F5, PE_MOD_NONE)
        self.assertEqual(result, bid_root)

    def test_no_handler_in_tree_returns_zero(self):
        root  = lib.pe_tree_add(PE_NO_PARENT)
        child = lib.pe_tree_add(root)
        result = lib.pe_dispatch_key(child, PE_KEY_ESCAPE, PE_MOD_NONE)
        self.assertEqual(result, 0)


class TestFocus(unittest.TestCase):

    def setUp(self):
        lib.pe_reset_all()

    def test_initial_focus_is_minus_one(self):
        self.assertEqual(lib.pe_focus_get(), -1)

    def test_set_and_get_focus(self):
        nid = lib.pe_tree_add(PE_NO_PARENT)
        lib.pe_focus_set(nid)
        self.assertEqual(lib.pe_focus_get(), nid)

    def test_focus_changes(self):
        n1 = lib.pe_tree_add(PE_NO_PARENT)
        n2 = lib.pe_tree_add(PE_NO_PARENT)
        lib.pe_focus_set(n1)
        lib.pe_focus_set(n2)
        self.assertEqual(lib.pe_focus_get(), n2)


if __name__ == "__main__":
    unittest.main()
