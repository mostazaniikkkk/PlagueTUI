#ifndef PLAGUE_WIDGETS_H
#define PLAGUE_WIDGETS_H

#include "../../plague-app/include/plague_app.h"

#ifdef _WIN32
  #define PW_API __declspec(dllexport)
#else
  #define PW_API __attribute__((visibility("default")))
#endif

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

/* Must be called after pa_init_headless / pa_init, before any pw_* call. */
PW_API void pw_init(void);
PW_API void pw_shutdown(void);

// ---------------------------------------------------------------------------
// Static / Label
// ---------------------------------------------------------------------------

/*
 * pw_static_create — plain text widget (no interaction).
 * pw_label_create  — same but with optional variant class applied.
 * variant: NULL, "primary", "secondary", "success", "warning", "error"
 */
PW_API int pw_static_create(const char *text, int parent_wid);
PW_API int pw_label_create (const char *text, const char *variant, int parent_wid);

PW_API void pw_label_set_text(int wid, const char *text);

// ---------------------------------------------------------------------------
// Rule
// ---------------------------------------------------------------------------

/*
 * Horizontal or vertical divider.
 * line_char: one of "─", "━", "═", "─" (UTF-8).  NULL → "─".
 */
PW_API int  pw_rule_create   (const char *line_char, int parent_wid);
PW_API void pw_rule_set_char (int wid, const char *line_char);

// ---------------------------------------------------------------------------
// Placeholder
// ---------------------------------------------------------------------------

/*
 * Debug placeholder with a coloured background and its label centred.
 * label: text shown in the centre.  NULL → shows its wid.
 * variant index 0-6 cycles through the built-in colours.
 */
PW_API int  pw_placeholder_create (const char *label, int parent_wid);
PW_API void pw_placeholder_set_variant(int wid, int variant_index);

// ---------------------------------------------------------------------------
// ProgressBar
// ---------------------------------------------------------------------------

/*
 * Unicode block progress bar (░▓█).
 * total: denominator (must be > 0).
 * progress: current value (0 … total).
 *
 * pw_progressbar_update re-renders the bar text using the widget region
 * computed during the last pa_render() call — call it AFTER pa_render().
 */
PW_API int  pw_progressbar_create(float total, int parent_wid);
PW_API void pw_progressbar_set_progress(int wid, float progress);
PW_API void pw_progressbar_update(int wid);

/* Returns current progress value [0 … total]. */
PW_API float pw_progressbar_get_progress(int wid);
PW_API float pw_progressbar_get_total   (int wid);

// ---------------------------------------------------------------------------
// Sparkline
// ---------------------------------------------------------------------------

/*
 * Single-row bar chart using ▁▂▃▄▅▆▇█.
 * data: array of float values (normalised to [min,max] internally).
 * count: number of data points.
 */
PW_API int  pw_sparkline_create(int parent_wid);
PW_API void pw_sparkline_set_data(int wid, const float *data, int count);

// ---------------------------------------------------------------------------
// Digits (big-font numbers)
// ---------------------------------------------------------------------------

/*
 * 3-row big-font display using box-drawing characters.
 * text: string containing digits 0-9 and separators : . , + - space.
 * Characters not in the supported set are rendered as spaces.
 */
PW_API int  pw_digits_create  (const char *text, int parent_wid);
PW_API void pw_digits_set_text(int wid, const char *text);

// ---------------------------------------------------------------------------
// LoadingIndicator
// ---------------------------------------------------------------------------

/*
 * Animated braille spinner (⣾⣽⣻⢿⡿⣟⣯⣷).
 * Call pw_loading_tick(wid, elapsed_ms) each frame to advance the animation.
 * The widget does NOT automatically register a timer — the caller is
 * responsible for driving pa_tick_timers() and calling pw_loading_tick().
 */
PW_API int  pw_loading_create(int parent_wid);
PW_API void pw_loading_tick  (int wid, int elapsed_ms);

/* Start / stop animating (stops advancing but keeps last frame visible). */
PW_API void pw_loading_start(int wid);
PW_API void pw_loading_stop (int wid);

// ---------------------------------------------------------------------------
// ToggleButton
// ---------------------------------------------------------------------------

/*
 * Base interactive toggle: ○ label (unchecked) / ● label (checked).
 * Focusable by default. Use pa_bind_click / pa_bind_key to react to input,
 * then call pw_toggle_toggle() to flip state and refresh text.
 */
