"""
Carga compartida de DLLs para todos los tests del compositor.
Importar: from conftest import comp, dc, PC_DrawContext, PC_WidgetNode, ...
"""

import ctypes
import sys
from pathlib import Path

root    = Path(__file__).parent.parent
dc_root = root.parent / "plague-drawcontext"

if sys.platform == "win32":
    comp_path = root    / "bin" / "plague_compositor.dll"
    dc_path   = dc_root / "bin" / "plague_drawcontext.dll"
elif sys.platform == "darwin":
    comp_path = root    / "bin" / "libplague_compositor.dylib"
    dc_path   = dc_root / "bin" / "libplague_drawcontext.dylib"
else:
    comp_path = root    / "bin" / "libplague_compositor.so"
    dc_path   = dc_root / "bin" / "libplague_drawcontext.so"

dc   = ctypes.CDLL(str(dc_path))
comp = ctypes.CDLL(str(comp_path))

# ---------------------------------------------------------------------------
# Tipos de plague-geometry
# ---------------------------------------------------------------------------

class TG_Offset(ctypes.Structure):
    _fields_ = [("x", ctypes.c_int), ("y", ctypes.c_int)]

class TG_Region(ctypes.Structure):
    _fields_ = [
        ("x",      ctypes.c_int),
        ("y",      ctypes.c_int),
        ("width",  ctypes.c_int),
        ("height", ctypes.c_int),
    ]

# ---------------------------------------------------------------------------
# Tipos de plague-drawcontext
# ---------------------------------------------------------------------------

class PG_Color(ctypes.Structure):
    _fields_ = [
        ("r", ctypes.c_float),
        ("g", ctypes.c_float),
        ("b", ctypes.c_float),
        ("a", ctypes.c_float),
    ]

class PG_TextStyle(ctypes.Structure):
    _fields_ = [
        ("font_name", ctypes.c_char * 64),
        ("font_size", ctypes.c_float),
        ("color",     PG_Color),
        ("bold",      ctypes.c_int),
        ("italic",    ctypes.c_int),
    ]

# Firmas del StubContext (plague-drawcontext)
dc.pg_stub_reset.argtypes = []
dc.pg_stub_reset.restype  = None

dc.pg_stub_count.argtypes = []
dc.pg_stub_count.restype  = ctypes.c_int

dc.pg_stub_fill_rect.argtypes   = [TG_Region, PG_Color]
dc.pg_stub_fill_rect.restype    = None

dc.pg_stub_stroke_rect.argtypes = [TG_Region, PG_Color, ctypes.c_float]
dc.pg_stub_stroke_rect.restype  = None

dc.pg_stub_draw_text.argtypes   = [TG_Offset, ctypes.c_char_p, PG_TextStyle]
dc.pg_stub_draw_text.restype    = None

dc.pg_stub_clip_push.argtypes    = [TG_Region]
dc.pg_stub_clip_push.restype     = None

dc.pg_stub_clip_pop.argtypes     = []
dc.pg_stub_clip_pop.restype      = None

dc.pg_stub_translate_push.argtypes = [TG_Offset]
dc.pg_stub_translate_push.restype  = None

dc.pg_stub_translate_pop.argtypes  = []
dc.pg_stub_translate_pop.restype   = None

# ---------------------------------------------------------------------------
# DrawCommand (para inspeccionar comandos emitidos)
# ---------------------------------------------------------------------------

class FillRectData(ctypes.Structure):
    _fields_ = [("region", TG_Region), ("color", PG_Color)]

class StrokeRectData(ctypes.Structure):
    _fields_ = [("region", TG_Region), ("color", PG_Color), ("stroke_width", ctypes.c_float)]

class DrawTextData(ctypes.Structure):
    _fields_ = [("pos", TG_Offset), ("text", ctypes.c_char * 256), ("style", PG_TextStyle)]

class ClipPushData(ctypes.Structure):
    _fields_ = [("region", TG_Region)]

class TranslatePushData(ctypes.Structure):
    _fields_ = [("offset", TG_Offset)]

class DrawCommandUnion(ctypes.Union):
    _fields_ = [
        ("fill_rect",      FillRectData),
        ("stroke_rect",    StrokeRectData),
        ("draw_text",      DrawTextData),
        ("clip_push",      ClipPushData),
        ("translate_push", TranslatePushData),
    ]

class DrawCommand(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int), ("data", DrawCommandUnion)]

dc.pg_stub_get.argtypes = [ctypes.c_int]
dc.pg_stub_get.restype  = DrawCommand

