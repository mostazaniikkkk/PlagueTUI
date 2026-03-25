import unittest
from conftest import (lib, TG_Spacing, TG_Region,
                      PL_DOCK_TOP, PL_DOCK_BOTTOM, PL_DOCK_LEFT,
                      PL_DOCK_RIGHT, PL_DOCK_CENTER,
                      no_spacing, fixed, fr, compute, result)


def make_docked(edge, w, h):
    node = lib.pl_node_docked(edge, w, h)
    return lib.pl_tree_add(node)


class TestDockHeaderFooter(unittest.TestCase):
    """Header (top=3) + Footer (bottom=1) + contenido central."""

    def setUp(self):
        lib.pl_tree_init()
        self.root = lib.pl_tree_add(lib.pl_node_dock_root(no_spacing()))
        self.header  = make_docked(PL_DOCK_TOP,    fr(1),  fixed(3))
        self.footer  = make_docked(PL_DOCK_BOTTOM, fr(1),  fixed(1))
        self.content = make_docked(PL_DOCK_CENTER, fr(1),  fr(1))
        lib.pl_tree_set_child(self.root, self.header)
        lib.pl_tree_set_child(self.root, self.footer)
        lib.pl_tree_set_child(self.root, self.content)
        compute(self.root, w=80, h=24)

    def test_header_at_top(self):
        r = result(self.header).region
        self.assertEqual(r.x, 0);   self.assertEqual(r.y, 0)
        self.assertEqual(r.width, 80)
        self.assertEqual(r.height, 3)

    def test_footer_at_bottom(self):
        r = result(self.footer).region
        self.assertEqual(r.y, 23)   # 24-1
        self.assertEqual(r.height, 1)
        self.assertEqual(r.width, 80)

    def test_content_fills_middle(self):
        r = result(self.content).region
        self.assertEqual(r.y, 3)
        self.assertEqual(r.height, 20)  # 24-3-1
        self.assertEqual(r.width,  80)

    def test_content_x_unchanged(self):
        self.assertEqual(result(self.content).region.x, 0)


class TestDockSidebar(unittest.TestCase):
    """Sidebar izquierdo (w=20) + contenido central."""

    def setUp(self):
        lib.pl_tree_init()
        self.root    = lib.pl_tree_add(lib.pl_node_dock_root(no_spacing()))
        self.sidebar = make_docked(PL_DOCK_LEFT,   fixed(20), fr(1))
        self.main    = make_docked(PL_DOCK_CENTER, fr(1),     fr(1))
        lib.pl_tree_set_child(self.root, self.sidebar)
        lib.pl_tree_set_child(self.root, self.main)
        compute(self.root, w=80, h=24)

    def test_sidebar_on_left(self):
        r = result(self.sidebar).region
        self.assertEqual(r.x, 0)
        self.assertEqual(r.width, 20)
        self.assertEqual(r.height, 24)

    def test_main_fills_right(self):
        r = result(self.main).region
        self.assertEqual(r.x, 20)
        self.assertEqual(r.width, 60)  # 80-20


class TestDockFull(unittest.TestCase):
    """Header + Footer + Sidebar izquierdo + contenido central."""

    def setUp(self):
        lib.pl_tree_init()
        self.root    = lib.pl_tree_add(lib.pl_node_dock_root(no_spacing()))
        self.header  = make_docked(PL_DOCK_TOP,    fr(1),    fixed(3))
        self.footer  = make_docked(PL_DOCK_BOTTOM, fr(1),    fixed(1))
        self.sidebar = make_docked(PL_DOCK_LEFT,   fixed(20), fr(1))
        self.main    = make_docked(PL_DOCK_CENTER, fr(1),     fr(1))
        for child in [self.header, self.footer, self.sidebar, self.main]:
            lib.pl_tree_set_child(self.root, child)
        compute(self.root, w=80, h=24)

    def test_header_top(self):
        r = result(self.header).region
        self.assertEqual(r.y, 0);  self.assertEqual(r.height, 3)
        self.assertEqual(r.width, 80)

    def test_footer_bottom(self):
        r = result(self.footer).region
        self.assertEqual(r.y, 23); self.assertEqual(r.height, 1)
        self.assertEqual(r.width, 80)

    def test_sidebar_left_respects_header_footer(self):
        r = result(self.sidebar).region
        self.assertEqual(r.x, 0)
        self.assertEqual(r.width, 20)
        # La sidebar ocupa el espacio restante entre header y footer
        self.assertEqual(r.y,      3)
        self.assertEqual(r.height, 20)  # 24-3-1

    def test_main_center(self):
        r = result(self.main).region
        self.assertEqual(r.x,      20)
        self.assertEqual(r.y,      3)
        self.assertEqual(r.width,  60)  # 80-20
        self.assertEqual(r.height, 20)  # 24-3-1


class TestDockRightSidebar(unittest.TestCase):

    def setUp(self):
        lib.pl_tree_init()
        self.root    = lib.pl_tree_add(lib.pl_node_dock_root(no_spacing()))
        self.panel   = make_docked(PL_DOCK_RIGHT,  fixed(15), fr(1))
        self.main    = make_docked(PL_DOCK_CENTER, fr(1),     fr(1))
        lib.pl_tree_set_child(self.root, self.panel)
        lib.pl_tree_set_child(self.root, self.main)
        compute(self.root, w=80, h=24)

    def test_panel_on_right(self):
        r = result(self.panel).region
        self.assertEqual(r.x,     65)   # 80-15
        self.assertEqual(r.width, 15)

    def test_main_fills_left(self):
        r = result(self.main).region
        self.assertEqual(r.x,     0)
        self.assertEqual(r.width, 65)


if __name__ == "__main__":
    unittest.main()
