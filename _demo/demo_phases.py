#!/usr/bin/env python3
"""
PlagueTUI -- Phase showcase demo
Each menu item shows the widgets introduced in that phase.

  Arrow up/down or j/k   navigate menu
  Tab / Shift+Tab         cycle focus within pane
  Space                   toggle focused widget
  p                       +10% progress (Phase 2)
  l                       append log line (Phase 7)
  Ctrl+Q                  quit
"""
import ctypes, os, sys, time
from pathlib import Path

BASE    = Path(__file__).resolve().parent.parent
BIN_APP = BASE / "plague-app"     / "bin"
BIN_W   = BASE / "plague-widgets" / "bin"

if sys.platform != "win32":
    print("Windows only.")
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

PL_SIZE_FIXED      = 0
PL_SIZE_FRACTION   = 1
PL_LAYOUT_VERTICAL   = 0
PL_LAYOUT_HORIZONTAL = 1
PL_LAYOUT_DOCK       = 2
PL_DOCK_TOP    = 0
PL_DOCK_BOTTOM = 1
PL_DOCK_LEFT   = 2
PL_DOCK_RIGHT  = 3
PL_DOCK_CENTER = 4
PT_MOD_NONE    = 0
PT_MOD_SHIFT   = 1

KEY_UP    = 0x0100
KEY_DOWN  = 0x0101

def fixed(v): return PL_SizeValue(PL_SIZE_FIXED,    v)
def fr(v=1):  return PL_SizeValue(PL_SIZE_FRACTION, v)
def sp(t=0,r=0,b=0,l=0): return TG_Spacing(t,r,b,l)
def pad(n):   return TG_Spacing(n,n,n,n)

C  = ctypes.c_int
CP = ctypes.c_char_p
CF = ctypes.c_float
SV = PL_SizeValue
SP = TG_Spacing

def _sig(fn, restype, *argtypes):
    fn.restype  = restype
    fn.argtypes = list(argtypes)

# ---------------------------------------------------------------------------
# Signatures
# ---------------------------------------------------------------------------
_sig(app.pa_init,                 C)
_sig(app.pa_shutdown,             None)
_sig(app.pa_css_load,             C,    CP)
_sig(app.pa_widget_add,           C,    CP, CP, CP, C)
_sig(app.pa_widget_set_text,      None, C,  CP)
_sig(app.pa_widget_set_size,      None, C,  SV, SV)
_sig(app.pa_widget_set_dock,      None, C,  C)
_sig(app.pa_widget_set_layout,    None, C,  C)
_sig(app.pa_widget_set_padding,   None, C,  SP)
_sig(app.pa_widget_set_focusable, None, C,  C)
_sig(app.pa_widget_set_visible,   None, C,  C)
_sig(app.pa_widget_region,        TG_Region, C)
_sig(app.pa_render,               None)
_sig(app.pa_focus_get,            C)
_sig(app.pa_focus_next,           None)
_sig(app.pa_focus_prev,           None)
_sig(app.pa_bind_key,             C,    C, C, C)
_sig(app.pa_bind_click,           C,    C)
_sig(app.pa_mouse_pos,            None, ctypes.POINTER(C), ctypes.POINTER(C))
_sig(app.pa_poll,                 C)
_sig(app.pa_quit,                 None)
_sig(app.pa_timer_create,         C,    C, C)
_sig(app.pa_tick_timers,          C,    C)

_sig(pw.pw_init,              None)
_sig(pw.pw_shutdown,          None)
_sig(pw.pw_default_tcss,      CP)

_sig(pw.pw_static_create,           C,    CP, C)
_sig(pw.pw_label_create,            C,    CP, CP, C)
_sig(pw.pw_label_set_text,          None, C,  CP)
_sig(pw.pw_rule_create,             C,    CP, C)
_sig(pw.pw_placeholder_create,      C,    CP, C)
_sig(pw.pw_placeholder_set_variant, None, C,  C)

_sig(pw.pw_progressbar_create,       C,    CF, C)
_sig(pw.pw_progressbar_set_progress, None, C,  CF)
_sig(pw.pw_progressbar_update,       None, C)
_sig(pw.pw_sparkline_create,         C,    C)
_sig(pw.pw_sparkline_set_data,       None, C,  ctypes.POINTER(CF), C)
_sig(pw.pw_digits_create,            C,    CP, C)
_sig(pw.pw_digits_set_text,          None, C,  CP)
_sig(pw.pw_loading_create,           C,    C)
_sig(pw.pw_loading_tick,             None, C,  C)
_sig(pw.pw_loading_start,            None, C)

_sig(pw.pw_header_create,    C,    CP, CP, C)
_sig(pw.pw_header_set_clock, None, C,  C)
_sig(pw.pw_header_tick,      None, C)
_sig(pw.pw_footer_create,    C,    C)
_sig(pw.pw_footer_add_key,   None, C,  CP, CP)

_sig(pw.pw_toggle_create,      C,    CP, C, C)
_sig(pw.pw_toggle_toggle,      None, C)
_sig(pw.pw_checkbox_create,    C,    CP, C, C)
_sig(pw.pw_checkbox_toggle,    None, C)
_sig(pw.pw_switch_create,      C,    C, C)
_sig(pw.pw_switch_toggle,      None, C)
_sig(pw.pw_button_create,      C,    CP, CP, C)

