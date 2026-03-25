import ctypes
import unittest
from conftest import (
    comp, dc, STUB_CTX,
    PC_WidgetNode, TG_Region, TG_Offset, PG_Color, PG_TextStyle,
    PC_NODE_CONTAINER, PC_NODE_LABEL, PC_NODE_PANEL,
    CMD_FILL_RECT, CMD_STROKE_RECT, CMD_DRAW_TEXT,
)


def render(root_idx=0):
    comp.pc_tree_render(root_idx, ctypes.byref(STUB_CTX))


def red():
    return PG_Color(1.0, 0.0, 0.0, 1.0)

def transparent():
    return PG_Color(0.0, 0.0, 0.0, 0.0)

def region(x=0, y=0, w=100, h=50):
    return TG_Region(x, y, w, h)


class TestRenderEmpty(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()
        dc.pg_stub_reset()

    def test_empty_tree_no_commands(self):
        render()
        self.assertEqual(dc.pg_stub_count(), 0)

    def test_invalid_root_no_commands(self):
        comp.pc_tree_render(99, ctypes.byref(STUB_CTX))
        self.assertEqual(dc.pg_stub_count(), 0)


class TestRenderBackground(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()
        dc.pg_stub_reset()

    def test_fill_rect_emitted(self):
        node = PC_WidgetNode()
        node.region   = region(10, 20, 200, 100)
        node.bg_color = red()
        node.parent   = -1
        node.visible  = 1
        comp.pc_tree_add(node)
        render()
        self.assertEqual(dc.pg_stub_count(), 1)
        cmd = dc.pg_stub_get(0)
        self.assertEqual(cmd.type, CMD_FILL_RECT)
        self.assertEqual(cmd.data.fill_rect.region.x,     10)
        self.assertEqual(cmd.data.fill_rect.region.width, 200)
        self.assertAlmostEqual(cmd.data.fill_rect.color.r, 1.0)

    def test_zero_alpha_no_background(self):
        node = PC_WidgetNode()
        node.region   = region()
        node.bg_color = transparent()
        node.parent   = -1
        node.visible  = 1
        comp.pc_tree_add(node)
        render()
        self.assertEqual(dc.pg_stub_count(), 0)


class TestRenderBorder(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()
        dc.pg_stub_reset()

    def test_stroke_rect_emitted(self):
        node = PC_WidgetNode()
        node.region       = region()
        node.border_color = PG_Color(0.0, 0.0, 1.0, 1.0)
        node.border_width = 2.0
        node.parent       = -1
        node.visible      = 1
        comp.pc_tree_add(node)
        render()
        self.assertEqual(dc.pg_stub_count(), 1)
        cmd = dc.pg_stub_get(0)
        self.assertEqual(cmd.type, CMD_STROKE_RECT)
        self.assertAlmostEqual(cmd.data.stroke_rect.stroke_width, 2.0)

    def test_zero_border_not_emitted(self):
        node = PC_WidgetNode()
        node.region       = region()
        node.border_width = 0.0
        node.parent       = -1
        node.visible      = 1
        comp.pc_tree_add(node)
        render()
        self.assertEqual(dc.pg_stub_count(), 0)


class TestRenderText(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()
        dc.pg_stub_reset()

    def test_draw_text_emitted(self):
        node = PC_WidgetNode()
        node.region  = region(5, 10, 150, 30)
        node.text    = b"hello world"
        node.parent  = -1
        node.visible = 1
        comp.pc_tree_add(node)
        render()
        self.assertEqual(dc.pg_stub_count(), 1)
        cmd = dc.pg_stub_get(0)
        self.assertEqual(cmd.type, CMD_DRAW_TEXT)
        self.assertEqual(cmd.data.draw_text.pos.x,  5)
        self.assertEqual(cmd.data.draw_text.pos.y, 10)

    def test_empty_text_not_emitted(self):
        node = PC_WidgetNode()
        node.region  = region()
        node.text    = b""
        node.parent  = -1
        node.visible = 1
        comp.pc_tree_add(node)
        render()
        self.assertEqual(dc.pg_stub_count(), 0)


class TestRenderVisibility(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()
        dc.pg_stub_reset()

    def test_hidden_node_not_emitted(self):
        node = PC_WidgetNode()
        node.region   = region()
        node.bg_color = red()
        node.parent   = -1
        node.visible  = 0
        comp.pc_tree_add(node)
        render()
        self.assertEqual(dc.pg_stub_count(), 0)

    def test_hidden_child_not_emitted(self):
        parent = PC_WidgetNode()
        parent.region   = region()
        parent.bg_color = red()
        parent.parent   = -1
        parent.visible  = 1
        pi = comp.pc_tree_add(parent)

        child = PC_WidgetNode()
        child.region   = region()
        child.bg_color = red()
        child.parent   = -1
        child.visible  = 0
        ci = comp.pc_tree_add(child)

        comp.pc_tree_set_child(pi, ci)
        render()
        self.assertEqual(dc.pg_stub_count(), 1)


class TestRenderOrder(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()
        dc.pg_stub_reset()

    def test_parent_before_child(self):
        """Parent emits fill_rect before child."""
        parent = PC_WidgetNode()
        parent.region   = region(0, 0, 200, 200)
        parent.bg_color = PG_Color(1.0, 0.0, 0.0, 1.0)
        parent.parent   = -1
        parent.visible  = 1
        pi = comp.pc_tree_add(parent)

        child = PC_WidgetNode()
        child.region   = region(10, 10, 50, 50)
        child.bg_color = PG_Color(0.0, 1.0, 0.0, 1.0)
        child.parent   = -1
        child.visible  = 1
        ci = comp.pc_tree_add(child)

        comp.pc_tree_set_child(pi, ci)
        render()

        self.assertEqual(dc.pg_stub_count(), 2)
        self.assertEqual(dc.pg_stub_get(0).data.fill_rect.region.width, 200)
        self.assertEqual(dc.pg_stub_get(1).data.fill_rect.region.width,  50)

    def test_order_bg_border_text(self):
        """A node with background + border + text emits in that order."""
        node = PC_WidgetNode()
        node.region       = region(0, 0, 100, 40)
        node.bg_color     = red()
        node.border_color = PG_Color(0.0, 0.0, 0.0, 1.0)
        node.border_width = 1.0
        node.text         = b"label"
        node.parent       = -1
        node.visible      = 1
        comp.pc_tree_add(node)
        render()

        self.assertEqual(dc.pg_stub_count(), 3)
        self.assertEqual(dc.pg_stub_get(0).type, CMD_FILL_RECT)
        self.assertEqual(dc.pg_stub_get(1).type, CMD_STROKE_RECT)
        self.assertEqual(dc.pg_stub_get(2).type, CMD_DRAW_TEXT)


class TestRenderHelpers(unittest.TestCase):

    def setUp(self):
        comp.pc_tree_init()
        dc.pg_stub_reset()

    def test_panel_node_emits_fill_rect(self):
        node = comp.pc_node_panel(region(0, 0, 80, 40), red())
        comp.pc_tree_add(node)
        render()
        self.assertEqual(dc.pg_stub_count(), 1)
        self.assertEqual(dc.pg_stub_get(0).type, CMD_FILL_RECT)

    def test_container_node_no_appearance_not_emitted(self):
        node = comp.pc_node_container(region())
        comp.pc_tree_add(node)
        render()
        self.assertEqual(dc.pg_stub_count(), 0)

    def test_label_node_emits_draw_text(self):
        style = PG_TextStyle()
        style.font_name = b"Inter"
        style.font_size = 14.0
        node = comp.pc_node_label(region(), b"hello", style)
        comp.pc_tree_add(node)
        render()
        self.assertEqual(dc.pg_stub_count(), 1)
        self.assertEqual(dc.pg_stub_get(0).type, CMD_DRAW_TEXT)


if __name__ == "__main__":
    unittest.main()
