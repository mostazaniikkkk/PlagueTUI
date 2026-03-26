"""
Loads plague_widgets.dll for tests.
Dependent DLLs must be in bin/ (copied by build.bat).
"""

import ctypes
import sys
import os
from pathlib import Path

root    = Path(__file__).parent.parent
bin_dir = root / "bin"

if sys.platform == "win32":
    os.add_dll_directory(str(bin_dir))

if sys.platform == "win32":
    lib_path = bin_dir / "plague_widgets.dll"
elif sys.platform == "darwin":
    lib_path = bin_dir / "libplague_widgets.dylib"
else:
    lib_path = bin_dir / "libplague_widgets.so"

lib = ctypes.CDLL(str(lib_path))

# ---------------------------------------------------------------------------
# Re-use plague-app types (plague_app.dll must be in bin\ too)
# ---------------------------------------------------------------------------

app_lib_path = bin_dir / ("plague_app.dll" if sys.platform == "win32" else "libplague_app.so")
app = ctypes.CDLL(str(app_lib_path))

class TG_Region(ctypes.Structure):
    _fields_ = [
        ("x",      ctypes.c_int),
        ("y",      ctypes.c_int),
        ("width",  ctypes.c_int),
        ("height", ctypes.c_int),
    ]

class TG_Spacing(ctypes.Structure):
    _fields_ = [
        ("top",    ctypes.c_int),
        ("right",  ctypes.c_int),
        ("bottom", ctypes.c_int),
        ("left",   ctypes.c_int),
    ]

PL_SIZE_FIXED    = 0
PL_SIZE_FRACTION = 1

class PL_SizeValue(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int), ("value", ctypes.c_int)]

PA_NO_PARENT = -1

# ---------------------------------------------------------------------------
# plague-app signatures (subset used by tests)
# ---------------------------------------------------------------------------

app.pa_init_headless.argtypes = [ctypes.c_int, ctypes.c_int]
app.pa_init_headless.restype  = ctypes.c_int

app.pa_shutdown.argtypes = []
app.pa_shutdown.restype  = None

app.pa_render.argtypes = []
app.pa_render.restype  = None

app.pa_widget_region.argtypes = [ctypes.c_int]
app.pa_widget_region.restype  = TG_Region

app.pa_widget_set_size.argtypes = [ctypes.c_int, PL_SizeValue, PL_SizeValue]
app.pa_widget_set_size.restype  = None

app.pa_widget_add.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
app.pa_widget_add.restype  = ctypes.c_int

# ---------------------------------------------------------------------------
# plague-widgets signatures
# ---------------------------------------------------------------------------

lib.pw_init.argtypes     = []
lib.pw_init.restype      = None

lib.pw_shutdown.argtypes = []
lib.pw_shutdown.restype  = None

# Static / Label
lib.pw_static_create.argtypes = [ctypes.c_char_p, ctypes.c_int]
lib.pw_static_create.restype  = ctypes.c_int

lib.pw_label_create.argtypes  = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
lib.pw_label_create.restype   = ctypes.c_int

lib.pw_label_set_text.argtypes = [ctypes.c_int, ctypes.c_char_p]
lib.pw_label_set_text.restype  = None

# Rule
lib.pw_rule_create.argtypes   = [ctypes.c_char_p, ctypes.c_int]
lib.pw_rule_create.restype    = ctypes.c_int

lib.pw_rule_set_char.argtypes = [ctypes.c_int, ctypes.c_char_p]
lib.pw_rule_set_char.restype  = None

# Placeholder
lib.pw_placeholder_create.argtypes       = [ctypes.c_char_p, ctypes.c_int]
lib.pw_placeholder_create.restype        = ctypes.c_int

lib.pw_placeholder_set_variant.argtypes  = [ctypes.c_int, ctypes.c_int]
lib.pw_placeholder_set_variant.restype   = None

# ProgressBar
lib.pw_progressbar_create.argtypes      = [ctypes.c_float, ctypes.c_int]
lib.pw_progressbar_create.restype       = ctypes.c_int

