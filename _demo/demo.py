#!/usr/bin/env python3
"""
PlagueTUI — Demo
================
Muestra un layout dock con Header, Footer, Sidebar y panel central.
Usa plague-app.dll que orquesta layout + CSS + terminal + events.

Presiona 'q' para salir.
"""

import ctypes
import sys
import os
from pathlib import Path

# ---------------------------------------------------------------------------
# Cargar DLLs
# ---------------------------------------------------------------------------

BASE = Path(__file__).parent.parent

def _lib_path(module):
    name = module.replace("-", "_")
    if sys.platform == "win32":
        return BASE / module / "bin" / f"{name}.dll"
    elif sys.platform == "darwin":
        return BASE / module / "bin" / f"lib{name}.dylib"
    return BASE / module / "bin" / f"lib{name}.so"

# En Windows, registrar todos los directorios bin\ como fuentes de DLLs
if sys.platform == "win32":
    for mod in ("plague-terminal", "plague-layout", "plague-css",
                "plague-events", "plague-app"):
        os.add_dll_directory(str(BASE / mod / "bin"))

# Cargar dependencias primero (ya están en memoria cuando plague-app las necesita)
def _load(module):
    path = _lib_path(module)
    if not path.exists():
        raise FileNotFoundError(
            f"DLL no encontrada: {path}\n"
            f"  → Corre build.bat en {BASE / module}"
        )
    try:
        return ctypes.CDLL(str(path))
    except OSError as e:
        raise OSError(f"No se pudo cargar {path.name}: {e}") from e

_terminal = _load("plague-terminal")
_layout   = _load("plague-layout")
_css      = _load("plague-css")
_events   = _load("plague-events")
app       = _load("plague-app")

# ---------------------------------------------------------------------------
# Tipos ctypes
# ---------------------------------------------------------------------------

class TG_Spacing(ctypes.Structure):
    _fields_ = [("top", ctypes.c_int), ("right", ctypes.c_int),
                ("bottom", ctypes.c_int), ("left", ctypes.c_int)]

class PL_SizeValue(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int), ("value", ctypes.c_int)]

class TG_Region(ctypes.Structure):
    _fields_ = [("x", ctypes.c_int), ("y", ctypes.c_int),
                ("width", ctypes.c_int), ("height", ctypes.c_int)]

PL_SIZE_FIXED    = 0
PL_SIZE_FRACTION = 1
PL_LAYOUT_DOCK   = 2
PL_DOCK_TOP      = 0
PL_DOCK_BOTTOM   = 1
PL_DOCK_LEFT     = 2
PL_DOCK_RIGHT    = 3
PL_DOCK_CENTER   = 4
PA_NO_PARENT     = -1

# Teclas especiales (PE_KEY_*)
KEY_UP    = 0x0100
KEY_DOWN  = 0x0101
KEY_ENTER = 0x000D

def fixed(v):   return PL_SizeValue(PL_SIZE_FIXED, v)
def fr(v=1):    return PL_SizeValue(PL_SIZE_FRACTION, v)
def pad(n):     return TG_Spacing(n, n, n, n)

# ---------------------------------------------------------------------------
# Firmas
# ---------------------------------------------------------------------------

app.pa_init.restype              = ctypes.c_int
app.pa_shutdown.restype          = None
app.pa_get_cols.restype          = ctypes.c_int
app.pa_get_rows.restype          = ctypes.c_int
app.pa_css_load.argtypes         = [ctypes.c_char_p]
app.pa_css_load.restype          = ctypes.c_int
app.pa_widget_add.argtypes       = [ctypes.c_char_p, ctypes.c_char_p,
                                     ctypes.c_char_p, ctypes.c_int]
app.pa_widget_add.restype        = ctypes.c_int
app.pa_widget_set_text.argtypes  = [ctypes.c_int, ctypes.c_char_p]
app.pa_widget_set_size.argtypes  = [ctypes.c_int, PL_SizeValue, PL_SizeValue]
app.pa_widget_set_dock.argtypes  = [ctypes.c_int, ctypes.c_int]
app.pa_widget_set_layout.argtypes= [ctypes.c_int, ctypes.c_int]
app.pa_widget_set_padding.argtypes=[ctypes.c_int, TG_Spacing]
app.pa_render.restype            = None
app.pa_bind_key.argtypes         = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
app.pa_bind_key.restype          = ctypes.c_int
app.pa_wait_poll.restype         = ctypes.c_int
app.pa_quit.restype              = None

# ---------------------------------------------------------------------------
# TCSS
# ---------------------------------------------------------------------------

TCSS = b"""
Screen  { background: #0d1117; }

Header  {
    background: #1f3a5f;
    color: #e6edf3;
    border: solid #388bfd;
}

Footer  {
    background: #161b22;
    color: #8b949e;
}

Sidebar {
    background: #161b22;
    color: #c9d1d9;
    border: solid #30363d;
}

Main    {
    background: #0d1117;
    color: #e6edf3;
}
"""

# ---------------------------------------------------------------------------
# Contenido de cada sección del menú
# ---------------------------------------------------------------------------

MENU_ITEMS = ["Home", "Widgets", "Layout", "Events", "CSS", "About"]

