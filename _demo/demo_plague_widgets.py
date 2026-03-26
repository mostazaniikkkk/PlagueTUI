#!/usr/bin/env python3
"""
PlagueTUI — Widget Debug Demo
Exercises all Phase 2-5 widgets in a single interactive TUI.

  Tab / Shift+Tab   cycle focus between interactive widgets
  j / k             navigate cursor in focused list (OptionList, ListView, SelectionList)
  Space             toggle focused Checkbox / Switch / ToggleButton
                    or toggle SelectionList item under cursor
  p                 add +10% to ProgressBar
  1 / 2 / 3         select RadioSet option
  q                 quit

Requires:
  plague-app/build.bat   → plague-app/bin/plague_app.dll
  plague-widgets/build.bat → plague-widgets/bin/plague_widgets.dll
"""

import ctypes, os, sys, time
from pathlib import Path

# ---------------------------------------------------------------------------
# DLL loading
# ---------------------------------------------------------------------------
ROOT    = Path(__file__).resolve().parent.parent
BIN_APP = ROOT / "plague-app"     / "bin"
BIN_W   = ROOT / "plague-widgets" / "bin"

if sys.platform != "win32":
    print("Demo currently targets Windows (DLL). Adapt paths for your platform.")
    sys.exit(1)

os.add_dll_directory(str(BIN_APP))
os.add_dll_directory(str(BIN_W))

app = ctypes.CDLL(str(BIN_APP / "plague_app.dll"))
pw  = ctypes.CDLL(str(BIN_W  / "plague_widgets.dll"))

# ---------------------------------------------------------------------------
# C types
# ---------------------------------------------------------------------------
class TG_Spacing(ctypes.Structure):
    _fields_ = [("top",ctypes.c_int),("right",ctypes.c_int),
                ("bottom",ctypes.c_int),("left",ctypes.c_int)]

class PL_SizeValue(ctypes.Structure):
    _fields_ = [("type",ctypes.c_int),("value",ctypes.c_int)]

class TG_Region(ctypes.Structure):
    _fields_ = [("x",ctypes.c_int),("y",ctypes.c_int),
                ("width",ctypes.c_int),("height",ctypes.c_int)]

PL_SIZE_FIXED    = 0
PL_SIZE_FRACTION = 1
PL_LAYOUT_VERTICAL   = 0
PL_LAYOUT_HORIZONTAL = 1
PL_LAYOUT_DOCK       = 2
PL_DOCK_TOP    = 0
PL_DOCK_BOTTOM = 1
PL_DOCK_CENTER = 4
PA_NO_PARENT   = -1
PT_MOD_NONE    = 0

def fixed(v): return PL_SizeValue(PL_SIZE_FIXED,    v)
def fr(v=1):  return PL_SizeValue(PL_SIZE_FRACTION, v)
def sp(t=0, r=0, b=0, l=0): return TG_Spacing(t, r, b, l)

# ---------------------------------------------------------------------------
# Bind function signatures
# ---------------------------------------------------------------------------
def _sig(fn, restype, *argtypes):
    fn.restype  = restype
    fn.argtypes = list(argtypes)

C  = ctypes.c_int
CP = ctypes.c_char_p
CF = ctypes.c_float
SV = PL_SizeValue
SP = TG_Spacing

# plague-app
_sig(app.pa_init,               C)
_sig(app.pa_shutdown,           None)
_sig(app.pa_get_cols,           C)
_sig(app.pa_get_rows,           C)
_sig(app.pa_css_load,           C,    CP)
_sig(app.pa_widget_add,         C,    CP, CP, CP, C)
_sig(app.pa_widget_set_text,    None, C,  CP)
_sig(app.pa_widget_set_size,    None, C,  SV, SV)
_sig(app.pa_widget_set_dock,    None, C,  C)
_sig(app.pa_widget_set_layout,  None, C,  C)
_sig(app.pa_widget_set_padding, None, C,  SP)
_sig(app.pa_widget_set_focusable, None, C, C)
_sig(app.pa_widget_region,      TG_Region, C)
_sig(app.pa_render,             None)
_sig(app.pa_focus_get,          C)
_sig(app.pa_focus_next,         None)
_sig(app.pa_focus_prev,         None)
_sig(app.pa_bind_key,           C,    C, C, C)
_sig(app.pa_bind_click,         C,    C)
_sig(app.pa_mouse_pos,          None, ctypes.POINTER(C), ctypes.POINTER(C))
_sig(app.pa_poll,               C)
_sig(app.pa_quit,               None)
_sig(app.pa_timer_create,       C,    C, C)
_sig(app.pa_tick_timers,        C,    C)