lib.pw_progressbar_set_progress.argtypes = [ctypes.c_int, ctypes.c_float]
lib.pw_progressbar_set_progress.restype  = None

lib.pw_progressbar_update.argtypes      = [ctypes.c_int]
lib.pw_progressbar_update.restype       = None

lib.pw_progressbar_get_progress.argtypes = [ctypes.c_int]
lib.pw_progressbar_get_progress.restype  = ctypes.c_float

lib.pw_progressbar_get_total.argtypes    = [ctypes.c_int]
lib.pw_progressbar_get_total.restype     = ctypes.c_float

# Sparkline
lib.pw_sparkline_create.argtypes   = [ctypes.c_int]
lib.pw_sparkline_create.restype    = ctypes.c_int

lib.pw_sparkline_set_data.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_float), ctypes.c_int]
lib.pw_sparkline_set_data.restype  = None

# Digits
lib.pw_digits_create.argtypes   = [ctypes.c_char_p, ctypes.c_int]
lib.pw_digits_create.restype    = ctypes.c_int

lib.pw_digits_set_text.argtypes = [ctypes.c_int, ctypes.c_char_p]
lib.pw_digits_set_text.restype  = None

# LoadingIndicator
lib.pw_loading_create.argtypes = [ctypes.c_int]
lib.pw_loading_create.restype  = ctypes.c_int

lib.pw_loading_tick.argtypes   = [ctypes.c_int, ctypes.c_int]
lib.pw_loading_tick.restype    = None

lib.pw_loading_start.argtypes  = [ctypes.c_int]
lib.pw_loading_start.restype   = None

lib.pw_loading_stop.argtypes   = [ctypes.c_int]
lib.pw_loading_stop.restype    = None

# RadioButton
lib.pw_radio_create.argtypes      = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int]
lib.pw_radio_create.restype       = ctypes.c_int

lib.pw_radio_set_checked.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pw_radio_set_checked.restype  = None

lib.pw_radio_get_checked.argtypes = [ctypes.c_int]
lib.pw_radio_get_checked.restype  = ctypes.c_int

# RadioSet
lib.pw_radioset_create.argtypes      = [ctypes.c_int]
lib.pw_radioset_create.restype       = ctypes.c_int

lib.pw_radioset_add.argtypes         = [ctypes.c_int, ctypes.c_char_p]
lib.pw_radioset_add.restype          = ctypes.c_int

lib.pw_radioset_select.argtypes      = [ctypes.c_int, ctypes.c_int]
lib.pw_radioset_select.restype       = None

lib.pw_radioset_get_selected.argtypes = [ctypes.c_int]
lib.pw_radioset_get_selected.restype  = ctypes.c_int

lib.pw_radioset_count.argtypes       = [ctypes.c_int]
lib.pw_radioset_count.restype        = ctypes.c_int

# OptionList
lib.pw_optionlist_create.argtypes      = [ctypes.c_int]
lib.pw_optionlist_create.restype       = ctypes.c_int

lib.pw_optionlist_add_option.argtypes  = [ctypes.c_int, ctypes.c_char_p]
lib.pw_optionlist_add_option.restype   = ctypes.c_int

lib.pw_optionlist_clear.argtypes       = [ctypes.c_int]
lib.pw_optionlist_clear.restype        = None

lib.pw_optionlist_set_cursor.argtypes  = [ctypes.c_int, ctypes.c_int]
lib.pw_optionlist_set_cursor.restype   = None

lib.pw_optionlist_get_cursor.argtypes  = [ctypes.c_int]
lib.pw_optionlist_get_cursor.restype   = ctypes.c_int

lib.pw_optionlist_cursor_next.argtypes = [ctypes.c_int]
lib.pw_optionlist_cursor_next.restype  = None

lib.pw_optionlist_cursor_prev.argtypes = [ctypes.c_int]
lib.pw_optionlist_cursor_prev.restype  = None

lib.pw_optionlist_count.argtypes       = [ctypes.c_int]
lib.pw_optionlist_count.restype        = ctypes.c_int