_sig(pw.pw_radioset_create,    C,    C)
_sig(pw.pw_radioset_add,       None, C,  CP)
_sig(pw.pw_radioset_select,    None, C,  C)

_sig(pw.pw_optionlist_create,      C,    C)
_sig(pw.pw_optionlist_add_option,  None, C,  CP)
_sig(pw.pw_optionlist_get_cursor,  C,    C)
_sig(pw.pw_optionlist_set_cursor,  None, C,  C)
_sig(pw.pw_optionlist_cursor_next, None, C)
_sig(pw.pw_optionlist_cursor_prev, None, C)

_sig(pw.pw_selectionlist_create,           C,    C)
_sig(pw.pw_selectionlist_add_option,       C,    C,  CP, C)
_sig(pw.pw_selectionlist_get_cursor,       C,    C)
_sig(pw.pw_selectionlist_set_cursor,       None, C,  C)
_sig(pw.pw_selectionlist_cursor_next,      None, C)
_sig(pw.pw_selectionlist_cursor_prev,      None, C)
_sig(pw.pw_selectionlist_toggle_selection, None, C,  C)

_sig(pw.pw_switcher_create,   C,    C)
_sig(pw.pw_switcher_add,      None, C,  C)
_sig(pw.pw_switcher_show,     None, C,  C)
_sig(pw.pw_dispatch_click,    C,    C)
_sig(pw.pw_dispatch_scroll,   C,    C)
_sig(app.pa_bind_scroll,      C,    C)
_sig(app.pa_scroll_dy,        C)

_sig(pw.pw_input_create,        C,    CP, C, C)
_sig(pw.pw_input_register_keys, None, C)
_sig(pw.pw_input_handle,        C,    C,  C)
_sig(pw.pw_input_get_value,     CP,   C)
_sig(pw.pw_input_is_submitted,  C,    C)
_sig(pw.pw_input_set_password,  None, C,  C)
_sig(pw.pw_input_tick,          None, C,  C)

_sig(pw.pw_collapsible_create,     C,    CP, C)
_sig(pw.pw_collapsible_header_wid, C,    C)
_sig(pw.pw_collapsible_content,    C,    C)
_sig(pw.pw_collapsible_toggle,     None, C)

_sig(pw.pw_tabbedcontent_create,          C,    C)
_sig(pw.pw_tabbedcontent_add_pane,        C,    C,  CP)
_sig(pw.pw_tabbedcontent_register_clicks, None, C)
_sig(pw.pw_tabbedcontent_handle_click,    C,    C,  C)

_sig(pw.pw_tree_create,      C,    C)
_sig(pw.pw_tree_add_node,    C,    C,  C,  CP, C)
_sig(pw.pw_tree_expand,      None, C,  C)
_sig(pw.pw_tree_toggle,      None, C)
_sig(pw.pw_tree_cursor_next, None, C)
_sig(pw.pw_tree_cursor_prev, None, C)
_sig(pw.pw_tree_click_row,   C,    C,  C)

_sig(pw.pw_log_create,     C,    C)
_sig(pw.pw_richlog_create, C,    C)
_sig(pw.pw_log_write,      None, C,  CP)
_sig(pw.pw_log_line_count, C,    C)

_sig(pw.pw_textarea_create,        C,    C)
_sig(pw.pw_textarea_register_keys, None, C)
_sig(pw.pw_textarea_handle,        C,    C,  C)
_sig(pw.pw_textarea_get_text,      CP,   C)
_sig(pw.pw_textarea_set_text,      None, C,  CP)
_sig(pw.pw_textarea_tick,          None, C,  C)

_sig(pw.pw_markdown_create,        C,    C)
_sig(pw.pw_markdown_set_text,      None, C,  CP)

_sig(pw.pw_datatable_create,        C,    C)
_sig(pw.pw_datatable_register_keys, None, C)
_sig(pw.pw_datatable_add_column,    C,    C,  CP, C)
_sig(pw.pw_datatable_add_row,       C,    C)
_sig(pw.pw_datatable_set_cell,      None, C,  C,  C,  CP)
_sig(pw.pw_datatable_set_cursor_type, None, C, C)
_sig(pw.pw_datatable_set_zebra,     None, C,  C)
_sig(pw.pw_datatable_get_cursor_row, C,   C)
_sig(pw.pw_datatable_get_cursor_col, C,   C)
_sig(pw.pw_datatable_move_cursor,   None, C,  C,  C)
_sig(pw.pw_datatable_handle,        C,    C,  C)
_sig(pw.pw_datatable_is_selected,   C,    C)

_sig(pw.pw_welcome_create,          C,    C,  CP, CP)
_sig(pw.pw_welcome_button_wid,      C,    C)

# ---------------------------------------------------------------------------
# Init
# ---------------------------------------------------------------------------
app.pa_init()
pw.pw_init()
app.pa_css_load(pw.pw_default_tcss())

# ---------------------------------------------------------------------------
# Root layout (same structure as demo.py)
# ---------------------------------------------------------------------------
root = app.pa_widget_add(b"Screen", b"screen", b"", -1)
app.pa_widget_set_layout(root, PL_LAYOUT_DOCK)

header = pw.pw_header_create(b"PlagueTUI -- Widget Showcase", b"", root)
app.pa_widget_set_dock(header, PL_DOCK_TOP)
pw.pw_header_set_clock(header, 1)