# plague-widgets — lifecycle
_sig(pw.pw_init,              None)
_sig(pw.pw_shutdown,          None)
_sig(pw.pw_default_tcss,      CP)

# Display
_sig(pw.pw_static_create,     C,    CP, C)
_sig(pw.pw_label_create,      C,    CP, CP, C)
_sig(pw.pw_label_set_text,    None, C,  CP)
_sig(pw.pw_rule_create,       C,    CP, C)
_sig(pw.pw_placeholder_create,C,    CP, C)
_sig(pw.pw_placeholder_set_variant, None, C, C)

_sig(pw.pw_progressbar_create,       C,    CF, C)
_sig(pw.pw_progressbar_set_progress, None, C,  CF)
_sig(pw.pw_progressbar_get_progress, CF,   C)
_sig(pw.pw_progressbar_update,       None, C)

_sig(pw.pw_sparkline_create,   C,    C)
_sig(pw.pw_sparkline_set_data, None, C,  ctypes.POINTER(CF), C)

_sig(pw.pw_digits_create,   C,    CP, C)
_sig(pw.pw_digits_set_text, None, C,  CP)

_sig(pw.pw_loading_create, C,    C)
_sig(pw.pw_loading_tick,   None, C, C)
_sig(pw.pw_loading_start,  None, C)

# System
_sig(pw.pw_header_create,    C,    CP, CP, C)
_sig(pw.pw_header_set_clock, None, C,  C)
_sig(pw.pw_header_tick,      None, C)
_sig(pw.pw_footer_create,    C,    C)
_sig(pw.pw_footer_add_key,   None, C,  CP, CP)

# Phase 4 — interactive
_sig(pw.pw_toggle_create,      C,    CP, C, C)
_sig(pw.pw_toggle_toggle,      None, C)
_sig(pw.pw_toggle_get_checked, C,    C)

_sig(pw.pw_checkbox_create,    C,    CP, C, C)
_sig(pw.pw_checkbox_toggle,    None, C)

_sig(pw.pw_switch_create,      C,    C, C)
_sig(pw.pw_switch_toggle,      None, C)
_sig(pw.pw_switch_get_active,  C,    C)

_sig(pw.pw_button_create,    C,    CP, CP, C)
_sig(pw.pw_button_set_label, None, C,  CP)

# Phase 5 — lists
_sig(pw.pw_radioset_create,       C,    C)
_sig(pw.pw_radioset_add,          C,    C, CP)
_sig(pw.pw_radioset_select,       None, C, C)
_sig(pw.pw_radioset_get_selected, C,    C)

_sig(pw.pw_optionlist_create,      C,    C)
_sig(pw.pw_optionlist_add_option,  C,    C, CP)
_sig(pw.pw_optionlist_cursor_next, None, C)
_sig(pw.pw_optionlist_cursor_prev, None, C)
_sig(pw.pw_optionlist_set_cursor,  None, C, C)
_sig(pw.pw_optionlist_get_cursor,  C,    C)

_sig(pw.pw_listview_create,      C,    C)
_sig(pw.pw_listview_add_item,    C,    C, CP)
_sig(pw.pw_listview_cursor_next, None, C)
_sig(pw.pw_listview_cursor_prev, None, C)
_sig(pw.pw_listview_set_cursor,  None, C, C)
_sig(pw.pw_listview_get_cursor,  C,    C)

_sig(pw.pw_selectionlist_create,           C,    C)
_sig(pw.pw_selectionlist_add_option,       C,    C, CP, C)
_sig(pw.pw_selectionlist_cursor_next,      None, C)
_sig(pw.pw_selectionlist_cursor_prev,      None, C)
_sig(pw.pw_selectionlist_set_cursor,       None, C, C)
_sig(pw.pw_selectionlist_get_cursor,       C,    C)
_sig(pw.pw_selectionlist_toggle_selection, None, C, C)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def mouse_pos():
    mx, my = C(0), C(0)
    app.pa_mouse_pos(ctypes.byref(mx), ctypes.byref(my))
    return mx.value, my.value

def click_row(wid):
    """Row index within widget that was just clicked (0-based)."""
    _, my = mouse_pos()
    reg = app.pa_widget_region(wid)
    return max(0, my - reg.y)

# ---------------------------------------------------------------------------
# Init
# ---------------------------------------------------------------------------
app.pa_init()
pw.pw_init()
app.pa_css_load(pw.pw_default_tcss())

# ---------------------------------------------------------------------------
# Widget tree
# ---------------------------------------------------------------------------