# ListView
lib.pw_listview_create.argtypes      = [ctypes.c_int]
lib.pw_listview_create.restype       = ctypes.c_int

lib.pw_listview_add_item.argtypes    = [ctypes.c_int, ctypes.c_char_p]
lib.pw_listview_add_item.restype     = ctypes.c_int

lib.pw_listview_clear.argtypes       = [ctypes.c_int]
lib.pw_listview_clear.restype        = None

lib.pw_listview_set_cursor.argtypes  = [ctypes.c_int, ctypes.c_int]
lib.pw_listview_set_cursor.restype   = None

lib.pw_listview_get_cursor.argtypes  = [ctypes.c_int]
lib.pw_listview_get_cursor.restype   = ctypes.c_int

lib.pw_listview_cursor_next.argtypes = [ctypes.c_int]
lib.pw_listview_cursor_next.restype  = None

lib.pw_listview_cursor_prev.argtypes = [ctypes.c_int]
lib.pw_listview_cursor_prev.restype  = None

lib.pw_listview_count.argtypes       = [ctypes.c_int]
lib.pw_listview_count.restype        = ctypes.c_int

# SelectionList
lib.pw_selectionlist_create.argtypes           = [ctypes.c_int]
lib.pw_selectionlist_create.restype            = ctypes.c_int

lib.pw_selectionlist_add_option.argtypes       = [ctypes.c_int, ctypes.c_char_p, ctypes.c_int]
lib.pw_selectionlist_add_option.restype        = ctypes.c_int

lib.pw_selectionlist_clear.argtypes            = [ctypes.c_int]
lib.pw_selectionlist_clear.restype             = None

lib.pw_selectionlist_set_cursor.argtypes       = [ctypes.c_int, ctypes.c_int]
lib.pw_selectionlist_set_cursor.restype        = None

lib.pw_selectionlist_get_cursor.argtypes       = [ctypes.c_int]
lib.pw_selectionlist_get_cursor.restype        = ctypes.c_int

lib.pw_selectionlist_cursor_next.argtypes      = [ctypes.c_int]
lib.pw_selectionlist_cursor_next.restype       = None

lib.pw_selectionlist_cursor_prev.argtypes      = [ctypes.c_int]
lib.pw_selectionlist_cursor_prev.restype       = None

lib.pw_selectionlist_toggle_selection.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pw_selectionlist_toggle_selection.restype  = None

lib.pw_selectionlist_set_selected.argtypes     = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
lib.pw_selectionlist_set_selected.restype      = None

lib.pw_selectionlist_is_selected.argtypes      = [ctypes.c_int, ctypes.c_int]
lib.pw_selectionlist_is_selected.restype       = ctypes.c_int

lib.pw_selectionlist_count.argtypes            = [ctypes.c_int]
lib.pw_selectionlist_count.restype             = ctypes.c_int

# ToggleButton
lib.pw_toggle_create.argtypes      = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int]
lib.pw_toggle_create.restype       = ctypes.c_int

lib.pw_toggle_set_checked.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pw_toggle_set_checked.restype  = None

lib.pw_toggle_get_checked.argtypes = [ctypes.c_int]
lib.pw_toggle_get_checked.restype  = ctypes.c_int

lib.pw_toggle_set_label.argtypes   = [ctypes.c_int, ctypes.c_char_p]
lib.pw_toggle_set_label.restype    = None

lib.pw_toggle_toggle.argtypes      = [ctypes.c_int]
lib.pw_toggle_toggle.restype       = None

# Checkbox
lib.pw_checkbox_create.argtypes      = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int]
lib.pw_checkbox_create.restype       = ctypes.c_int

lib.pw_checkbox_set_checked.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pw_checkbox_set_checked.restype  = None

lib.pw_checkbox_get_checked.argtypes = [ctypes.c_int]
lib.pw_checkbox_get_checked.restype  = ctypes.c_int

lib.pw_checkbox_set_label.argtypes   = [ctypes.c_int, ctypes.c_char_p]
lib.pw_checkbox_set_label.restype    = None