footer = pw.pw_footer_create(root)
app.pa_widget_set_dock(footer, PL_DOCK_BOTTOM)
pw.pw_footer_add_key(footer, b"Ctrl+Q", b"Quit")
pw.pw_footer_add_key(footer, b"j/k",    b"Navigate")
pw.pw_footer_add_key(footer, b"Tab",    b"Focus")
pw.pw_footer_add_key(footer, b"Space",  b"Toggle")
pw.pw_footer_add_key(footer, b"p",      b"+Progress")
pw.pw_footer_add_key(footer, b"l",      b"+Log")

# Sidebar (same as demo.py)
sidebar = app.pa_widget_add(b"Sidebar", b"sidebar", b"", root)
app.pa_widget_set_dock(sidebar, PL_DOCK_LEFT)
app.pa_widget_set_size(sidebar, fixed(20), fr())
app.pa_widget_set_padding(sidebar, pad(1))

# Main content area
main_area = app.pa_widget_add(b"Main", b"main", b"", root)
app.pa_widget_set_dock(main_area, PL_DOCK_CENTER)
app.pa_widget_set_padding(main_area, sp(1,2,1,2))
app.pa_widget_set_layout(main_area, PL_LAYOUT_VERTICAL)

# ContentSwitcher inside main area
switcher = pw.pw_switcher_create(main_area)
app.pa_widget_set_size(switcher, fr(), fr())

MENU_ITEMS = [
    b"1. Display",
    b"2. Data Viz",
    b"3. Buttons",
    b"4. Lists",
    b"5. Input",
    b"6. Navigation",
    b"7. Logs",
    b"8. Edit",
    b"9. DataTable",
    b"10. Welcome",
]
NUM_PHASES = len(MENU_ITEMS)

sidebar_ol = pw.pw_optionlist_create(sidebar)
for item in MENU_ITEMS:
    pw.pw_optionlist_add_option(sidebar_ol, item)

# ---------------------------------------------------------------------------
# Panes — all created as children of the switcher
# ---------------------------------------------------------------------------
def pane(name):
    p = app.pa_widget_add(b"container", name, b"", switcher)
    app.pa_widget_set_size(p, fr(), fr())
    app.pa_widget_set_layout(p, PL_LAYOUT_VERTICAL)
    pw.pw_switcher_add(switcher, p)
    return p

# ── Pane 0: Display ──────────────────────────────────────────────────────
p0 = pane(b"p0")
pw.pw_label_create(b"Phase 1 -- Display", b"primary", p0)
pw.pw_rule_create(None, p0)
pw.pw_static_create(b"Static - plain text, no interaction", p0)
pw.pw_label_create(b"Label  primary",   b"primary",   p0)
pw.pw_label_create(b"Label  success",   b"success",   p0)
pw.pw_label_create(b"Label  warning",   b"warning",   p0)
pw.pw_label_create(b"Label  error",     b"error",     p0)
pw.pw_rule_create(None, p0)
row_ph = app.pa_widget_add(b"container", b"row_ph", b"", p0)
app.pa_widget_set_layout(row_ph, PL_LAYOUT_HORIZONTAL)
app.pa_widget_set_size(row_ph, fr(), fixed(3))
for i in range(4):
    ph = pw.pw_placeholder_create(f"Box {i}".encode(), row_ph)
    app.pa_widget_set_size(ph, fr(), fr())
    pw.pw_placeholder_set_variant(ph, i)

# ── Pane 1: Data Viz ─────────────────────────────────────────────────────
p1 = pane(b"p1")
pw.pw_label_create(b"Phase 2 -- Data Visualization", b"primary", p1)
pw.pw_rule_create(None, p1)

pw.pw_label_create(b"ProgressBar  (press p to advance):", b"secondary", p1)
pb = pw.pw_progressbar_create(ctypes.c_float(100.0), p1)
app.pa_widget_set_size(pb, fr(), fixed(1))

pw.pw_label_create(b"Sparkline:", b"secondary", p1)
spark_data = (CF * 20)(*[float((i * 3) % 10) for i in range(20)])
spark = pw.pw_sparkline_create(p1)
app.pa_widget_set_size(spark, fr(), fixed(3))
pw.pw_sparkline_set_data(spark, spark_data, 20)

pw.pw_label_create(b"Digits:", b"secondary", p1)
digits_w = pw.pw_digits_create(b"00:00", p1)
app.pa_widget_set_size(digits_w, fr(), fixed(3))

pw.pw_rule_create(None, p1)
load_row = app.pa_widget_add(b"container", b"lrow", b"", p1)
app.pa_widget_set_layout(load_row, PL_LAYOUT_HORIZONTAL)
loading_w = pw.pw_loading_create(load_row)
pw.pw_loading_start(loading_w)
pw.pw_static_create(b"  LoadingIndicator", load_row)

progress_val = [0.0]

# ── Pane 2: Buttons & toggles ────────────────────────────────────────────
p2 = pane(b"p2")
pw.pw_label_create(b"Phase 3 -- Toggle / Checkbox / Switch / Buttons", b"primary", p2)
pw.pw_rule_create(None, p2)

toggle_w = pw.pw_toggle_create(b"Enable notifications", 0, p2)
check_w  = pw.pw_checkbox_create(b"Accept terms", 0, p2)