# Root — full screen, DOCK layout
screen = app.pa_widget_add(b"Screen", b"screen", b"", PA_NO_PARENT)
app.pa_widget_set_layout(screen, PL_LAYOUT_DOCK)
app.pa_widget_set_size(screen, fr(), fr())

# Header (auto-docks TOP)
header = pw.pw_header_create(b"PlagueTUI -- Widget Debug Demo", b"\xe2\x96\xb6", screen)
pw.pw_header_set_clock(header, 1)

# Footer (auto-docks BOTTOM)
footer = pw.pw_footer_create(screen)
pw.pw_footer_add_key(footer, b"q",     b"Quit")
pw.pw_footer_add_key(footer, b"Tab",   b"Focus")
pw.pw_footer_add_key(footer, b"j/k",   b"Navigate")
pw.pw_footer_add_key(footer, b"Space", b"Toggle")
pw.pw_footer_add_key(footer, b"p",     b"+Progress")
pw.pw_footer_add_key(footer, b"1-3",   b"Radio")

# Main content (CENTER dock, split horizontally)
main = app.pa_widget_add(b"container", b"main", b"", screen)
app.pa_widget_set_dock(main, PL_DOCK_CENTER)
app.pa_widget_set_layout(main, PL_LAYOUT_HORIZONTAL)
app.pa_widget_set_size(main, fr(), fr())
app.pa_widget_set_padding(main, sp(1, 2, 0, 2))

# ── LEFT PANEL — Display widgets ──────────────────────────────────────────
left = app.pa_widget_add(b"container", b"left", b"", main)
app.pa_widget_set_size(left, fr(), fr())
app.pa_widget_set_layout(left, PL_LAYOUT_VERTICAL)

pw.pw_label_create(b"[ Display Widgets ]", b"primary", left)
pw.pw_rule_create(None, left)

# Static / Labels
pw.pw_static_create(b"Static widget", left)
pw.pw_label_create(b"primary  ", b"primary", left)
pw.pw_label_create(b"success  ", b"success", left)
pw.pw_label_create(b"warning  ", b"warning", left)
pw.pw_label_create(b"error    ", b"error",   left)

# ProgressBar
pw.pw_label_create(b"ProgressBar [press p]:", b"secondary", left)
pb = pw.pw_progressbar_create(ctypes.c_float(100.0), left)
app.pa_widget_set_size(pb, fr(), fixed(1))
pw.pw_progressbar_set_progress(pb, ctypes.c_float(40.0))
progress_val = [40.0]

# Sparkline
pw.pw_label_create(b"Sparkline:", b"secondary", left)
spark = pw.pw_sparkline_create(left)
app.pa_widget_set_size(spark, fr(), fixed(1))
_sd = (ctypes.c_float * 16)(1,3,5,2,8,6,9,4,7,3,6,8,2,5,7,4)
pw.pw_sparkline_set_data(spark, _sd, 16)

# Digits (counts seconds, animated)
pw.pw_label_create(b"Digits (live):", b"secondary", left)
digits_w = pw.pw_digits_create(b"00:00", left)
app.pa_widget_set_size(digits_w, fr(), fixed(3))

# Loading indicator
pw.pw_label_create(b"LoadingIndicator:", b"secondary", left)
loading_row = app.pa_widget_add(b"container", b"lr", b"", left)
app.pa_widget_set_layout(loading_row, PL_LAYOUT_HORIZONTAL)
app.pa_widget_set_size(loading_row, fr(), fixed(1))
loading_w = pw.pw_loading_create(loading_row)
app.pa_widget_set_size(loading_w, fixed(2), fixed(1))
pw.pw_loading_start(loading_w)
pw.pw_static_create(b"  running...", loading_row)

# Placeholder
ph = pw.pw_placeholder_create(b"Placeholder", left)
app.pa_widget_set_size(ph, fr(), fixed(2))

# ListView
pw.pw_rule_create("\u2501".encode(), left)  # ━ heavy rule
pw.pw_label_create(b"ListView [focus + j/k]:", b"secondary", left)
lv = pw.pw_listview_create(left)
app.pa_widget_set_size(lv, fr(), fixed(4))
for fname in [b"main.py", b"app.c", b"widgets.h", b"render.c"]:
    pw.pw_listview_add_item(lv, fname)

# ── RIGHT PANEL — Interactive widgets ─────────────────────────────────────
right = app.pa_widget_add(b"container", b"right", b"", main)
app.pa_widget_set_size(right, fr(), fr())
app.pa_widget_set_layout(right, PL_LAYOUT_VERTICAL)

pw.pw_label_create(b"[ Interactive Widgets ]", b"primary", right)
pw.pw_rule_create(None, right)

