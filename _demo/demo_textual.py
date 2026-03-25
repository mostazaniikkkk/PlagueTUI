#!/usr/bin/env python3
"""
Textual — Demo de referencia con Textual
==========================================
Réplica exacta del demo de Textual usando la librería Textual real.
Sirve como referencia visual para comparar precisión de estilos.

Ejecutar:
    .venv\\Scripts\\python demo_textual.py      (Windows)
    .venv/bin/python demo_textual.py           (Linux/macOS)
"""

from textual.app import App, ComposeResult
from textual.widgets import Header, Footer, Static
from textual.containers import Container
from textual import on
from textual.binding import Binding

# ---------------------------------------------------------------------------
# Contenido de cada sección del menú (idéntico al demo de Textual)
# ---------------------------------------------------------------------------

MENU_ITEMS = ["Home", "Widgets", "Layout", "Events", "CSS", "About"]

MENU_CONTENT = {
    "Home": (
        "Bienvenido a Textual!\n\n"
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
        "Textual v0.1\n\n"
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

# ---------------------------------------------------------------------------
# CSS — mismo esquema de colores GitHub dark que usa Textual
# ---------------------------------------------------------------------------

CSS = """
Screen {
    background: #0d1117;
}

#sidebar {
    width: 22;
    background: #161b22;
    color: #c9d1d9;
    border: solid #30363d;
    padding: 1;
    dock: left;
}

#main {
    background: #0d1117;
    color: #e6edf3;
    padding: 1;
}

Header {
    background: #1f3a5f;
    color: #e6edf3;
    border-bottom: solid #388bfd;
}

Footer {
    background: #161b22;
    color: #8b949e;
}

.menu-item {
    color: #c9d1d9;
}

.menu-item--selected {
    color: #e6edf3;
    text-style: bold;
}
"""

# ---------------------------------------------------------------------------
# Widgets
# ---------------------------------------------------------------------------

class Sidebar(Container):
    """Panel lateral con lista de menú navegable."""

    def __init__(self):
        super().__init__(id="sidebar")
        self._selected = 0

    def compose(self) -> ComposeResult:
        for i, item in enumerate(MENU_ITEMS):
            prefix = "> " if i == 0 else "  "
            classes = "menu-item menu-item--selected" if i == 0 else "menu-item"
            yield Static(f"{prefix}{item}", id=f"item-{i}", classes=classes)

    def select(self, index: int) -> None:
        old = self._selected
        self._selected = index % len(MENU_ITEMS)

        old_widget = self.query_one(f"#item-{old}", Static)
        old_widget.update(f"  {MENU_ITEMS[old]}")
        old_widget.remove_class("menu-item--selected")

        new_widget = self.query_one(f"#item-{self._selected}", Static)
        new_widget.update(f"> {MENU_ITEMS[self._selected]}")
        new_widget.add_class("menu-item--selected")

    @property
    def selected(self) -> int:
        return self._selected


class MainPanel(Static):
    """Panel central con contenido de la sección seleccionada."""

    def __init__(self):
        super().__init__(MENU_CONTENT["Home"], id="main")

    def show(self, section: str) -> None:
        self.update(MENU_CONTENT[section])


# ---------------------------------------------------------------------------
# App
# ---------------------------------------------------------------------------

class TextualDemo(App):
    """Demo de Textual replicado con Textual."""

    CSS = CSS
    TITLE = "Textual  v0.1"

    BINDINGS = [
        Binding("q", "quit", "Salir"),
        Binding("Q", "quit", "Salir"),
        Binding("up",    "move_up",   "Arriba", show=False),
        Binding("down",  "move_down", "Abajo",  show=False),
        Binding("enter", "select",    "Seleccionar", show=False),
    ]

    def compose(self) -> ComposeResult:
        yield Header()
        yield Sidebar()
        yield MainPanel()
        yield Footer()

    def action_move_up(self) -> None:
        sidebar = self.query_one(Sidebar)
        sidebar.select(sidebar.selected - 1)
        self.query_one(MainPanel).show(MENU_ITEMS[sidebar.selected])

    def action_move_down(self) -> None:
        sidebar = self.query_one(Sidebar)
        sidebar.select(sidebar.selected + 1)
        self.query_one(MainPanel).show(MENU_ITEMS[sidebar.selected])

    def action_select(self) -> None:
        # En Textual también era un no-op por ahora
        pass


if __name__ == "__main__":
    TextualDemo().run()