sw_row = app.pa_widget_add(b"container", b"swr", b"", p2)
app.pa_widget_set_layout(sw_row, PL_LAYOUT_HORIZONTAL)
app.pa_widget_set_size(sw_row, fr(), fixed(1))
lbl_sw = pw.pw_static_create(b"Dark mode: ", sw_row)
app.pa_widget_set_size(lbl_sw, fixed(12), fixed(1))
switch_w = pw.pw_switch_create(0, sw_row)

pw.pw_rule_create(None, p2)
btn_row = app.pa_widget_add(b"container", b"btnr", b"", p2)
app.pa_widget_set_layout(btn_row, PL_LAYOUT_HORIZONTAL)
btn_ok     = pw.pw_button_create(b"OK",     b"success", btn_row)
btn_cancel = pw.pw_button_create(b"Cancel", b"error",   btn_row)
btn_info   = pw.pw_button_create(b"Info",   b"primary", btn_row)
btn_status = pw.pw_label_create(b"", b"secondary", p2)

# ── Pane 3: Lists ────────────────────────────────────────────────────────
p3 = pane(b"p3")
pw.pw_label_create(b"Phase 4 -- RadioSet / OptionList / SelectionList", b"primary", p3)
pw.pw_rule_create(None, p3)

lists_row = app.pa_widget_add(b"container", b"lrow3", b"", p3)
app.pa_widget_set_layout(lists_row, PL_LAYOUT_HORIZONTAL)
app.pa_widget_set_size(lists_row, fr(), fr())

col_r = app.pa_widget_add(b"container", b"cr", b"", lists_row)
app.pa_widget_set_size(col_r, fr(), fr())
app.pa_widget_set_layout(col_r, PL_LAYOUT_VERTICAL)
pw.pw_label_create(b"RadioSet", b"secondary", col_r)
rs = pw.pw_radioset_create(col_r)
pw.pw_radioset_add(rs, b"Low priority")
pw.pw_radioset_add(rs, b"Medium priority")
pw.pw_radioset_add(rs, b"High priority")
pw.pw_radioset_select(rs, 1)

col_o = app.pa_widget_add(b"container", b"co", b"", lists_row)
app.pa_widget_set_size(col_o, fr(), fr())
app.pa_widget_set_layout(col_o, PL_LAYOUT_VERTICAL)
pw.pw_label_create(b"OptionList", b"secondary", col_o)
ol = pw.pw_optionlist_create(col_o)
for lang in [b"Python", b"C / C++", b"Rust", b"Go", b"Zig"]:
    pw.pw_optionlist_add_option(ol, lang)

col_s = app.pa_widget_add(b"container", b"cs", b"", lists_row)
app.pa_widget_set_size(col_s, fr(), fr())
app.pa_widget_set_layout(col_s, PL_LAYOUT_VERTICAL)
pw.pw_label_create(b"SelectionList", b"secondary", col_s)
sl = pw.pw_selectionlist_create(col_s)
for feat in [b"Syntax hl", b"Autocomplete", b"Git", b"Spell check"]:
    pw.pw_selectionlist_add_option(sl, feat, 0)

# ── Pane 4: Input ────────────────────────────────────────────────────────
p4 = pane(b"p4")
pw.pw_label_create(b"Phase 5 -- Input / TextArea", b"primary", p4)
pw.pw_rule_create(None, p4)

pw.pw_label_create(b"Name:", b"secondary", p4)
input_name = pw.pw_input_create(b"Enter name...", 64, p4)
app.pa_widget_set_size(input_name, fr(), fixed(1))
pw.pw_input_register_keys(input_name)

pw.pw_label_create(b"Password:", b"secondary", p4)
input_pass = pw.pw_input_create(b"Enter password...", 32, p4)
app.pa_widget_set_size(input_pass, fr(), fixed(1))
pw.pw_input_register_keys(input_pass)
pw.pw_input_set_password(input_pass, 1)

input_status = pw.pw_label_create(b"", b"success", p4)
pw.pw_rule_create(None, p4)

pw.pw_label_create(b"TextArea:", b"secondary", p4)
ta_w = pw.pw_textarea_create(p4)
app.pa_widget_set_size(ta_w, fr(), fixed(5))
pw.pw_textarea_register_keys(ta_w)

# ── Pane 5: Navigation ───────────────────────────────────────────────────
p5 = pane(b"p5")
pw.pw_label_create(b"Phase 6 -- Collapsible / TabbedContent / Tree", b"primary", p5)
pw.pw_rule_create(None, p5)

coll_w   = pw.pw_collapsible_create(b"Project settings", p5)
app.pa_widget_set_size(coll_w, fr(), fixed(4))
coll_hdr = pw.pw_collapsible_header_wid(coll_w)
coll_body= pw.pw_collapsible_content(coll_w)
pw.pw_static_create(b"  theme: dark",   coll_body)
pw.pw_static_create(b"  lang: es",      coll_body)
pw.pw_static_create(b"  autosave: on",  coll_body)

