import unittest
from conftest import comp, PC_WidgetNode, TG_Region, PG_Color, PC_NODE_CONTAINER, PC_NODE_LABEL


def make_node(region=None, visible=1):
    node = PC_WidgetNode()
    node.type    = PC_NODE_CONTAINER
    node.region  = region or TG_Region(0, 0, 100, 50)
    node.parent  = -1
    node.visible = visible
    return node


class TestTreeInit(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()

    def test_starts_empty(self):
        self.assertEqual(comp.pc_tree_count(), 0)

    def test_reinit_clears(self):
        comp.pc_tree_add(make_node())
        comp.pc_tree_add(make_node())
        comp.pc_tree_init()
        self.assertEqual(comp.pc_tree_count(), 0)


class TestTreeAdd(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()

    def test_add_returns_index(self):
        idx = comp.pc_tree_add(make_node())
        self.assertEqual(idx, 0)

    def test_add_multiple_consecutive_indices(self):
        i0 = comp.pc_tree_add(make_node())
        i1 = comp.pc_tree_add(make_node())
        i2 = comp.pc_tree_add(make_node())
        self.assertEqual((i0, i1, i2), (0, 1, 2))

    def test_count_increments(self):
        comp.pc_tree_add(make_node())
        comp.pc_tree_add(make_node())
        self.assertEqual(comp.pc_tree_count(), 2)

    def test_full_returns_minus_one(self):
        for _ in range(256):
            comp.pc_tree_add(make_node())
        self.assertEqual(comp.pc_tree_add(make_node()), -1)

    def test_count_max_256(self):
        for _ in range(256):
            comp.pc_tree_add(make_node())
        self.assertEqual(comp.pc_tree_count(), 256)


class TestTreeGet(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()

    def test_get_region(self):
        node = make_node(TG_Region(10, 20, 200, 80))
        comp.pc_tree_add(node)
        got = comp.pc_tree_get(0)
        self.assertAlmostEqual(got.region.x,      10)
        self.assertAlmostEqual(got.region.y,       20)
        self.assertAlmostEqual(got.region.width,  200)
        self.assertAlmostEqual(got.region.height,  80)

    def test_get_visible(self):
        comp.pc_tree_add(make_node(visible=0))
        got = comp.pc_tree_get(0)
        self.assertEqual(got.visible, 0)

    def test_get_out_of_range_returns_zero(self):
        got = comp.pc_tree_get(99)
        self.assertAlmostEqual(got.region.x, 0)
        self.assertAlmostEqual(got.region.width, 0)


class TestTreeSetChild(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()

    def test_set_child_links_parent(self):
        p = comp.pc_tree_add(make_node())
        c = comp.pc_tree_add(make_node())
        comp.pc_tree_set_child(p, c)
        parent_node = comp.pc_tree_get(p)
        self.assertEqual(parent_node.child_count, 1)
        self.assertEqual(parent_node.children[0], c)

    def test_set_child_updates_parent_in_child(self):
        p = comp.pc_tree_add(make_node())
        c = comp.pc_tree_add(make_node())
        comp.pc_tree_set_child(p, c)
        child_node = comp.pc_tree_get(c)
        self.assertEqual(child_node.parent, p)

    def test_set_multiple_children(self):
        p  = comp.pc_tree_add(make_node())
        c0 = comp.pc_tree_add(make_node())
        c1 = comp.pc_tree_add(make_node())
        c2 = comp.pc_tree_add(make_node())
        comp.pc_tree_set_child(p, c0)
        comp.pc_tree_set_child(p, c1)
        comp.pc_tree_set_child(p, c2)
        pn = comp.pc_tree_get(p)
        self.assertEqual(pn.child_count, 3)
        self.assertEqual(list(pn.children[:3]), [c0, c1, c2])

    def test_set_child_invalid_index_ignored(self):
        p = comp.pc_tree_add(make_node())
        comp.pc_tree_set_child(p, 99)
        pn = comp.pc_tree_get(p)
        self.assertEqual(pn.child_count, 0)


if __name__ == "__main__":
    unittest.main()