PW_API int  pw_toggle_create     (const char *label, int checked, int parent_wid);
PW_API void pw_toggle_set_checked(int wid, int checked);
PW_API int  pw_toggle_get_checked(int wid);
PW_API void pw_toggle_set_label  (int wid, const char *label);
PW_API void pw_toggle_toggle     (int wid);

// ---------------------------------------------------------------------------
// Checkbox
// ---------------------------------------------------------------------------

/*
 * Checkbox: [ ] label (unchecked) / [X] label (checked).
 * Shares focus/state infrastructure with ToggleButton.
 */
PW_API int  pw_checkbox_create     (const char *label, int checked, int parent_wid);
PW_API void pw_checkbox_set_checked(int wid, int checked);
PW_API int  pw_checkbox_get_checked(int wid);
PW_API void pw_checkbox_set_label  (int wid, const char *label);
PW_API void pw_checkbox_toggle     (int wid);

// ---------------------------------------------------------------------------
// Switch
// ---------------------------------------------------------------------------

/*
 * Sliding switch: ○╺━━ (off) / ━━╸● (on).
 * Width is always 4 terminal cells. Focusable by default.
 */
PW_API int  pw_switch_create    (int active, int parent_wid);
PW_API void pw_switch_set_active(int wid, int active);
PW_API int  pw_switch_get_active(int wid);
PW_API void pw_switch_toggle    (int wid);

// ---------------------------------------------------------------------------
// Button
// ---------------------------------------------------------------------------

/*
 * Clickable button. Focusable by default.
 * variant: NULL/"default", "primary", "success", "warning", "error"
 * CSS border and background are applied by the active stylesheet.
 */
PW_API int  pw_button_create   (const char *label, const char *variant, int parent_wid);
PW_API void pw_button_set_label(int wid, const char *label);

// ---------------------------------------------------------------------------
// RadioButton / RadioSet
// ---------------------------------------------------------------------------

/*
 * Standalone radio button: ◯ label (unchecked) / ◉ label (checked).
 * Focusable. Normally managed by a RadioSet; can also be used standalone.
 */
PW_API int  pw_radio_create     (const char *label, int checked, int parent_wid);
PW_API void pw_radio_set_checked(int wid, int checked);
PW_API int  pw_radio_get_checked(int wid);

/*
 * RadioSet — vertical container enforcing mutual exclusivity.
 * pw_radioset_add creates a RadioButton child and returns its wid.
 * The first radio added is auto-selected.
 */
PW_API int  pw_radioset_create      (int parent_wid);
PW_API int  pw_radioset_add         (int set_wid, const char *label);
PW_API void pw_radioset_select      (int set_wid, int index);
PW_API int  pw_radioset_get_selected(int set_wid);
PW_API int  pw_radioset_count       (int set_wid);

// ---------------------------------------------------------------------------
// OptionList
// ---------------------------------------------------------------------------

/*
 * Scrollable list of options. Cursor shown with ▸ prefix.
 * pw_optionlist_add_option returns the option index (0-based) or -1 on error.
 * cursor_next / cursor_prev clamp at list boundaries (no wrapping).
 */
PW_API int  pw_optionlist_create     (int parent_wid);
PW_API int  pw_optionlist_add_option (int wid, const char *text);
PW_API void pw_optionlist_clear      (int wid);
PW_API void pw_optionlist_set_cursor (int wid, int index);
PW_API int  pw_optionlist_get_cursor (int wid);
PW_API void pw_optionlist_cursor_next(int wid);
PW_API void pw_optionlist_cursor_prev(int wid);
PW_API int  pw_optionlist_count      (int wid);

// ---------------------------------------------------------------------------
// ListView
// ---------------------------------------------------------------------------

/*
 * Scrollable list of items. Cursor shown with ▶ prefix.
 * API mirrors OptionList; CSS type is "ListView".
 */
PW_API int  pw_listview_create     (int parent_wid);
PW_API int  pw_listview_add_item   (int wid, const char *text);
PW_API void pw_listview_clear      (int wid);
PW_API void pw_listview_set_cursor (int wid, int index);
PW_API int  pw_listview_get_cursor (int wid);
PW_API void pw_listview_cursor_next(int wid);
PW_API void pw_listview_cursor_prev(int wid);
PW_API int  pw_listview_count      (int wid);

// ---------------------------------------------------------------------------
// SelectionList
// ---------------------------------------------------------------------------

/*
 * Multi-select list: each item has an independent checked state.
 * Renders as: ▸ [X] item (cursor + selected) / "  [ ] item" (normal).
 * pw_selectionlist_add_option returns the item index or -1 on error.
 */