pw.pw_rule_create(None, p5)
tc_w   = pw.pw_tabbedcontent_create(p5)
app.pa_widget_set_size(tc_w, fr(), fixed(5))
pa_a   = pw.pw_tabbedcontent_add_pane(tc_w, b"Stats")
pa_b   = pw.pw_tabbedcontent_add_pane(tc_w, b"Config")
pa_c   = pw.pw_tabbedcontent_add_pane(tc_w, b"Help")
pw.pw_static_create(b"  CPU: 42%  RAM: 1.2 GB", pa_a)
pw.pw_static_create(b"  theme: dark  lang: es",  pa_b)
pw.pw_static_create(b"  Tab to switch panes",     pa_c)
pw.pw_tabbedcontent_register_clicks(tc_w)

pw.pw_rule_create(None, p5)
pw.pw_label_create(b"Tree (click nodes to expand):", b"secondary", p5)
tree_w = pw.pw_tree_create(p5)
app.pa_widget_set_size(tree_w, fr(), fixed(7))
n_src  = pw.pw_tree_add_node(tree_w, -1,     b"src",          0)
pw.pw_tree_add_node(tree_w, n_src,  b"app.c",        1)
pw.pw_tree_add_node(tree_w, n_src,  b"widgets.c",    1)
n_inc  = pw.pw_tree_add_node(tree_w, -1,     b"include",      0)
pw.pw_tree_add_node(tree_w, n_inc,  b"plague_app.h", 1)
pw.pw_tree_add_node(tree_w, n_inc,  b"plague_wid.h", 1)
n_test = pw.pw_tree_add_node(tree_w, -1,     b"tests",        0)
pw.pw_tree_add_node(tree_w, n_test, b"test_app.py",  1)
pw.pw_tree_expand(tree_w, n_src)
pw.pw_tree_expand(tree_w, n_inc)

# ── Pane 6: Logs ─────────────────────────────────────────────────────────
p6 = pane(b"p6")
pw.pw_label_create(b"Phase 7 -- Log / RichLog", b"primary", p6)
pw.pw_rule_create(None, p6)

pw.pw_label_create(b"Log  (plain text + scroll, press l to append):", b"secondary", p6)
log_w = pw.pw_log_create(p6)
app.pa_widget_set_size(log_w, fr(), fixed(6))
pw.pw_log_write(log_w, b"[2026-03-27 14:00:01] [INFO] demo started")
pw.pw_log_write(log_w, b"[2026-03-27 14:00:02] [INFO] widgets ready")
pw.pw_log_write(log_w, b"[2026-03-27 14:00:03] [DEBUG] loading config")
pw.pw_log_write(log_w, b"[2026-03-27 14:00:04] [INFO] config loaded")
pw.pw_log_write(log_w, b"[2026-03-27 14:00:05] [WARN] high memory usage (87%)")
pw.pw_log_write(log_w, b"[2026-03-27 14:00:06] [WARN] disk usage (91%)")
pw.pw_log_write(log_w, b"[2026-03-27 14:00:07] [ERR] connection lost {host: localhost, port: 5432}")
pw.pw_log_write(log_w, b"[2026-03-27 14:00:08] [INFO] reconnecting...")
pw.pw_log_write(log_w, b"[2026-03-27 14:00:09] [INFO] connection restored")
pw.pw_log_write(log_w, b"[2026-03-27 14:00:10] [DEBUG] sync completed (312ms)")

pw.pw_rule_create(None, p6)
pw.pw_label_create(b"RichLog  (color markup + syntax highlight):", b"secondary", p6)
rlog_w = pw.pw_richlog_create(p6)
app.pa_widget_set_size(rlog_w, fr(), fixed(6))
pw.pw_log_write(rlog_w, b"[2026-03-27 14:00:01] [green]OK[/green] connected to server")
pw.pw_log_write(rlog_w, b"[2026-03-27 14:00:02] [green]OK[/green] auth successful")
pw.pw_log_write(rlog_w, b"[2026-03-27 14:00:03] [yellow]WARN[/yellow] latency (120ms)")
pw.pw_log_write(rlog_w, b"[2026-03-27 14:00:04] [yellow]WARN[/yellow] queue depth (450)")
pw.pw_log_write(rlog_w, b"[2026-03-27 14:00:05] [red]ERR[/red] timeout on /api/data")
pw.pw_log_write(rlog_w, b"[2026-03-27 14:00:06] [red]ERR[/red] db write failed {table: events}")
pw.pw_log_write(rlog_w, b"[2026-03-27 14:00:07] [cyan]DEBUG[/cyan] retry {attempt: 1, max: 3}")
pw.pw_log_write(rlog_w, b"[2026-03-27 14:00:08] [cyan]DEBUG[/cyan] retry {attempt: 2, max: 3}")
pw.pw_log_write(rlog_w, b"[2026-03-27 14:00:09] [green]OK[/green] recovered")
pw.pw_log_write(rlog_w, b"[2026-03-27 14:00:10] [green]OK[/green] sync complete (312ms)")

# ── Pane 7: Edit ─────────────────────────────────────────────────────────
p7 = pane(b"p7")
p7_lbl  = pw.pw_label_create(b"Phase 7.2 -- TextArea / Markdown", b"primary", p7)
app.pa_widget_set_size(p7_lbl, fr(), fixed(1))
p7_rule = pw.pw_rule_create(None, p7)
app.pa_widget_set_size(p7_rule, fr(), fixed(1))