MENU_CONTENT = {
    "Home": (
        "Bienvenido a PlagueTUI!\n\n"
        "Este demo integra:\n"
        "  plague-layout   - calcula regiones\n"
        "  plague-css      - aplica estilos\n"
        "  plague-terminal - dibuja en ANSI\n"
        "  plague-events   - gestiona teclas\n"
        "  plague-app      - orquesta todo"
    ),
    "Widgets": (
        "Widgets disponibles:\n\n"
        "  Label      - texto estático\n"
        "  Button     - acción al presionar\n"
        "  Header     - barra superior\n"
        "  Footer     - barra inferior\n"
        "  Sidebar    - panel lateral\n"
        "  [proximos] - Input, ListView..."
    ),
    "Layout": (
        "Motor de layout — plague-layout:\n\n"
        "  VERTICAL    - apila hijos en Y\n"
        "  HORIZONTAL  - apila hijos en X\n"
        "  DOCK        - ancla en bordes\n"
        "  GRID        - cuadrícula N×M\n\n"
        "  Soporta fr() y fixed() igual\n"
        "  que CSS grid/flexbox."
    ),
    "Events": (
        "Sistema de eventos — plague-events:\n\n"
        "  Cola FIFO de 256 eventos\n"
        "  Árbol de nodos con bubbling\n"
        "  Key bindings por nodo\n"
        "  Timers con repeat\n\n"
        "  Ctrl+Q = panic exit (siempre)"
    ),
    "CSS": (
        "Estilos — plague-css:\n\n"
        "  Subconjunto de TCSS (Textual)\n"
        "  Selectores: Type .class #id\n"
        "  Pseudo-clases: :hover :focus\n"
        "  Propiedades: color, background,\n"
        "    border, padding, margin,\n"
        "    width, height, dock, display"
    ),
    "About": (
        "PlagueTUI v0.1\n\n"
        "Reimplementación de Textual\n"
        "en C + Python via DLLs.\n\n"
        "Módulos:\n"
        "  plague-geometry\n"
        "  plague-drawcontext\n"
        "  plague-compositor\n"
        "  plague-terminal\n"
        "  plague-layout\n"
        "  plague-css\n"
        "  plague-events\n"
        "  plague-app"
    ),
}

def sidebar_text(selected):
    lines = []
    for i, item in enumerate(MENU_ITEMS):
        prefix = "> " if i == selected else "  "
        lines.append(f"{prefix}{item}")
    return "\n".join(lines).encode()

# ---------------------------------------------------------------------------
# Construcción del layout
# ---------------------------------------------------------------------------

def build_ui():
    root = app.pa_widget_add(b"Screen", b"screen", b"", PA_NO_PARENT)
    app.pa_widget_set_layout(root, PL_LAYOUT_DOCK)

    # Header
    header = app.pa_widget_add(b"Header", b"header", b"", root)
    app.pa_widget_set_dock(header, PL_DOCK_TOP)
    app.pa_widget_set_size(header, fr(1), fixed(3))
    app.pa_widget_set_padding(header, pad(1))
    app.pa_widget_set_text(header, b"PlagueTUI  v0.1")

    # Footer
    footer = app.pa_widget_add(b"Footer", b"footer", b"", root)
    app.pa_widget_set_dock(footer, PL_DOCK_BOTTOM)
    app.pa_widget_set_size(footer, fr(1), fixed(1))
    app.pa_widget_set_text(footer,
        " ↑↓ navegar   Enter seleccionar   q salir   Ctrl+Q forzar salida".encode())

    # Sidebar
    sidebar = app.pa_widget_add(b"Sidebar", b"sidebar", b"", root)
    app.pa_widget_set_dock(sidebar, PL_DOCK_LEFT)
    app.pa_widget_set_size(sidebar, fixed(22), fr(1))
    app.pa_widget_set_padding(sidebar, pad(1))

    # Panel central
    main_panel = app.pa_widget_add(b"Main", b"main", b"", root)
    app.pa_widget_set_dock(main_panel, PL_DOCK_CENTER)
    app.pa_widget_set_padding(main_panel, pad(1))

    return root, sidebar, main_panel

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    if not app.pa_init():
        err = ctypes.WinError() if sys.platform == "win32" else OSError("pt_init falló")
        print(f"Error al inicializar la terminal: {err}", file=sys.stderr)
        print("  ¿Estás ejecutando desde una consola real (cmd/PowerShell/WT)?",
              file=sys.stderr)
        sys.exit(1)

    try:
        app.pa_css_load(TCSS)
        root, sidebar, main_panel = build_ui()

        # Bindings
        bid_up    = app.pa_bind_key(root, KEY_UP,    0)
        bid_down  = app.pa_bind_key(root, KEY_DOWN,  0)
        bid_enter = app.pa_bind_key(root, KEY_ENTER, 0)
        bid_quit  = app.pa_bind_key(root, ord('q'),  0)
        bid_quit2 = app.pa_bind_key(root, ord('Q'),  0)

        selected = 0

        def refresh():
            app.pa_widget_set_text(sidebar, sidebar_text(selected))
            app.pa_widget_set_text(main_panel,
                MENU_CONTENT[MENU_ITEMS[selected]].encode())
            app.pa_render()

        refresh()

        while True:
            result = app.pa_wait_poll()
            if result < 0 or result in (bid_quit, bid_quit2):
                break
            elif result == bid_up:
                selected = (selected - 1) % len(MENU_ITEMS)
                refresh()
            elif result == bid_down:
                selected = (selected + 1) % len(MENU_ITEMS)
                refresh()
            elif result == bid_enter:
                refresh()  # podría abrir subpanel en el futuro

    finally:
        app.pa_shutdown()


if __name__ == "__main__":
    main()