PW_API int  pw_selectionlist_create          (int parent_wid);
PW_API int  pw_selectionlist_add_option      (int wid, const char *text, int initially_selected);
PW_API void pw_selectionlist_clear           (int wid);
PW_API void pw_selectionlist_set_cursor      (int wid, int index);
PW_API int  pw_selectionlist_get_cursor      (int wid);
PW_API void pw_selectionlist_cursor_next     (int wid);
PW_API void pw_selectionlist_cursor_prev     (int wid);
PW_API void pw_selectionlist_toggle_selection(int wid, int index);
PW_API void pw_selectionlist_set_selected    (int wid, int index, int selected);
PW_API int  pw_selectionlist_is_selected     (int wid, int index);
PW_API int  pw_selectionlist_count           (int wid);

// ---------------------------------------------------------------------------
// Header
// ---------------------------------------------------------------------------

/*
 * System header bar: icon + centred title + optional live clock.
 * Automatically docks to TOP with height=1.
 * icon: short UTF-8 string (emoji OK). NULL → no icon.
 *
 * pw_header_tick(wid) reads the system clock and refreshes the widget text.
 * Call it once per render loop tick when show_clock is enabled.
 */
PW_API int  pw_header_create   (const char *title, const char *icon, int parent_wid);
PW_API void pw_header_set_title(int wid, const char *title);
PW_API void pw_header_set_icon (int wid, const char *icon);
PW_API void pw_header_set_clock(int wid, int show);   /* 1=show, 0=hide */
PW_API void pw_header_tick     (int wid);             /* refresh clock text */

// ---------------------------------------------------------------------------
// Footer
// ---------------------------------------------------------------------------

/*
 * System footer bar: horizontal list of key-hint pairs "[key] description".
 * Automatically docks to BOTTOM with height=1.
 *
 * pw_footer_add_key    — append a key hint (up to 16 hints per footer).
 * pw_footer_clear_keys — remove all hints.
 * pw_footer_refresh    — rebuild and push the widget text (called internally
 *                        by add/clear, but exposed for manual refresh).
 */
PW_API int  pw_footer_create    (int parent_wid);
PW_API void pw_footer_add_key   (int wid, const char *key_label, const char *description);
PW_API void pw_footer_clear_keys(int wid);
PW_API void pw_footer_refresh   (int wid);

// ---------------------------------------------------------------------------
// ContentSwitcher
// ---------------------------------------------------------------------------

/*
 * Shows exactly one child widget at a time; hides the rest.
 * The switcher itself is a transparent container (no visual of its own).
 *
 * pw_switcher_add    — register a widget wid managed by this switcher.
 *                      The first widget added is shown; the rest are hidden.
 *                      Returns the slot index (0-based) or -1 on error.
 * pw_switcher_show   — make widget at index visible, hide all others.
 * pw_switcher_active — index of currently visible widget (-1 if empty).
 * pw_switcher_count  — number of registered widgets.
 */
PW_API int  pw_switcher_create(int parent_wid);
PW_API int  pw_switcher_add   (int switcher_wid, int child_wid);
PW_API void pw_switcher_show  (int switcher_wid, int index);
PW_API int  pw_switcher_active(int switcher_wid);
PW_API int  pw_switcher_count (int switcher_wid);

// ---------------------------------------------------------------------------
// Toast
// ---------------------------------------------------------------------------

/*
 * Floating notification overlay that auto-dismisses after duration_ms.
 * The Toast is created as an overlay widget docked to BOTTOM of parent_wid.
 * It remains visible until pw_toast_tick() is called with enough elapsed ms,
 * at which point pw_toast_is_visible() returns 0.
 *
 * Typical usage in your loop:
 *   fired = pa_tick_timers(elapsed_ms);
 *   if (fired == toast_timer) pw_toast_tick(toast_wid, elapsed_ms);
 *   if (!pw_toast_is_visible(toast_wid)) { ... handle dismiss ... }
 *
 * pw_toast_show — re-show with a new message and reset the timer.
 */
PW_API int  pw_toast_create    (const char *message, int duration_ms, int parent_wid);
PW_API void pw_toast_show      (int wid, const char *message, int duration_ms);
PW_API void pw_toast_tick      (int wid, int elapsed_ms);
PW_API int  pw_toast_is_visible(int wid);

// ---------------------------------------------------------------------------
// Default TCSS
// ---------------------------------------------------------------------------

/*
 * Returns a static TCSS string with sensible defaults for all plague-widgets.
 * Pass it to pa_css_load() if you don't have your own stylesheet.
 */
PW_API const char *pw_default_tcss(void);

#endif /* PLAGUE_WIDGETS_H */