edit_row = app.pa_widget_add(b"container", b"edit_row", b"", p7)
app.pa_widget_set_layout(edit_row, PL_LAYOUT_HORIZONTAL)
app.pa_widget_set_size(edit_row, fr(), fr())

# Left column: editable TextArea
edit_left = app.pa_widget_add(b"container", b"edit_left", b"", edit_row)
app.pa_widget_set_layout(edit_left, PL_LAYOUT_VERTICAL)
app.pa_widget_set_size(edit_left, fr(), fr())
edit_left_lbl = pw.pw_label_create(b"TextArea (edit here):", b"secondary", edit_left)
app.pa_widget_set_size(edit_left_lbl, fr(), fixed(1))
ta2_w = pw.pw_textarea_create(edit_left)
app.pa_widget_set_size(ta2_w, fr(), fr())
pw.pw_textarea_register_keys(ta2_w)
pw.pw_textarea_set_text(ta2_w,
    b"Hello, PlagueTUI!\n"
    b"This is a multi-line\n"
    b"editable text area.\n"
    b"Use Tab to focus it,\n"
    b"then type freely.")

# Right column: Markdown preview
edit_right = app.pa_widget_add(b"container", b"edit_right", b"", edit_row)
app.pa_widget_set_layout(edit_right, PL_LAYOUT_VERTICAL)
app.pa_widget_set_size(edit_right, fr(), fr())
edit_right_lbl = pw.pw_label_create(b"Markdown preview:", b"secondary", edit_right)
app.pa_widget_set_size(edit_right_lbl, fr(), fixed(1))
md_w = pw.pw_markdown_create(edit_right)
app.pa_widget_set_size(md_w, fr(), fr())
pw.pw_markdown_set_text(md_w,
    b"# H1 - PlagueTUI Markdown\n"
    b"## H2 - Syntax showcase\n"
    b"### H3 - All types below\n"
    b"Plain text, no markup applied here.\n"
    b"Inline: **bold text**, *italic text*, `inline code`.\n"
    b"Mixed: **bold** and *italic* and `code` combined.\n"
    b"## H2 - Bullet list\n"
    b"- Plain bullet item\n"
    b"- Bullet with **bold** content\n"
    b"- Bullet with *italic* content\n"
    b"- Bullet with `code` content\n"
    b"- **bold** + *italic* + `code`\n"
    b"### H3 - More inline\n"
    b"*italic paragraph* with **bold** word.\n"
    b"`code block` next to **bold** text.")

# ── Pane 8: DataTable ────────────────────────────────────────────────────
p8 = pane(b"p8")
p8_lbl = pw.pw_label_create(b"Phase 10 -- DataTable", b"primary", p8)
app.pa_widget_set_size(p8_lbl, fr(), fixed(1))

# Cursor mode selector row
dt_mode_row = app.pa_widget_add(b"container", b"dt_mode_row", b"", p8)
app.pa_widget_set_layout(dt_mode_row, PL_LAYOUT_HORIZONTAL)
app.pa_widget_set_size(dt_mode_row, fr(), fixed(1))
pw.pw_static_create(b"Cursor: ", dt_mode_row)
dt_btn_none = pw.pw_button_create(b"None",   b"default", dt_mode_row)
dt_btn_cell = pw.pw_button_create(b"Cell",   b"primary", dt_mode_row)
dt_btn_row  = pw.pw_button_create(b"Row",    b"success", dt_mode_row)
dt_btn_col  = pw.pw_button_create(b"Column", b"warning", dt_mode_row)
dt_status   = pw.pw_label_create(b"", b"secondary", p8)
app.pa_widget_set_size(dt_status, fr(), fixed(1))

dt_w = pw.pw_datatable_create(p8)
app.pa_widget_set_size(dt_w, fr(), fr())
app.pa_widget_set_focusable(dt_w, 1)
pw.pw_datatable_register_keys(dt_w)
pw.pw_datatable_set_cursor_type(dt_w, 2)   # ROW
pw.pw_datatable_set_zebra(dt_w, 1)

pw.pw_datatable_add_column(dt_w, b"Name",       12)
pw.pw_datatable_add_column(dt_w, b"Role",       14)
pw.pw_datatable_add_column(dt_w, b"Language",   10)
pw.pw_datatable_add_column(dt_w, b"Stars",       6)
pw.pw_datatable_add_column(dt_w, b"Active",      7)

_DT_ROWS = [
    (b"Alice",    b"Engineer",    b"Python",  b"4821",  b"Yes"),
    (b"Bob",      b"Designer",    b"JS",      b"1203",  b"Yes"),
    (b"Carol",    b"Lead",        b"Rust",    b"9932",  b"Yes"),
    (b"Dave",     b"DevOps",      b"Go",      b"542",   b"No"),
    (b"Eve",      b"Researcher",  b"C",       b"7714",  b"Yes"),
    (b"Frank",    b"Backend",     b"Java",    b"2890",  b"No"),
    (b"Grace",    b"Frontend",    b"TS",      b"3310",  b"Yes"),
    (b"Hank",     b"QA",          b"Python",  b"88",    b"No"),
    (b"Iris",     b"ML Eng",      b"Python",  b"6601",  b"Yes"),
    (b"Jack",     b"Infra",       b"Bash",    b"445",   b"Yes"),
    (b"Karen",    b"PM",          b"N/A",     b"0",     b"Yes"),
    (b"Leo",      b"Security",    b"C++",     b"2100",  b"No"),
]
for row_data in _DT_ROWS:
    r = pw.pw_datatable_add_row(dt_w)
    for ci, val in enumerate(row_data):
        pw.pw_datatable_set_cell(dt_w, r, ci, val)