# Constantes de tipo de comando
CMD_FILL_RECT      = 0
CMD_STROKE_RECT    = 1
CMD_DRAW_TEXT      = 2
CMD_CLIP_PUSH      = 3
CMD_CLIP_POP       = 4
CMD_TRANSLATE_PUSH = 5
CMD_TRANSLATE_POP  = 6

# ---------------------------------------------------------------------------
# PC_DrawContext — vtable para el compositor
# ---------------------------------------------------------------------------

FillRectFn      = ctypes.CFUNCTYPE(None, TG_Region, PG_Color)
StrokeRectFn    = ctypes.CFUNCTYPE(None, TG_Region, PG_Color, ctypes.c_float)
DrawTextFn      = ctypes.CFUNCTYPE(None, TG_Offset, ctypes.c_char_p, PG_TextStyle)
ClipPushFn      = ctypes.CFUNCTYPE(None, TG_Region)
ClipPopFn       = ctypes.CFUNCTYPE(None)
TranslatePushFn = ctypes.CFUNCTYPE(None, TG_Offset)
TranslatePopFn  = ctypes.CFUNCTYPE(None)

class PC_DrawContext(ctypes.Structure):
    _fields_ = [
        ("fill_rect",      FillRectFn),
        ("stroke_rect",    StrokeRectFn),
        ("draw_text",      DrawTextFn),
        ("clip_push",      ClipPushFn),
        ("clip_pop",       ClipPopFn),
        ("translate_push", TranslatePushFn),
        ("translate_pop",  TranslatePopFn),
    ]

# Vtable preconstruida apuntando al StubContext — usar en tests de rendering
# (guardamos referencias para evitar garbage collection)
_fn_fill_rect      = FillRectFn(dc.pg_stub_fill_rect)
_fn_stroke_rect    = StrokeRectFn(dc.pg_stub_stroke_rect)
_fn_draw_text      = DrawTextFn(dc.pg_stub_draw_text)
_fn_clip_push      = ClipPushFn(dc.pg_stub_clip_push)
_fn_clip_pop       = ClipPopFn(dc.pg_stub_clip_pop)
_fn_translate_push = TranslatePushFn(dc.pg_stub_translate_push)
_fn_translate_pop  = TranslatePopFn(dc.pg_stub_translate_pop)

STUB_CTX = PC_DrawContext(
    fill_rect      = _fn_fill_rect,
    stroke_rect    = _fn_stroke_rect,
    draw_text      = _fn_draw_text,
    clip_push      = _fn_clip_push,
    clip_pop       = _fn_clip_pop,
    translate_push = _fn_translate_push,
    translate_pop  = _fn_translate_pop,
)

# ---------------------------------------------------------------------------
# PC_WidgetNode
# ---------------------------------------------------------------------------

class PC_WidgetNode(ctypes.Structure):
    _fields_ = [
        ("type",         ctypes.c_int),
        ("region",       TG_Region),
        ("bg_color",     PG_Color),
        ("border_color", PG_Color),
        ("border_width", ctypes.c_float),
        ("text",         ctypes.c_char * 256),
        ("text_style",   PG_TextStyle),
        ("children",     ctypes.c_int * 32),
        ("child_count",  ctypes.c_int),
        ("parent",       ctypes.c_int),
        ("visible",      ctypes.c_int),
    ]

# Constantes PC_NodeType
PC_NODE_CONTAINER = 0
PC_NODE_LABEL     = 1
PC_NODE_PANEL     = 2

# ---------------------------------------------------------------------------
# Firmas del compositor
# ---------------------------------------------------------------------------

comp.pc_tree_init.argtypes     = []
comp.pc_tree_init.restype      = None

comp.pc_tree_add.argtypes      = [PC_WidgetNode]
comp.pc_tree_add.restype       = ctypes.c_int

comp.pc_tree_set_child.argtypes = [ctypes.c_int, ctypes.c_int]
comp.pc_tree_set_child.restype  = None

comp.pc_tree_count.argtypes    = []
comp.pc_tree_count.restype     = ctypes.c_int

comp.pc_tree_get.argtypes      = [ctypes.c_int]
comp.pc_tree_get.restype       = PC_WidgetNode

comp.pc_tree_render.argtypes   = [ctypes.c_int, ctypes.POINTER(PC_DrawContext)]
comp.pc_tree_render.restype    = None

comp.pc_node_container.argtypes = [TG_Region]
comp.pc_node_container.restype  = PC_WidgetNode

comp.pc_node_label.argtypes     = [TG_Region, ctypes.c_char_p, PG_TextStyle]
comp.pc_node_label.restype      = PC_WidgetNode

comp.pc_node_panel.argtypes     = [TG_Region, PG_Color]
comp.pc_node_panel.restype      = PC_WidgetNode
