import unittest
from conftest import (lib, PL_LayoutNode, TG_Spacing, TG_Region,
                      PL_LAYOUT_VERTICAL, no_spacing, fixed, fr, compute, result)


def setup(children_specs):
    """Crea árbol vertical con hijos dados como (width_sv, height_sv)."""
    lib.pl_tree_init()
    root = lib.pl_node_vertical(fr(1), fr(1), no_spacing())
    ri   = lib.pl_tree_add(root)
    ids  = []
    for (w, h) in children_specs:
        node = lib.pl_node_vertical(w, h, no_spacing())
        ci = lib.pl_tree_add(node)
        lib.pl_tree_set_child(ri, ci)
        ids.append(ci)
    return ri, ids


class TestVerticalSingleChild(unittest.TestCase):

    def test_single_fr_child_fills_container(self):
        ri, [ci] = setup([(fr(1), fr(1))])
        compute(ri, w=80, h=24)
        r = result(ci).region
        self.assertEqual(r.x, 0);  self.assertEqual(r.y, 0)
        self.assertEqual(r.width, 80);  self.assertEqual(r.height, 24)

    def test_single_fixed_child(self):
        ri, [ci] = setup([(fr(1), fixed(10))])
        compute(ri, w=80, h=24)
        r = result(ci).region
        self.assertEqual(r.height, 10)
        self.assertEqual(r.width,  80)

    def test_root_region_equals_container(self):
        ri, _ = setup([(fr(1), fr(1))])
        compute(ri, x=5, y=3, w=70, h=20)
        r = result(ri).region
        self.assertEqual(r.x, 5);    self.assertEqual(r.y, 3)
        self.assertEqual(r.width, 70); self.assertEqual(r.height, 20)


class TestVerticalTwoChildren(unittest.TestCase):

    def test_two_fr1_children_split_equally(self):
        ri, [a, b] = setup([(fr(1), fr(1)), (fr(1), fr(1))])
        compute(ri, w=80, h=24)
        self.assertEqual(result(a).region.height, 12)
        self.assertEqual(result(b).region.height, 12)

    def test_two_fr1_children_stacked_vertically(self):
        ri, [a, b] = setup([(fr(1), fr(1)), (fr(1), fr(1))])
        compute(ri, w=80, h=24)
        self.assertEqual(result(a).region.y, 0)
        self.assertEqual(result(b).region.y, 12)

    def test_fixed_plus_fr(self):
        ri, [a, b] = setup([(fr(1), fixed(5)), (fr(1), fr(1))])
        compute(ri, w=80, h=24)
        self.assertEqual(result(a).region.height, 5)
        self.assertEqual(result(b).region.height, 19)  # 24-5

    def test_two_fixed_children(self):
        ri, [a, b] = setup([(fr(1), fixed(8)), (fr(1), fixed(6))])
        compute(ri, w=80, h=24)
        self.assertEqual(result(a).region.y, 0)
        self.assertEqual(result(a).region.height, 8)
        self.assertEqual(result(b).region.y, 8)
        self.assertEqual(result(b).region.height, 6)

    def test_children_inherit_full_width(self):
        ri, [a, b] = setup([(fr(1), fr(1)), (fr(1), fr(1))])
        compute(ri, w=60, h=20)
        self.assertEqual(result(a).region.width, 60)
        self.assertEqual(result(b).region.width, 60)


class TestVerticalFrWeights(unittest.TestCase):

    def test_fr2_gets_double_fr1(self):
        ri, [a, b] = setup([(fr(1), fr(1)), (fr(1), fr(2))])
        compute(ri, w=80, h=30)
        # fr1=10, fr2=20
        self.assertEqual(result(a).region.height, 10)
        self.assertEqual(result(b).region.height, 20)

    def test_fr3_children(self):
        ri, [a, b, c] = setup([(fr(1), fr(1)), (fr(1), fr(1)), (fr(1), fr(1))])
        compute(ri, w=80, h=24)
        self.assertEqual(result(a).region.height, 8)
        self.assertEqual(result(b).region.height, 8)
        self.assertEqual(result(c).region.height, 8)


class TestVerticalPadding(unittest.TestCase):

    def test_padding_shrinks_content_region(self):
        lib.pl_tree_init()
        padding = TG_Spacing(2, 3, 2, 3)  # top=2, right=3, bottom=2, left=3
        root = lib.pl_node_vertical(fr(1), fr(1), padding)
        ri = lib.pl_tree_add(root)
        compute(ri, w=80, h=24)
        cr = result(ri).content_region
        self.assertEqual(cr.x,      3)
        self.assertEqual(cr.y,      2)
        self.assertEqual(cr.width,  74)  # 80-3-3
        self.assertEqual(cr.height, 20)  # 24-2-2

    def test_children_positioned_within_content(self):
        lib.pl_tree_init()
        padding = TG_Spacing(1, 0, 1, 0)
        root = lib.pl_node_vertical(fr(1), fr(1), padding)
        ri   = lib.pl_tree_add(root)
        child = lib.pl_node_vertical(fr(1), fr(1), TG_Spacing(0, 0, 0, 0))
        ci   = lib.pl_tree_add(child)
        lib.pl_tree_set_child(ri, ci)
        compute(ri, w=80, h=24)
        r = result(ci).region
        self.assertEqual(r.y, 1)         # padding.top=1
        self.assertEqual(r.height, 22)   # 24-1-1


class TestVerticalMargin(unittest.TestCase):

    def test_margin_offsets_child(self):
        lib.pl_tree_init()
        root = lib.pl_node_vertical(fr(1), fr(1), no_spacing())
        ri   = lib.pl_tree_add(root)

        child = lib.pl_node_vertical(fr(1), fixed(5), TG_Spacing(0, 0, 0, 0))
        child.margin = TG_Spacing(2, 0, 0, 4)  # top=2, left=4
        ci = lib.pl_tree_add(child)
        lib.pl_tree_set_child(ri, ci)
        compute(ri, w=80, h=24)

        r = result(ci).region
        self.assertEqual(r.x, 4)  # margin.left
        self.assertEqual(r.y, 2)  # margin.top
        self.assertEqual(r.height, 5)
        self.assertEqual(r.width,  76)  # 80 - margin.left - margin.right


if __name__ == "__main__":
    unittest.main()