BID_DT_NONE = app.pa_bind_click(dt_btn_none)
BID_DT_CELL = app.pa_bind_click(dt_btn_cell)
BID_DT_ROW  = app.pa_bind_click(dt_btn_row)
BID_DT_COL  = app.pa_bind_click(dt_btn_col)

# ── Pane 9: Welcome ───────────────────────────────────────────────────────
p9 = pane(b"p9")
p9_lbl = pw.pw_label_create(b"Phase 10 -- Welcome", b"primary", p9)
app.pa_widget_set_size(p9_lbl, fr(), fixed(1))

wlc_w = pw.pw_welcome_create(p9,
    b"# Welcome to PlagueTUI\n"
    b"## A native TUI framework in C\n"
    b"\n"
    b"All **35 widgets** implemented across 10 phases:\n"
    b"- Display, Data Viz, Buttons, Lists\n"
    b"- Input, Navigation, Logs, Editing\n"
    b"- **DataTable** with 4 cursor modes\n"
    b"- This **Welcome** screen\n"
    b"\n"
    b"### Stack\n"
    b"*plague-terminal* -> *plague-app* -> *plague-widgets*\n"
    b"\n"
    b"`C99` + Python FFI bindings. Zero dependencies.",
    b"  Get started  ")
app.pa_widget_set_size(wlc_w, fr(), fr())
BID_WLC_BTN = app.pa_bind_click(pw.pw_welcome_button_wid(wlc_w))

# ---------------------------------------------------------------------------
# Show first pane
# ---------------------------------------------------------------------------
current = [0]
pw.pw_switcher_show(switcher, 0)
pw.pw_optionlist_set_cursor(sidebar_ol, 0)

# ---------------------------------------------------------------------------
# Bindings
# ---------------------------------------------------------------------------
BID_DOWN     = app.pa_bind_key(root, KEY_DOWN, PT_MOD_NONE)
BID_UP       = app.pa_bind_key(root, KEY_UP,   PT_MOD_NONE)
BID_DOWN_J   = app.pa_bind_key(root, ord('j'), PT_MOD_NONE)
BID_UP_K     = app.pa_bind_key(root, ord('k'), PT_MOD_NONE)
BID_SPACE    = app.pa_bind_key(root, ord(' '),  PT_MOD_NONE)
BID_PROG     = app.pa_bind_key(root, ord('p'),  PT_MOD_NONE)
BID_LOG      = app.pa_bind_key(root, ord('l'),  PT_MOD_NONE)

BID_TOGGLE_CLK = app.pa_bind_click(toggle_w)
BID_CHECK_CLK  = app.pa_bind_click(check_w)
BID_SWITCH_CLK = app.pa_bind_click(switch_w)
BID_BTN_OK     = app.pa_bind_click(btn_ok)
BID_BTN_CANCEL = app.pa_bind_click(btn_cancel)
BID_BTN_INFO   = app.pa_bind_click(btn_info)
# OL, SL, RS auto-register their own click binding internally
BID_COLL_HDR   = app.pa_bind_click(coll_hdr)
BID_COLL_W     = app.pa_bind_click(coll_w)
BID_TREE_CLK   = app.pa_bind_click(tree_w)

TIMER_ANIM = app.pa_timer_create(100, 1)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def mouse_pos():
    mx, my = C(0), C(0)
    app.pa_mouse_pos(ctypes.byref(mx), ctypes.byref(my))
    return mx.value, my.value

elapsed_sec = [0]

def nav_to(idx):
    if idx < 0: idx = 0
    if idx >= NUM_PHASES: idx = NUM_PHASES - 1
    current[0] = idx
    pw.pw_optionlist_set_cursor(sidebar_ol, idx)
    pw.pw_switcher_show(switcher, idx)
    app.pa_render()

app.pa_render()