# Toggle / Checkbox
toggle_w = pw.pw_toggle_create(b"Enable notifications", 0, right)
check_w  = pw.pw_checkbox_create(b"Accept terms", 0, right)

# Switch inline
sw_row = app.pa_widget_add(b"container", b"swr", b"", right)
app.pa_widget_set_layout(sw_row, PL_LAYOUT_HORIZONTAL)
app.pa_widget_set_size(sw_row, fr(), fixed(1))
lbl_dm = pw.pw_static_create(b"Dark mode: ", sw_row)
app.pa_widget_set_size(lbl_dm, fixed(11), fixed(1))
switch_w = pw.pw_switch_create(1, sw_row)

# Buttons + status label
pw.pw_rule_create(None, right)
pw.pw_label_create(b"Buttons:", b"secondary", right)
btn_row = app.pa_widget_add(b"container", b"brow", b"", right)
app.pa_widget_set_layout(btn_row, PL_LAYOUT_HORIZONTAL)
app.pa_widget_set_size(btn_row, fr(), fixed(1))
btn_ok     = pw.pw_button_create(b" OK ",     b"success", btn_row)
btn_cancel = pw.pw_button_create(b" Cancel ", b"error",   btn_row)
btn_info   = pw.pw_button_create(b" Info ",   b"primary", btn_row)
status_lbl = pw.pw_label_create(b"", b"secondary", right)

# RadioSet
pw.pw_rule_create(None, right)
pw.pw_label_create(b"RadioSet [1/2/3]:", b"secondary", right)
rs = pw.pw_radioset_create(right)
app.pa_widget_set_size(rs, fr(), fixed(3))
rs_opts = [
    pw.pw_radioset_add(rs, b"Low priority"),
    pw.pw_radioset_add(rs, b"Medium priority"),
    pw.pw_radioset_add(rs, b"High priority"),
]
pw.pw_radioset_select(rs, 1)

# OptionList
pw.pw_rule_create(None, right)
pw.pw_label_create(b"OptionList [focus + j/k]:", b"secondary", right)
ol = pw.pw_optionlist_create(right)
app.pa_widget_set_size(ol, fr(), fixed(4))
for lang in [b"Python", b"C / C++", b"Rust", b"Go", b"JavaScript", b"Zig"]:
    pw.pw_optionlist_add_option(ol, lang)

# SelectionList
pw.pw_rule_create(None, right)
pw.pw_label_create(b"SelectionList [focus + j/k + Space]:", b"secondary", right)
sl = pw.pw_selectionlist_create(right)
app.pa_widget_set_size(sl, fr(), fixed(4))
pw.pw_selectionlist_add_option(sl, b"Syntax highlighting", 1)
pw.pw_selectionlist_add_option(sl, b"Auto-complete",       0)
pw.pw_selectionlist_add_option(sl, b"Git integration",     1)
pw.pw_selectionlist_add_option(sl, b"Spell check",         0)
pw.pw_selectionlist_add_option(sl, b"Dark theme",          1)

# ---------------------------------------------------------------------------
# Bindings
# ---------------------------------------------------------------------------
BID_QUIT   = app.pa_bind_key(screen, ord('q'), PT_MOD_NONE)
BID_DOWN   = app.pa_bind_key(screen, ord('j'), PT_MOD_NONE)
BID_UP     = app.pa_bind_key(screen, ord('k'), PT_MOD_NONE)
BID_SPACE  = app.pa_bind_key(screen, ord(' '), PT_MOD_NONE)
BID_PROG   = app.pa_bind_key(screen, ord('p'), PT_MOD_NONE)
BID_R1     = app.pa_bind_key(screen, ord('1'), PT_MOD_NONE)
BID_R2     = app.pa_bind_key(screen, ord('2'), PT_MOD_NONE)
BID_R3     = app.pa_bind_key(screen, ord('3'), PT_MOD_NONE)
BID_TOGGLE   = app.pa_bind_click(toggle_w)
BID_CHECK    = app.pa_bind_click(check_w)
BID_SWITCH   = app.pa_bind_click(switch_w)
BID_OK       = app.pa_bind_click(btn_ok)
BID_CANCEL   = app.pa_bind_click(btn_cancel)
BID_INFO     = app.pa_bind_click(btn_info)
BID_RS_OPTS  = [app.pa_bind_click(w) for w in rs_opts]
BID_OL_CLICK = app.pa_bind_click(ol)
BID_LV_CLICK = app.pa_bind_click(lv)
BID_SL_CLICK = app.pa_bind_click(sl)

# 100ms repeating animation timer
TIMER_ANIM = app.pa_timer_create(100, 1)

