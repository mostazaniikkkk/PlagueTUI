"""
Carga compartida del DLL para todos los tests.
Importar: from conftest import lib, Color, TextStyle, DrawCommand, ...
"""

import ctypes
import sys
from pathlib import Path

root = Path(__file__).parent.parent

if sys.platform == "win32":
    lib_path = root / "bin" / "plague_drawcontext.dll"
elif sys.platform == "darwin":
    lib_path = root / "bin" / "libplague_drawcontext.dylib"
else:
    lib_path = root / "bin" / "libplague_drawcontext.so"

lib = ctypes.CDLL(str(lib_path))

# ---------------------------------------------------------------------------
# Structs de plague-geometry (tipos dependientes)
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
# Structs propios
# ---------------------------------------------------------------------------

class Color(ctypes.Structure):
    _fields_ = [
        ("r", ctypes.c_float),
        ("g", ctypes.c_float),
        ("b", ctypes.c_float),
        ("a", ctypes.c_float),
    ]

class TextStyle(ctypes.Structure):
    _fields_ = [
        ("font_name", ctypes.c_char * 64),
        ("font_size", ctypes.c_float),
        ("color",     Color),
        ("bold",      ctypes.c_int),
        ("italic",    ctypes.c_int),
    ]

# ---------------------------------------------------------------------------
# Firmas de funciones — Color
# ---------------------------------------------------------------------------

lib.pg_color_rgba.argtypes      = [ctypes.c_float] * 4
lib.pg_color_rgba.restype       = Color

lib.pg_color_from_hex.argtypes  = [ctypes.c_uint32]
lib.pg_color_from_hex.restype   = Color

lib.pg_color_mix.argtypes       = [Color, Color, ctypes.c_float]
lib.pg_color_mix.restype        = Color

lib.pg_color_with_alpha.argtypes = [Color, ctypes.c_float]
lib.pg_color_with_alpha.restype  = Color

lib.pg_color_eq.argtypes        = [Color, Color]
lib.pg_color_eq.restype         = ctypes.c_int

# ---------------------------------------------------------------------------
# Firmas de funciones — TextStyle
# ---------------------------------------------------------------------------

lib.pg_text_style.argtypes = [ctypes.c_char_p, ctypes.c_float, Color,
                               ctypes.c_int, ctypes.c_int]
lib.pg_text_style.restype  = TextStyle

# ---------------------------------------------------------------------------
# Firmas de funciones — StubContext
# ---------------------------------------------------------------------------

lib.pg_stub_reset.argtypes  = []
lib.pg_stub_reset.restype   = None

lib.pg_stub_count.argtypes  = []
lib.pg_stub_count.restype   = ctypes.c_int

lib.pg_stub_fill_rect.argtypes      = [TG_Region, Color]
lib.pg_stub_fill_rect.restype       = None

lib.pg_stub_stroke_rect.argtypes    = [TG_Region, Color, ctypes.c_float]
lib.pg_stub_stroke_rect.restype     = None

lib.pg_stub_draw_text.argtypes      = [TG_Offset, ctypes.c_char_p, TextStyle]
lib.pg_stub_draw_text.restype       = None

lib.pg_stub_clip_push.argtypes      = [TG_Region]
lib.pg_stub_clip_push.restype       = None

lib.pg_stub_clip_pop.argtypes       = []
lib.pg_stub_clip_pop.restype        = None

lib.pg_stub_translate_push.argtypes = [TG_Offset]
lib.pg_stub_translate_push.restype  = None

lib.pg_stub_translate_pop.argtypes  = []
lib.pg_stub_translate_pop.restype   = None

# ---------------------------------------------------------------------------
# DrawCommand — tagged union para recuperar comandos del stub
# ---------------------------------------------------------------------------

class FillRectData(ctypes.Structure):
    _fields_ = [("region", TG_Region), ("color", Color)]

class StrokeRectData(ctypes.Structure):
    _fields_ = [("region", TG_Region), ("color", Color), ("stroke_width", ctypes.c_float)]

class DrawTextData(ctypes.Structure):
    _fields_ = [("pos", TG_Offset), ("text", ctypes.c_char * 256), ("style", TextStyle)]

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

lib.pg_stub_get.argtypes = [ctypes.c_int]
lib.pg_stub_get.restype  = DrawCommand