# ---------------------------------------------------------------------------
# Event loop
# ---------------------------------------------------------------------------
while True:
    fired = app.pa_tick_timers(16)
    if fired == TIMER_ANIM:
        pw.pw_loading_tick(loading_w, 100)
        pw.pw_header_tick(header)
        pw.pw_input_tick(input_name, 100)
        pw.pw_input_tick(input_pass, 100)
        pw.pw_textarea_tick(ta_w, 100)
        pw.pw_textarea_tick(ta2_w, 100)
        elapsed_sec[0] += 1
        mm = (elapsed_sec[0] // 60) % 60
        ss = elapsed_sec[0] % 60
        pw.pw_digits_set_text(digits_w, f"{mm:02d}:{ss:02d}".encode())
        app.pa_render()
        continue

    bid = app.pa_poll()

    if bid == -1:
        break
    elif bid == 0:
        time.sleep(0.016)
        continue

    # DataTable
    elif pw.pw_datatable_handle(dt_w, bid):
        if pw.pw_datatable_is_selected(dt_w):
            r = pw.pw_datatable_get_cursor_row(dt_w)
            c = pw.pw_datatable_get_cursor_col(dt_w)
            pw.pw_label_set_text(dt_status, f"Selected row {r}, col {c}".encode())
        app.pa_render(); continue
    # Input / TextArea consume first
    elif pw.pw_input_handle(input_name, bid):
        if pw.pw_input_is_submitted(input_name):
            val = pw.pw_input_get_value(input_name).decode(errors="replace")
            pw.pw_label_set_text(input_status, f"Name: {val}".encode())
        app.pa_render(); continue
    elif pw.pw_input_handle(input_pass, bid):
        if pw.pw_input_is_submitted(input_pass):
            pw.pw_label_set_text(input_status, b"Password submitted!")
        app.pa_render(); continue
    elif pw.pw_textarea_handle(ta_w, bid):
        app.pa_render(); continue
    elif pw.pw_textarea_handle(ta2_w, bid):
        app.pa_render(); continue

    # Navigation
    elif bid in (BID_DOWN, BID_DOWN_J):
        foc = app.pa_focus_get()
        if   foc == sidebar_ol: nav_to(current[0] + 1)
        elif foc == ol:         pw.pw_optionlist_cursor_next(ol);    app.pa_render()
        elif foc == sl:         pw.pw_selectionlist_cursor_next(sl); app.pa_render()
        elif foc == tree_w:     pw.pw_tree_cursor_next(tree_w);      app.pa_render()
        else:                   nav_to(current[0] + 1)

    elif bid in (BID_UP, BID_UP_K):
        foc = app.pa_focus_get()
        if   foc == sidebar_ol: nav_to(current[0] - 1)
        elif foc == ol:         pw.pw_optionlist_cursor_prev(ol);    app.pa_render()
        elif foc == sl:         pw.pw_selectionlist_cursor_prev(sl); app.pa_render()
        elif foc == tree_w:     pw.pw_tree_cursor_prev(tree_w);      app.pa_render()
        else:                   nav_to(current[0] - 1)

    elif bid == BID_SPACE:
        foc = app.pa_focus_get()
        if   foc == toggle_w: pw.pw_toggle_toggle(toggle_w);   app.pa_render()
        elif foc == check_w:  pw.pw_checkbox_toggle(check_w);  app.pa_render()
        elif foc == switch_w: pw.pw_switch_toggle(switch_w);   app.pa_render()
        elif foc == sl:
            cur = pw.pw_selectionlist_get_cursor(sl)
            pw.pw_selectionlist_toggle_selection(sl, cur)
            app.pa_render()
        elif foc == tree_w:
            pw.pw_tree_toggle(tree_w); app.pa_render()

    elif bid == BID_PROG:
        progress_val[0] = min(100.0, progress_val[0] + 10.0)
        pw.pw_progressbar_set_progress(pb, ctypes.c_float(progress_val[0]))
        pw.pw_progressbar_update(pb)
        app.pa_render()

    elif bid == BID_LOG:
        n = pw.pw_log_line_count(log_w)
        pw.pw_log_write(log_w,  f"[INFO] line {n + 1}".encode())
        pw.pw_log_write(rlog_w, f"[cyan]line {n + 1}[/cyan] appended".encode())
        app.pa_render()

    # Clicks
    elif bid == BID_TOGGLE_CLK:
        pw.pw_toggle_toggle(toggle_w); app.pa_render()
    elif bid == BID_CHECK_CLK:
        pw.pw_checkbox_toggle(check_w); app.pa_render()
    elif bid == BID_SWITCH_CLK:
        pw.pw_switch_toggle(switch_w); app.pa_render()
    elif bid == BID_BTN_OK:
        pw.pw_label_set_text(btn_status, b"Clicked: OK"); app.pa_render()
    elif bid == BID_BTN_CANCEL:
        pw.pw_label_set_text(btn_status, b"Clicked: Cancel"); app.pa_render()
    elif bid == BID_BTN_INFO:
        pw.pw_label_set_text(btn_status, b"Clicked: Info"); app.pa_render()
    elif pw.pw_dispatch_scroll(bid):
        app.pa_render()
    elif pw.pw_dispatch_click(bid):
        idx = pw.pw_optionlist_get_cursor(sidebar_ol)
        if idx != current[0]:
            nav_to(idx)
        else:
            app.pa_render()
    elif bid == BID_COLL_HDR or bid == BID_COLL_W:
        pw.pw_collapsible_toggle(coll_w); app.pa_render()
    elif bid == BID_TREE_CLK:
        _, my = mouse_pos()
        pw.pw_tree_click_row(tree_w, my); app.pa_render()
    elif pw.pw_tabbedcontent_handle_click(tc_w, bid):
        app.pa_render()
    # DataTable cursor mode buttons
    elif bid == BID_DT_NONE:
        pw.pw_datatable_set_cursor_type(dt_w, 0); app.pa_render()
    elif bid == BID_DT_CELL:
        pw.pw_datatable_set_cursor_type(dt_w, 1); app.pa_render()
    elif bid == BID_DT_ROW:
        pw.pw_datatable_set_cursor_type(dt_w, 2); app.pa_render()
    elif bid == BID_DT_COL:
        pw.pw_datatable_set_cursor_type(dt_w, 3); app.pa_render()
    # Welcome button
    elif bid == BID_WLC_BTN:
        pw.pw_label_set_text(p9_lbl, b"Welcome dismissed -- ready to go!"); app.pa_render()

pw.pw_shutdown()
app.pa_shutdown()
