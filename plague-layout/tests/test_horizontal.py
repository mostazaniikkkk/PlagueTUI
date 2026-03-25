import unittest
from conftest import (lib, TG_Spacing, TG_Region,
                      no_spacing, fixed, fr, compute, result)


def setup(children_specs):
    lib.pl_tree_init()
    root = lib.pl_node_horizontal(fr(1), fr(1), no_spacing())
    ri   = lib.pl_tree_add(root)
    ids  = []
    for (w, h) in children_specs:
        node = lib.pl_node_horizontal(w, h, no_spacing())
        ci = lib.pl_tree_add(node)
        lib.pl_tree_set_child(ri, ci)
        ids.append(ci)
    return ri, ids


class TestHorizontalSingleChild(unittest.TestCase):

    def test_single_fr_fills_container(self):
        ri, [ci] = setup([(fr(1), fr(1))])
        compute(ri, w=80, h=24)
        r = result(ci).region
        self.assertEqual(r.width,  80)
        self.assertEqual(r.height, 24)

    def test_single_fixed_width(self):
        ri, [ci] = setup([(fixed(20), fr(1))])
        compute(ri, w=80, h=24)
        self.assertEqual(result(ci).region.width,  20)
        self.assertEqual(result(ci).region.height, 24)


class TestHorizontalTwoChildren(unittest.TestCase):

    def test_two_fr1_split_equally(self):
        ri, [a, b] = setup([(fr(1), fr(1)), (fr(1), fr(1))])
        compute(ri, w=80, h=24)
        self.assertEqual(result(a).region.width, 40)
        self.assertEqual(result(b).region.width, 40)

    def test_side_by_side_x_positions(self):
        ri, [a, b] = setup([(fr(1), fr(1)), (fr(1), fr(1))])
        compute(ri, w=80, h=24)
        self.assertEqual(result(a).region.x, 0)
        self.assertEqual(result(b).region.x, 40)

    def test_children_inherit_full_height(self):
        ri, [a, b] = setup([(fr(1), fr(1)), (fr(1), fr(1))])
        compute(ri, w=80, h=20)
        self.assertEqual(result(a).region.height, 20)
        self.assertEqual(result(b).region.height, 20)

    def test_sidebar_fixed_plus_main_fr(self):
        ri, [sidebar, main] = setup([(fixed(20), fr(1)), (fr(1), fr(1))])
        compute(ri, w=80, h=24)
        self.assertEqual(result(sidebar).region.width, 20)
        self.assertEqual(result(sidebar).region.x, 0)
        self.assertEqual(result(main).region.width, 60)   # 80-20
        self.assertEqual(result(main).region.x, 20)

    def test_fr2_gets_double_fr1(self):
        ri, [a, b] = setup([(fr(1), fr(1)), (fr(2), fr(1))])
        compute(ri, w=90, h=24)
        # total fr=3, remaining=90: a=30, b=60
        self.assertEqual(result(a).region.width, 30)
        self.assertEqual(result(b).region.width, 60)

    def test_two_fixed_children(self):
        ri, [a, b] = setup([(fixed(30), fr(1)), (fixed(50), fr(1))])
        compute(ri, w=80, h=24)
        self.assertEqual(result(a).region.width, 30)
        self.assertEqual(result(a).region.x, 0)
        self.assertEqual(result(b).region.width, 50)
        self.assertEqual(result(b).region.x, 30)


class TestHorizontalThreeChildren(unittest.TestCase):

    def test_three_equal_fr(self):
        ri, [a, b, c] = setup([(fr(1), fr(1))] * 3)
        compute(ri, w=90, h=24)
        self.assertEqual(result(a).region.width, 30)
        self.assertEqual(result(b).region.width, 30)
        self.assertEqual(result(c).region.width, 30)

    def test_fixed_fr_fixed(self):
        ri, [a, b, c] = setup([
            (fixed(10), fr(1)),
            (fr(1),     fr(1)),
            (fixed(10), fr(1)),
        ])
        compute(ri, w=80, h=24)
        self.assertEqual(result(a).region.width, 10)
        self.assertEqual(result(b).region.width, 60)  # 80-10-10
        self.assertEqual(result(c).region.width, 10)
        self.assertEqual(result(c).region.x,     70)


if __name__ == "__main__":
    unittest.main()