lib.pw_checkbox_toggle.argtypes      = [ctypes.c_int]
lib.pw_checkbox_toggle.restype       = None

# Switch
lib.pw_switch_create.argtypes    = [ctypes.c_int, ctypes.c_int]
lib.pw_switch_create.restype     = ctypes.c_int

lib.pw_switch_set_active.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pw_switch_set_active.restype  = None

lib.pw_switch_get_active.argtypes = [ctypes.c_int]
lib.pw_switch_get_active.restype  = ctypes.c_int

lib.pw_switch_toggle.argtypes    = [ctypes.c_int]
lib.pw_switch_toggle.restype     = None

# Button
lib.pw_button_create.argtypes    = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
lib.pw_button_create.restype     = ctypes.c_int

lib.pw_button_set_label.argtypes = [ctypes.c_int, ctypes.c_char_p]
lib.pw_button_set_label.restype  = None

# pa_widget_set_visible (needed by switcher tests)
app.pa_widget_set_visible.argtypes = [ctypes.c_int, ctypes.c_int]
app.pa_widget_set_visible.restype  = None

# ContentSwitcher
lib.pw_switcher_create.argtypes = [ctypes.c_int]
lib.pw_switcher_create.restype  = ctypes.c_int

lib.pw_switcher_add.argtypes    = [ctypes.c_int, ctypes.c_int]
lib.pw_switcher_add.restype     = ctypes.c_int

lib.pw_switcher_show.argtypes   = [ctypes.c_int, ctypes.c_int]
lib.pw_switcher_show.restype    = None

lib.pw_switcher_active.argtypes = [ctypes.c_int]
lib.pw_switcher_active.restype  = ctypes.c_int

lib.pw_switcher_count.argtypes  = [ctypes.c_int]
lib.pw_switcher_count.restype   = ctypes.c_int

# Toast
lib.pw_toast_create.argtypes     = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int]
lib.pw_toast_create.restype      = ctypes.c_int

lib.pw_toast_show.argtypes       = [ctypes.c_int, ctypes.c_char_p, ctypes.c_int]
lib.pw_toast_show.restype        = None

lib.pw_toast_tick.argtypes       = [ctypes.c_int, ctypes.c_int]
lib.pw_toast_tick.restype        = None

lib.pw_toast_is_visible.argtypes = [ctypes.c_int]
lib.pw_toast_is_visible.restype  = ctypes.c_int

# Header
lib.pw_header_create.argtypes    = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
lib.pw_header_create.restype     = ctypes.c_int

lib.pw_header_set_title.argtypes = [ctypes.c_int, ctypes.c_char_p]
lib.pw_header_set_title.restype  = None

lib.pw_header_set_icon.argtypes  = [ctypes.c_int, ctypes.c_char_p]
lib.pw_header_set_icon.restype   = None

lib.pw_header_set_clock.argtypes = [ctypes.c_int, ctypes.c_int]
lib.pw_header_set_clock.restype  = None

lib.pw_header_tick.argtypes      = [ctypes.c_int]
lib.pw_header_tick.restype       = None

# Footer
lib.pw_footer_create.argtypes     = [ctypes.c_int]
lib.pw_footer_create.restype      = ctypes.c_int

lib.pw_footer_add_key.argtypes    = [ctypes.c_int, ctypes.c_char_p, ctypes.c_char_p]
lib.pw_footer_add_key.restype     = None

lib.pw_footer_clear_keys.argtypes = [ctypes.c_int]
lib.pw_footer_clear_keys.restype  = None

lib.pw_footer_refresh.argtypes    = [ctypes.c_int]
lib.pw_footer_refresh.restype     = None

# Default TCSS
lib.pw_default_tcss.argtypes = []
lib.pw_default_tcss.restype  = ctypes.c_char_p

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def fixed(v):
    return PL_SizeValue(PL_SIZE_FIXED, v)

def fr(v=1):
    return PL_SizeValue(PL_SIZE_FRACTION, v)