# Initial render (two passes — second one picks up progressbar width)
app.pa_render()
pw.pw_progressbar_update(pb)
app.pa_render()

# ---------------------------------------------------------------------------
# Event loop
# ---------------------------------------------------------------------------
last_tick = time.monotonic()
elapsed_sec = [0]

try:
    while True:
        now      = time.monotonic()
        delta_ms = int((now - last_tick) * 1000)
        last_tick = now

        # Advance animation timer
        fired = app.pa_tick_timers(max(delta_ms, 1))
        if fired == TIMER_ANIM:
            pw.pw_loading_tick(loading_w, 100)
            pw.pw_header_tick(header)
            elapsed_sec[0] += 1
            mm = (elapsed_sec[0] // 60) % 60
            ss = elapsed_sec[0] % 60
            pw.pw_digits_set_text(digits_w, f"{mm:02d}:{ss:02d}".encode())
            app.pa_render()

        bid = app.pa_poll()

        if bid == -1:
            break
        elif bid == 0:
            time.sleep(0.016)
            continue
        elif bid == BID_QUIT:
            app.pa_quit()

        elif bid == BID_DOWN:
            foc = app.pa_focus_get()
            if   foc == ol: pw.pw_optionlist_cursor_next(ol)
            elif foc == sl: pw.pw_selectionlist_cursor_next(sl)
            elif foc == lv: pw.pw_listview_cursor_next(lv)
            else:           app.pa_focus_next()
            app.pa_render()

        elif bid == BID_UP:
            foc = app.pa_focus_get()
            if   foc == ol: pw.pw_optionlist_cursor_prev(ol)
            elif foc == sl: pw.pw_selectionlist_cursor_prev(sl)
            elif foc == lv: pw.pw_listview_cursor_prev(lv)
            else:           app.pa_focus_prev()
            app.pa_render()

        elif bid == BID_SPACE:
            foc = app.pa_focus_get()
            if   foc == toggle_w: pw.pw_toggle_toggle(toggle_w)
            elif foc == check_w:  pw.pw_checkbox_toggle(check_w)
            elif foc == switch_w: pw.pw_switch_toggle(switch_w)
            elif foc == sl:
                cur = pw.pw_selectionlist_get_cursor(sl)
                pw.pw_selectionlist_toggle_selection(sl, cur)
            app.pa_render()

        elif bid == BID_PROG:
            progress_val[0] = min(100.0, progress_val[0] + 10.0)
            pw.pw_progressbar_set_progress(pb, ctypes.c_float(progress_val[0]))
            pw.pw_progressbar_update(pb)
            app.pa_render()

        elif bid == BID_R1:
            pw.pw_radioset_select(rs, 0); app.pa_render()
        elif bid == BID_R2:
            pw.pw_radioset_select(rs, 1); app.pa_render()
        elif bid == BID_R3:
            pw.pw_radioset_select(rs, 2); app.pa_render()

        elif bid in BID_RS_OPTS:
            pw.pw_radioset_select(rs, BID_RS_OPTS.index(bid))
            app.pa_render()

        elif bid == BID_OL_CLICK:
            idx = click_row(ol)
            pw.pw_optionlist_set_cursor(ol, idx)
            app.pa_render()

        elif bid == BID_LV_CLICK:
            idx = click_row(lv)
            pw.pw_listview_set_cursor(lv, idx)
            app.pa_render()

        elif bid == BID_SL_CLICK:
            idx = click_row(sl)
            pw.pw_selectionlist_set_cursor(sl, idx)
            pw.pw_selectionlist_toggle_selection(sl, idx)
            app.pa_render()

        elif bid == BID_TOGGLE:
            pw.pw_toggle_toggle(toggle_w)
            app.pa_render()
        elif bid == BID_CHECK:
            pw.pw_checkbox_toggle(check_w)
            app.pa_render()
        elif bid == BID_SWITCH:
            pw.pw_switch_toggle(switch_w)
            app.pa_render()

        elif bid == BID_OK:
            pw.pw_label_set_text(status_lbl, b"Last: OK clicked")
            app.pa_render()
        elif bid == BID_CANCEL:
            pw.pw_label_set_text(status_lbl, b"Last: Cancel clicked")
            app.pa_render()
        elif bid == BID_INFO:
            foc = app.pa_focus_get()
            ol_cur = pw.pw_optionlist_get_cursor(ol)
            sl_cur = pw.pw_selectionlist_get_cursor(sl)
            msg = f"focus={foc}  ol={ol_cur}  sl={sl_cur}".encode()
            pw.pw_label_set_text(status_lbl, msg)
            app.pa_render()

finally:
    pw.pw_shutdown()
    app.pa_shutdown()
