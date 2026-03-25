import unittest
from conftest import (lib, TG_Spacing, no_spacing, fixed, fr, compute, result)


def leaf():
    return lib.pl_node_vertical(fr(1), fr(1), no_spacing())


class TestGrid2x2(unittest.TestCase):

    def setUp(self):
        lib.pl_tree_init()
        self.root = lib.pl_tree_add(lib.pl_node_grid(fr(1), fr(1), 2, 2))
        self.cells = [lib.pl_tree_add(leaf()) for _ in range(4)]
        for ci in self.cells:
            lib.pl_tree_set_child(self.root, ci)
        compute(self.root, w=80, h=24)

    def test_cell_size(self):
        for ci in self.cells:
            r = result(ci).region
            self.assertEqual(r.width,  40)
            self.assertEqual(r.height, 12)

    def test_positions_row_major(self):
        expected = [(0, 0), (40, 0), (0, 12), (40, 12)]
        for ci, (ex, ey) in zip(self.cells, expected):
            r = result(ci).region
            self.assertEqual(r.x, ex, f"cell {ci} x")
            self.assertEqual(r.y, ey, f"cell {ci} y")


class TestGrid3x1(unittest.TestCase):

    def setUp(self):
        lib.pl_tree_init()
        self.root = lib.pl_tree_add(lib.pl_node_grid(fr(1), fr(1), 3, 1))
        self.cells = [lib.pl_tree_add(leaf()) for _ in range(3)]
        for ci in self.cells:
            lib.pl_tree_set_child(self.root, ci)
        compute(self.root, w=90, h=24)

    def test_equal_widths(self):
        for ci in self.cells:
            self.assertEqual(result(ci).region.width, 30)

    def test_all_same_row(self):
        for ci in self.cells:
            self.assertEqual(result(ci).region.y, 0)

    def test_x_positions(self):
        xs = [result(ci).region.x for ci in self.cells]
        self.assertEqual(xs, [0, 30, 60])


class TestGrid1x3(unittest.TestCase):

    def setUp(self):
        lib.pl_tree_init()
        self.root = lib.pl_tree_add(lib.pl_node_grid(fr(1), fr(1), 1, 3))
        self.cells = [lib.pl_tree_add(leaf()) for _ in range(3)]
        for ci in self.cells:
            lib.pl_tree_set_child(self.root, ci)
        compute(self.root, w=80, h=24)

    def test_equal_heights(self):
        for ci in self.cells:
            self.assertEqual(result(ci).region.height, 8)

    def test_all_same_column(self):
        for ci in self.cells:
            self.assertEqual(result(ci).region.x, 0)

    def test_y_positions(self):
        ys = [result(ci).region.y for ci in self.cells]
        self.assertEqual(ys, [0, 8, 16])


class TestGridWithPadding(unittest.TestCase):

    def test_grid_children_use_content_region(self):
        lib.pl_tree_init()
        padding = TG_Spacing(2, 2, 2, 2)
        root = lib.pl_tree_add(lib.pl_node_grid(fr(1), fr(1), 2, 1))
        root_node = lib.pl_tree_get(0)
        root_node.padding = padding
        # Re-añadir con padding (la forma más simple dado que no hay setter separado)
        lib.pl_tree_init()
        import ctypes
        gnode = lib.pl_node_grid(fr(1), fr(1), 2, 1)
        gnode.padding = padding
        ri = lib.pl_tree_add(gnode)
        cells = [lib.pl_tree_add(leaf()) for _ in range(2)]
        for ci in cells: lib.pl_tree_set_child(ri, ci)
        compute(ri, w=84, h=12)
        # content: x=2, y=2, w=80, h=8
        # cell_w = 80/2 = 40
        r0 = result(cells[0]).region
        self.assertEqual(r0.x,     2)
        self.assertEqual(r0.y,     2)
        self.assertEqual(r0.width, 40)

    def test_grid_stops_at_rows_limit(self):
        """Hijos extra (más allá de cols*rows) no se colocan."""
        lib.pl_tree_init()
        root = lib.pl_tree_add(lib.pl_node_grid(fr(1), fr(1), 2, 1))
        cells = [lib.pl_tree_add(leaf()) for _ in range(4)]  # solo 2 celdas en grid
        for ci in cells: lib.pl_tree_set_child(root, ci)
        compute(root, w=80, h=10)
        # Los primeros 2 tienen región; los otros 2 tienen región {0,0,0,0}
        r2 = result(cells[2]).region
        r3 = result(cells[3]).region
        self.assertEqual(r2.width,  0)
        self.assertEqual(r3.height, 0)


if __name__ == "__main__":
    unittest.main()
