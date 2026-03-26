#include "../include/plague_widgets.h"
#include "plague_app.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

/* =========================================================================
 * Internal pool
 * ========================================================================= */

typedef enum {
    PW_TYPE_NONE = 0,
    PW_TYPE_STATIC,
    PW_TYPE_LABEL,
    PW_TYPE_RULE,
    PW_TYPE_PLACEHOLDER,
    PW_TYPE_PROGRESSBAR,
    PW_TYPE_SPARKLINE,
    PW_TYPE_DIGITS,
    PW_TYPE_LOADING,
    PW_TYPE_HEADER,
    PW_TYPE_FOOTER,
    PW_TYPE_TOGGLE,
    PW_TYPE_CHECKBOX,
    PW_TYPE_SWITCH,
    PW_TYPE_BUTTON,
    PW_TYPE_RADIO,
    PW_TYPE_RADIOSET,
    PW_TYPE_OPTIONLIST,
    PW_TYPE_LISTVIEW,
    PW_TYPE_SELECTIONLIST,
    PW_TYPE_SWITCHER,
    PW_TYPE_TOAST
} PW_Type;

#define PW_MAX_SLOTS       128
#define PW_MAX_SPARK       256   /* max data points for sparkline */
#define PW_FOOTER_MAX_KEYS  16
#define PW_MAX_OPTIONS      48   /* max items per list widget */
#define PW_OPTION_TEXT_LEN  48   /* max chars per list item */
#define PW_RADIOSET_MAX     16   /* max RadioButtons per RadioSet */
#define PW_SWITCHER_MAX     16   /* max panels per ContentSwitcher */

typedef struct {
    PW_Type widget_type;
    int     wid;        /* plague-app widget id */

    union {
        /* Rule */
        struct {
            char line_char[8]; /* UTF-8 char (up to 4 bytes + NUL) */
        } rule;

        /* Placeholder */
        struct {
            int variant_index;
        } placeholder;

        /* ProgressBar */
        struct {
            float progress;
            float total;
        } progressbar;

        /* Sparkline */
        struct {
            float data[PW_MAX_SPARK];
            int   count;
        } sparkline;

        /* LoadingIndicator */
        struct {
            int   frame;        /* 0-7  */
            int   running;
            int   accum_ms;     /* accumulated ms toward next frame */
        } loading;

        /* Header */
        struct {
            char title[128];
            char icon[16];
            int  show_clock;
        } header;

        /* Footer */
        struct {
            char key_label [PW_FOOTER_MAX_KEYS][16];
            char description[PW_FOOTER_MAX_KEYS][48];
            int  count;
        } footer;

        /* ToggleButton and Checkbox (identical fields, different type) */
        struct {
            char label[128];
            int  checked;
        } toggle;

        /* Switch */
        struct {
            int active;
        } sw;

        /* Button */
        struct {
            char label[128];
            char variant[32];
        } button;

        /* OptionList / ListView / SelectionList (shared struct, type discriminates) */
        struct {
            char    items  [PW_MAX_OPTIONS][PW_OPTION_TEXT_LEN];
            uint8_t checked[PW_MAX_OPTIONS]; /* SelectionList: per-item checked */
            int     count;
            int     cursor;
        } list;

        /* RadioSet — manages child RadioButton wids */
        struct {
            int radio_wids[PW_RADIOSET_MAX];
            int count;
            int selected; /* index of currently selected radio (-1 = none) */
        } radioset;

        /* ContentSwitcher */
        struct {
            int child_wids[PW_SWITCHER_MAX];
            int count;
            int active; /* index of visible child (-1 = none) */
        } switcher;

        /* Toast */
        struct {
            int duration_ms;
            int elapsed_ms;
            int visible;
        } toast;
    } u;
} PW_Slot;

static PW_Slot g_pool[PW_MAX_SLOTS];
static int     g_initialised = 0;

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

void pw_init(void)
{
    memset(g_pool, 0, sizeof(g_pool));
    g_initialised = 1;
}

void pw_shutdown(void)
{
    if (!g_initialised) return;
    memset(g_pool, 0, sizeof(g_pool));
    g_initialised = 0;
}

/* =========================================================================
 * Pool helpers
 * ========================================================================= */

static PW_Slot *slot_alloc(PW_Type type, int wid)
{
    for (int i = 0; i < PW_MAX_SLOTS; i++) {
        if (g_pool[i].widget_type == PW_TYPE_NONE) {
            g_pool[i].widget_type = type;
            g_pool[i].wid         = wid;
            return &g_pool[i];
        }
    }
    return NULL; /* pool full */
}

static PW_Slot *slot_find(int wid)
{
    for (int i = 0; i < PW_MAX_SLOTS; i++) {
        if (g_pool[i].widget_type != PW_TYPE_NONE && g_pool[i].wid == wid)
            return &g_pool[i];
    }
    return NULL;
}

/* =========================================================================
 * Rule text generation
 * =========================================================================
 * We fill the widget text with 256 repetitions of the UTF-8 line_char.
 * plague-app clips the text to the widget region automatically.
 * ========================================================================= */

#define RULE_REPS 256

static void rule_refresh(PW_Slot *s)
{
    /* Each UTF-8 char is at most 3 bytes; NUL at end: 256*3+1=769 < 4096 */
    static char buf[RULE_REPS * 4 + 2];
    int pos = 0;
    int clen = (int)strlen(s->u.rule.line_char);
    for (int i = 0; i < RULE_REPS; i++) {
        memcpy(buf + pos, s->u.rule.line_char, clen);
        pos += clen;
    }
    buf[pos] = '\0';
    pa_widget_set_text(s->wid, buf);
}

/* =========================================================================
 * ProgressBar text generation
 * ========================================================================= */

/* Block chars: empty ░ (U+2591), half ▓ (U+2593), full █ (U+2588) */
static const char *PB_FULL  = "\xe2\x96\x88"; /* █ */
static const char *PB_EMPTY = "\xe2\x96\x91"; /* ░ */

static void progressbar_refresh(PW_Slot *s, int bar_width)
{
    if (bar_width <= 0) bar_width = 20; /* fallback before first render */

    float ratio    = (s->u.progressbar.total > 0.0f)
                   ? (s->u.progressbar.progress / s->u.progressbar.total)
                   : 0.0f;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    /* Reserve 6 chars for " 100% " suffix */
    int suffix_len = 7; /* " NNN%  " */
    int avail      = bar_width - suffix_len;
    if (avail < 1) avail = 1;

    int filled  = (int)(ratio * avail + 0.5f);
    if (filled > avail) filled = avail;
    int empty   = avail - filled;

    /* Build text: 3 bytes per block char */
    static char buf[4096];
    int pos = 0;
    for (int i = 0; i < filled; i++) { memcpy(buf + pos, PB_FULL,  3); pos += 3; }
    for (int i = 0; i < empty;  i++) { memcpy(buf + pos, PB_EMPTY, 3); pos += 3; }

    int pct = (int)(ratio * 100.0f + 0.5f);
    pos += snprintf(buf + pos, sizeof(buf) - pos, " %3d%%", pct);
    buf[pos] = '\0';

    pa_widget_set_text(s->wid, buf);
}

/* =========================================================================
 * Sparkline text generation
 * ========================================================================= */

/* Unicode bar chars ▁▂▃▄▅▆▇█ */
static const char *SPARK_CHARS[8] = {
    "\xe2\x96\x81", /* ▁ */
    "\xe2\x96\x82", /* ▂ */
    "\xe2\x96\x83", /* ▃ */
    "\xe2\x96\x84", /* ▄ */
    "\xe2\x96\x85", /* ▅ */
    "\xe2\x96\x86", /* ▆ */
    "\xe2\x96\x87", /* ▇ */
    "\xe2\x96\x88", /* █ */
};

static void sparkline_refresh(PW_Slot *s)
{
    if (s->u.sparkline.count <= 0) {
        pa_widget_set_text(s->wid, "");
        return;
    }

    float mn = s->u.sparkline.data[0];
    float mx = s->u.sparkline.data[0];
    for (int i = 1; i < s->u.sparkline.count; i++) {
        if (s->u.sparkline.data[i] < mn) mn = s->u.sparkline.data[i];
        if (s->u.sparkline.data[i] > mx) mx = s->u.sparkline.data[i];
    }
    float range = mx - mn;

    static char buf[4096];
    int pos = 0;
    for (int i = 0; i < s->u.sparkline.count && pos + 4 < (int)sizeof(buf); i++) {
        int idx;
        if (range < 1e-6f) {
            idx = 3; /* middle bar for uniform data */
        } else {
            float norm = (s->u.sparkline.data[i] - mn) / range;
            idx = (int)(norm * 7.0f + 0.5f);
            if (idx < 0) idx = 0;
            if (idx > 7) idx = 7;
        }
        memcpy(buf + pos, SPARK_CHARS[idx], 3);
        pos += 3;
    }
    buf[pos] = '\0';
    pa_widget_set_text(s->wid, buf);
}

/* =========================================================================
 * Digits — 3-row big-font
 * =========================================================================
 *
 * Each digit is 5 visual columns wide (3 bytes each col, box-drawing chars).
 * Layout: top_row \n mid_row \n bot_row
 *
 * Pattern per digit (5 chars per row):
 *
 *   ┌─┐   ╶┐   ─┐   ─┐   ╷ ╷   ┌─   ┌─   ─┐   ┌─┐   ┌─┐
 *   │ │    │  ┌┘  ─┤  └─┤  └─┤   └─┤    │  ├─┤  │ │
 *   └─┘   ╵   ─┘   ─┘    ╵  └─┘  └─┘   ╵   └─┘   ─┘
 *
 * Box-drawing chars used (UTF-8):
 *   ─  U+2500  \xe2\x94\x80
 *   │  U+2502  \xe2\x94\x82
 *   ┌  U+250C  \xe2\x94\x8c
 *   ┐  U+2510  \xe2\x94\x90
 *   └  U+2514  \xe2\x94\x94
 *   ┘  U+2518  \xe2\x94\x98
 *   ├  U+251C  \xe2\x94\x9c
 *   ┤  U+2524  \xe2\x94\xa4
 *   ╶  U+2576  \xe2\x95\xb6
 *   ╷  U+2577  \xe2\x95\xb7
 *   ╴  U+2574  \xe2\x95\xb4
 *   ╵  U+2575  \xe2\x95\xb5
 *      space   0x20 (1 byte, but we use 1 cell → pad to 3 bytes with spaces)
 *
 * Each row-string for a digit is 5 UTF-8 chars = 5*3 bytes + NUL = 16 bytes
 * We store each row as a fixed 15-byte string (5 chars × 3 bytes each).
 * ========================================================================= */

#define D_HORIZ   "\xe2\x94\x80"  /* ─ */
#define D_VERT    "\xe2\x94\x82"  /* │ */
#define D_TL      "\xe2\x94\x8c"  /* ┌ */
#define D_TR      "\xe2\x94\x90"  /* ┐ */
#define D_BL      "\xe2\x94\x94"  /* └ */
#define D_BR      "\xe2\x94\x98"  /* ┘ */
#define D_ML      "\xe2\x94\x9c"  /* ├ */
#define D_MR      "\xe2\x94\xa4"  /* ┤ */
#define D_HL      "\xe2\x95\xb6"  /* ╶ */
#define D_HT      "\xe2\x95\xb7"  /* ╷ */
#define D_HR      "\xe2\x95\xb4"  /* ╴ */
#define D_HB      "\xe2\x95\xb5"  /* ╵ */
#define D_SP      " "             /* 1 ASCII space (1 cell) */

/* DIG_ROWS[digit][row0..2] — 3 chars × 3 bytes each = 9 bytes (rows 0/2),
 * middle row uses D_SP (1 byte) so 7 bytes; use strlen() to get actual len. */
static const char *DIG_ROWS[10][3] = {
    /* 0 */ { D_TL D_HORIZ D_TR,  D_VERT D_SP D_VERT,   D_BL D_HORIZ D_BR },
    /* 1 */ { D_SP D_HL D_HT,     D_SP D_SP D_VERT,     D_SP D_HB D_HB   },
    /* 2 */ { D_HL D_HORIZ D_TR,  D_HL D_HORIZ D_MR,    D_BL D_HORIZ D_HR },
    /* 3 */ { D_HL D_HORIZ D_TR,  D_HR D_HORIZ D_MR,    D_HR D_HORIZ D_BR },
    /* 4 */ { D_HT D_SP D_HT,     D_BL D_HORIZ D_MR,    D_SP D_SP D_HB   },
    /* 5 */ { D_TL D_HORIZ D_HR,  D_ML D_HORIZ D_HR,    D_HR D_HORIZ D_BR },
    /* 6 */ { D_TL D_HORIZ D_HR,  D_ML D_HORIZ D_TR,    D_BL D_HORIZ D_BR },
    /* 7 */ { D_HL D_HORIZ D_TR,  D_SP D_SP D_VERT,     D_SP D_SP D_HB   },
    /* 8 */ { D_TL D_HORIZ D_TR,  D_ML D_HORIZ D_MR,    D_BL D_HORIZ D_BR },
    /* 9 */ { D_TL D_HORIZ D_TR,  D_BL D_HORIZ D_MR,    D_SP D_SP D_BR   },
};

/* Special chars: ':', '.', ',', '+', '-', ' ' */
static const char *SPC_ROWS[6][3] = {
    /* ':' */ { D_SP D_HT D_SP,  D_SP D_SP D_SP,  D_SP D_HB D_SP },
    /* '.' */ { D_SP D_SP D_SP,  D_SP D_SP D_SP,  D_SP D_HB D_SP },
    /* ',' */ { D_SP D_SP D_SP,  D_SP D_SP D_SP,  D_SP D_HB D_SP },
    /* '+' */ { D_SP D_HT D_SP,  D_HL D_HORIZ D_HR, D_SP D_HB D_SP },
    /* '-' */ { D_SP D_SP D_SP,  D_HL D_HORIZ D_HR, D_SP D_SP D_SP },
    /* ' ' */ { D_SP D_SP D_SP,  D_SP D_SP D_SP,  D_SP D_SP D_SP },
};

static int char_to_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    return -1; /* not a digit */
}

/* Map a non-digit char to a SPC_ROWS index. Returns -1 if unknown (use space). */
static int char_to_special(char c)
{
    switch (c) {
        case ':': return 0;
        case '.': return 1;
        case ',': return 2;
        case '+': return 3;
        case '-': return 4;
        default:  return 5; /* space */
    }
}

#define GAP "  " /* 2-byte gap between digits (2 spaces = 2 cells) */

static void digits_refresh(PW_Slot *s, const char *text)
{
    if (!s) return;
    /* Build 3 row strings, then join with \n */
    static char rows[3][4096];
    int rpos[3] = {0, 0, 0};

    for (int ci = 0; text[ci] != '\0'; ci++) {
        char c = text[ci];
        const char *char_rows[3];
        int d = char_to_digit(c);
        if (d >= 0) {
            char_rows[0] = DIG_ROWS[d][0];
            char_rows[1] = DIG_ROWS[d][1];
            char_rows[2] = DIG_ROWS[d][2];
        } else {
            int sp = char_to_special(c);
            char_rows[0] = SPC_ROWS[sp][0];
            char_rows[1] = SPC_ROWS[sp][1];
            char_rows[2] = SPC_ROWS[sp][2];
        }

        for (int r = 0; r < 3; r++) {
            int len = (int)strlen(char_rows[r]);
            if (rpos[r] + len + 2 < (int)sizeof(rows[r])) {
                memcpy(rows[r] + rpos[r], char_rows[r], len);
                rpos[r] += len;
                /* gap between chars */
                memcpy(rows[r] + rpos[r], GAP, 2);
                rpos[r] += 2;
            }
        }
    }

    /* Terminate rows */
    for (int r = 0; r < 3; r++) rows[r][rpos[r]] = '\0';

    /* Combine: row0 \n row1 \n row2 */
    static char buf[4096];
    snprintf(buf, sizeof(buf), "%s\n%s\n%s", rows[0], rows[1], rows[2]);
    pa_widget_set_text(s->wid, buf);
}

/* =========================================================================
 * LoadingIndicator — braille spinner
 * ========================================================================= */

/* ⣾⣽⣻⢿⡿⣟⣯⣷ */
static const char *SPINNER_FRAMES[8] = {
    "\xe2\xa3\xbe", /* ⣾ */
    "\xe2\xa3\xbd", /* ⣽ */
    "\xe2\xa3\xbb", /* ⣻ */
    "\xe2\xa2\xbf", /* ⢿ */
    "\xe2\xa1\xbf", /* ⡿ */
    "\xe2\xa3\x9f", /* ⣟ */
    "\xe2\xa3\xaf", /* ⣯ */
    "\xe2\xa3\xb7", /* ⣷ */
};

#define LOADING_FRAME_MS 100  /* ms per frame */

static void loading_refresh(PW_Slot *s)
{
    pa_widget_set_text(s->wid, SPINNER_FRAMES[s->u.loading.frame]);
}

/* =========================================================================
 * Public API — ContentSwitcher
 * ========================================================================= */

int pw_switcher_create(int parent_wid)
{
    int wid = pa_widget_add("ContentSwitcher", "", "", parent_wid);
    if (wid < 0) return -1;

    PW_Slot *s = slot_alloc(PW_TYPE_SWITCHER, wid);
    if (!s) return -1;
    s->u.switcher.count  = 0;
    s->u.switcher.active = -1;
    return wid;
}

int pw_switcher_add(int switcher_wid, int child_wid)
{
    PW_Slot *s = slot_find(switcher_wid);
    if (!s || s->u.switcher.count >= PW_SWITCHER_MAX) return -1;

    int idx = s->u.switcher.count;
    s->u.switcher.child_wids[idx] = child_wid;
    s->u.switcher.count++;

    if (idx == 0) {
        pa_widget_set_visible(child_wid, 1);
        s->u.switcher.active = 0;
    } else {
        pa_widget_set_visible(child_wid, 0);
    }
    return idx;
}

void pw_switcher_show(int switcher_wid, int index)
{
    PW_Slot *s = slot_find(switcher_wid);
    if (!s || index < 0 || index >= s->u.switcher.count) return;

    for (int i = 0; i < s->u.switcher.count; i++)
        pa_widget_set_visible(s->u.switcher.child_wids[i], (i == index) ? 1 : 0);

    s->u.switcher.active = index;
}

int pw_switcher_active(int switcher_wid)
{
    PW_Slot *s = slot_find(switcher_wid);
    return s ? s->u.switcher.active : -1;
}

int pw_switcher_count(int switcher_wid)
{
    PW_Slot *s = slot_find(switcher_wid);
    return s ? s->u.switcher.count : 0;
}

/* =========================================================================
 * Public API — Toast
 * ========================================================================= */

int pw_toast_create(const char *message, int duration_ms, int parent_wid)
{
    int wid = pa_widget_add("Toast", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_overlay(wid, 1);
    pa_widget_set_dock(wid, PL_DOCK_BOTTOM);
    pa_widget_set_size(wid,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FIXED,    1 });
    pa_widget_set_text(wid, message ? message : "");

    PW_Slot *s = slot_alloc(PW_TYPE_TOAST, wid);
    if (!s) return -1;
    s->u.toast.duration_ms = (duration_ms > 0) ? duration_ms : 3000;
    s->u.toast.elapsed_ms  = 0;
    s->u.toast.visible     = 1;
    return wid;
}

void pw_toast_show(int wid, const char *message, int duration_ms)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    pa_widget_set_text(wid, message ? message : "");
    pa_widget_set_visible(wid, 1);
    s->u.toast.duration_ms = (duration_ms > 0) ? duration_ms : 3000;
    s->u.toast.elapsed_ms  = 0;
    s->u.toast.visible     = 1;
}

void pw_toast_tick(int wid, int elapsed_ms)
{
    PW_Slot *s = slot_find(wid);
    if (!s || !s->u.toast.visible) return;
    s->u.toast.elapsed_ms += elapsed_ms;
    if (s->u.toast.elapsed_ms >= s->u.toast.duration_ms) {
        s->u.toast.visible = 0;
        pa_widget_set_visible(wid, 0);
    }
}

int pw_toast_is_visible(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.toast.visible : 0;
}

/* =========================================================================
 * Default TCSS
 * ========================================================================= */

static const char DFLT_TCSS[] =
    "Static    { color: white; }\n"
    "Label     { color: white; }\n"
    "Label.primary   { color: #5294e2; }\n"
    "Label.secondary { color: #a0a0a0; }\n"
    "Label.success   { color: #2ecc71; }\n"
    "Label.warning   { color: #f39c12; }\n"
    "Label.error     { color: #e74c3c; }\n"
    "Rule      { color: #555577; }\n"
    "ProgressBar { color: #5294e2; background: #1a1a2e; }\n"
    "Sparkline  { color: #2ecc71; }\n"
    "Digits     { color: #e8e8ff; }\n"
    "LoadingIndicator { color: #5294e2; }\n"
    "Placeholder { background: #2a2a4a; color: #888899; }\n"
    "Header { background: #1a1a2e; color: #c8c8ff; bold: true; }\n"
    "Footer { background: #1a1a2e; color: #888899; }\n"
    "ToggleButton { color: white; }\n"
    "Checkbox { color: white; }\n"
    "Switch { color: #5294e2; }\n"
    "Button { color: white; background: #2a2a4a; }\n"
    "Button.primary { color: white; background: #5294e2; }\n"
    "Button.success { color: #1a1a2e; background: #2ecc71; }\n"
    "Button.warning { color: #1a1a2e; background: #f39c12; }\n"
    "Button.error   { color: white; background: #e74c3c; }\n"
    "RadioButton { color: white; }\n"
    "RadioSet    { background: transparent; }\n"
    "OptionList  { color: white; background: #1e1e30; }\n"
    "ListView    { color: white; background: #1e1e30; }\n"
    "SelectionList { color: white; background: #1e1e30; }\n"
    "ContentSwitcher { background: transparent; }\n"
    "Toast { color: #1a1a2e; background: #c8c8ff; bold: true; }\n";

/* Placeholder colours cycling (7 entries) */
static const char *PH_CLASSES[7] = {
    "ph0", "ph1", "ph2", "ph3", "ph4", "ph5", "ph6"
};

/* =========================================================================
 * Public API — Static / Label
 * ========================================================================= */

int pw_static_create(const char *text, int parent_wid)
{
    int wid = pa_widget_add("Static", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_text(wid, text ? text : "");
    PW_Slot *s = slot_alloc(PW_TYPE_STATIC, wid);
    (void)s;
    return wid;
}

int pw_label_create(const char *text, const char *variant, int parent_wid)
{
    char classes[64] = "";
    if (variant) {
        snprintf(classes, sizeof(classes), "%s", variant);
    }
    int wid = pa_widget_add("Label", "", classes, parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_text(wid, text ? text : "");
    PW_Slot *s = slot_alloc(PW_TYPE_LABEL, wid);
    (void)s;
    return wid;
}

void pw_label_set_text(int wid, const char *text)
{
    pa_widget_set_text(wid, text ? text : "");
}

/* =========================================================================
 * Public API — Rule
 * ========================================================================= */

int pw_rule_create(const char *line_char, int parent_wid)
{
    int wid = pa_widget_add("Rule", "", "", parent_wid);
    if (wid < 0) return -1;

    PW_Slot *s = slot_alloc(PW_TYPE_RULE, wid);
    if (!s) return -1;

    /* Default thin horizontal line ─ U+2500 */
    const char *ch = (line_char && line_char[0]) ? line_char : "\xe2\x94\x80";
    snprintf(s->u.rule.line_char, sizeof(s->u.rule.line_char), "%s", ch);
    rule_refresh(s);
    return wid;
}

void pw_rule_set_char(int wid, const char *line_char)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    const char *ch = (line_char && line_char[0]) ? line_char : "\xe2\x94\x80";
    snprintf(s->u.rule.line_char, sizeof(s->u.rule.line_char), "%s", ch);
    rule_refresh(s);
}

/* =========================================================================
 * Public API — Placeholder
 * ========================================================================= */

int pw_placeholder_create(const char *label, int parent_wid)
{
    int wid = pa_widget_add("Placeholder", "", "ph0", parent_wid);
    if (wid < 0) return -1;

    /* Show label or fall back to numeric wid */
    if (label && label[0]) {
        pa_widget_set_text(wid, label);
    } else {
        char buf[32];
        snprintf(buf, sizeof(buf), "[%d]", wid);
        pa_widget_set_text(wid, buf);
    }

    PW_Slot *s = slot_alloc(PW_TYPE_PLACEHOLDER, wid);
    if (s) s->u.placeholder.variant_index = 0;
    return wid;
}

void pw_placeholder_set_variant(int wid, int variant_index)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    int idx = variant_index % 7;
    if (idx < 0) idx = 0;
    s->u.placeholder.variant_index = idx;
    /* Update CSS class on the widget — we rebuild the widget's class string */
    /* plague-app does not expose a set_classes; we use a workaround via text only */
    /* The class change is cosmetic; for now record it in the slot. */
    (void)PH_CLASSES[idx]; /* referenced, no-op for now */
}

/* =========================================================================
 * Public API — ProgressBar
 * ========================================================================= */

int pw_progressbar_create(float total, int parent_wid)
{
    int wid = pa_widget_add("ProgressBar", "", "", parent_wid);
    if (wid < 0) return -1;

    PW_Slot *s = slot_alloc(PW_TYPE_PROGRESSBAR, wid);
    if (!s) return -1;
    s->u.progressbar.progress = 0.0f;
    s->u.progressbar.total    = (total > 0.0f) ? total : 100.0f;
    progressbar_refresh(s, 0);
    return wid;
}

void pw_progressbar_set_progress(int wid, float progress)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > s->u.progressbar.total) progress = s->u.progressbar.total;
    s->u.progressbar.progress = progress;
    /* Don't auto-refresh text here — caller should call pw_progressbar_update */
}

void pw_progressbar_update(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    TG_Region reg = pa_widget_region(wid);
    progressbar_refresh(s, reg.width);
}

float pw_progressbar_get_progress(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.progressbar.progress : 0.0f;
}

float pw_progressbar_get_total(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.progressbar.total : 0.0f;
}

/* =========================================================================
 * Public API — Sparkline
 * ========================================================================= */

int pw_sparkline_create(int parent_wid)
{
    int wid = pa_widget_add("Sparkline", "", "", parent_wid);
    if (wid < 0) return -1;
    PW_Slot *s = slot_alloc(PW_TYPE_SPARKLINE, wid);
    if (s) {
        s->u.sparkline.count = 0;
    }
    return wid;
}

void pw_sparkline_set_data(int wid, const float *data, int count)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    if (count < 0) count = 0;
    if (count > PW_MAX_SPARK) count = PW_MAX_SPARK;
    if (count > 0)
        memcpy(s->u.sparkline.data, data, count * sizeof(float));
    s->u.sparkline.count = count;
    sparkline_refresh(s);
}

/* =========================================================================
 * Public API — Digits
 * ========================================================================= */

int pw_digits_create(const char *text, int parent_wid)
{
    int wid = pa_widget_add("Digits", "", "", parent_wid);
    if (wid < 0) return -1;
    PW_Slot *s = slot_alloc(PW_TYPE_DIGITS, wid);
    if (s) {
        digits_refresh(s, text ? text : "");
    } else {
        pa_widget_set_text(wid, text ? text : "");
    }
    return wid;
}

void pw_digits_set_text(int wid, const char *text)
{
    PW_Slot *s = slot_find(wid);
    if (!s) {
        pa_widget_set_text(wid, text ? text : "");
        return;
    }
    digits_refresh(s, text ? text : "");
}

/* =========================================================================
 * Public API — LoadingIndicator
 * ========================================================================= */

int pw_loading_create(int parent_wid)
{
    int wid = pa_widget_add("LoadingIndicator", "", "", parent_wid);
    if (wid < 0) return -1;
    PW_Slot *s = slot_alloc(PW_TYPE_LOADING, wid);
    if (s) {
        s->u.loading.frame    = 0;
        s->u.loading.running  = 1;
        s->u.loading.accum_ms = 0;
        loading_refresh(s);
    }
    return wid;
}

void pw_loading_tick(int wid, int elapsed_ms)
{
    PW_Slot *s = slot_find(wid);
    if (!s || !s->u.loading.running) return;
    s->u.loading.accum_ms += elapsed_ms;
    while (s->u.loading.accum_ms >= LOADING_FRAME_MS) {
        s->u.loading.accum_ms -= LOADING_FRAME_MS;
        s->u.loading.frame = (s->u.loading.frame + 1) % 8;
    }
    loading_refresh(s);
}

void pw_loading_start(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (s) s->u.loading.running = 1;
}

void pw_loading_stop(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (s) s->u.loading.running = 0;
}

/* =========================================================================
 * Header — internal refresh
 * ========================================================================= */

static void header_refresh(PW_Slot *s)
{
    char buf[256];
    if (s->u.header.show_clock) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timebuf[12];
        strftime(timebuf, sizeof(timebuf), "%H:%M:%S", t);
        if (s->u.header.icon[0]) {
            snprintf(buf, sizeof(buf), "%s  %s  %s",
                     s->u.header.icon, s->u.header.title, timebuf);
        } else {
            snprintf(buf, sizeof(buf), "%s  %s",
                     s->u.header.title, timebuf);
        }
    } else {
        if (s->u.header.icon[0]) {
            snprintf(buf, sizeof(buf), "%s  %s",
                     s->u.header.icon, s->u.header.title);
        } else {
            snprintf(buf, sizeof(buf), "%s", s->u.header.title);
        }
    }
    pa_widget_set_text(s->wid, buf);
}

/* =========================================================================
 * Public API — Header
 * ========================================================================= */

int pw_header_create(const char *title, const char *icon, int parent_wid)
{
    int wid = pa_widget_add("Header", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_dock(wid, PL_DOCK_TOP);
    pa_widget_set_size(wid,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FIXED,    1 });

    PW_Slot *s = slot_alloc(PW_TYPE_HEADER, wid);
    if (!s) return -1;
    snprintf(s->u.header.title, sizeof(s->u.header.title),
             "%s", title ? title : "");
    snprintf(s->u.header.icon,  sizeof(s->u.header.icon),
             "%s", icon  ? icon  : "");
    s->u.header.show_clock = 0;
    header_refresh(s);
    return wid;
}

void pw_header_set_title(int wid, const char *title)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    snprintf(s->u.header.title, sizeof(s->u.header.title),
             "%s", title ? title : "");
    header_refresh(s);
}

void pw_header_set_icon(int wid, const char *icon)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    snprintf(s->u.header.icon, sizeof(s->u.header.icon),
             "%s", icon ? icon : "");
    header_refresh(s);
}

void pw_header_set_clock(int wid, int show)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.header.show_clock = show ? 1 : 0;
    header_refresh(s);
}

void pw_header_tick(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || !s->u.header.show_clock) return;
    header_refresh(s);
}

/* =========================================================================
 * Footer — internal refresh
 * ========================================================================= */

static void footer_refresh_internal(PW_Slot *s)
{
    static char buf[1024];
    int pos = 0;
    buf[0] = '\0';
    for (int i = 0; i < s->u.footer.count; i++) {
        int written = snprintf(buf + pos, sizeof(buf) - pos,
                               "  [%s] %s",
                               s->u.footer.key_label[i],
                               s->u.footer.description[i]);
        if (written < 0 || pos + written >= (int)sizeof(buf)) break;
        pos += written;
    }
    pa_widget_set_text(s->wid, buf);
}

/* =========================================================================
 * Public API — Footer
 * ========================================================================= */

int pw_footer_create(int parent_wid)
{
    int wid = pa_widget_add("Footer", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_dock(wid, PL_DOCK_BOTTOM);
    pa_widget_set_size(wid,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FIXED,    1 });

    PW_Slot *s = slot_alloc(PW_TYPE_FOOTER, wid);
    if (!s) return -1;
    s->u.footer.count = 0;
    pa_widget_set_text(wid, "");
    return wid;
}

void pw_footer_add_key(int wid, const char *key_label, const char *description)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->u.footer.count >= PW_FOOTER_MAX_KEYS) return;
    int i = s->u.footer.count;
    snprintf(s->u.footer.key_label[i],  sizeof(s->u.footer.key_label[i]),
             "%s", key_label   ? key_label   : "");
    snprintf(s->u.footer.description[i], sizeof(s->u.footer.description[i]),
             "%s", description ? description : "");
    s->u.footer.count++;
    footer_refresh_internal(s);
}

void pw_footer_clear_keys(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.footer.count = 0;
    pa_widget_set_text(wid, "");
}

void pw_footer_refresh(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    footer_refresh_internal(s);
}

/* =========================================================================
 * ToggleButton / Checkbox — text generation
 * ========================================================================= */

/* ○ U+25CB → \xe2\x97\x8b  ● U+25CF → \xe2\x97\x8f */
#define TOG_OFF "\xe2\x97\x8b"  /* ○ */
#define TOG_ON  "\xe2\x97\x8f"  /* ● */

static void toggle_refresh(PW_Slot *s)
{
    char buf[160];
    const char *indicator = s->u.toggle.checked ? TOG_ON : TOG_OFF;
    snprintf(buf, sizeof(buf), "%s %s", indicator, s->u.toggle.label);
    pa_widget_set_text(s->wid, buf);
}

static void checkbox_refresh(PW_Slot *s)
{
    char buf[160];
    const char *indicator = s->u.toggle.checked ? "[X]" : "[ ]";
    snprintf(buf, sizeof(buf), "%s %s", indicator, s->u.toggle.label);
    pa_widget_set_text(s->wid, buf);
}

/* =========================================================================
 * Switch — text generation
 * =========================================================================
 *  OFF: ○╺━━   U+25CB U+257A U+2501 U+2501
 *  ON:  ━━╸●   U+2501 U+2501 U+2578 U+25CF
 * ========================================================================= */

#define SW_CIRCLE_OFF "\xe2\x97\x8b"  /* ○ */
#define SW_CIRCLE_ON  "\xe2\x97\x8f"  /* ● */
#define SW_RIGHT      "\xe2\x95\xba"  /* ╺ */
#define SW_LEFT       "\xe2\x95\xb8"  /* ╸ */
#define SW_BAR        "\xe2\x94\x81"  /* ━ */

#define SW_TEXT_OFF  SW_CIRCLE_OFF SW_RIGHT SW_BAR SW_BAR
#define SW_TEXT_ON   SW_BAR SW_BAR SW_LEFT SW_CIRCLE_ON

static void switch_refresh(PW_Slot *s)
{
    pa_widget_set_text(s->wid, s->u.sw.active ? SW_TEXT_ON : SW_TEXT_OFF);
}

/* =========================================================================
 * Public API — ToggleButton
 * ========================================================================= */

int pw_toggle_create(const char *label, int checked, int parent_wid)
{
    int wid = pa_widget_add("ToggleButton", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_focusable(wid, 1);

    PW_Slot *s = slot_alloc(PW_TYPE_TOGGLE, wid);
    if (!s) return -1;
    snprintf(s->u.toggle.label, sizeof(s->u.toggle.label),
             "%s", label ? label : "");
    s->u.toggle.checked = checked ? 1 : 0;
    toggle_refresh(s);
    return wid;
}

void pw_toggle_set_checked(int wid, int checked)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.toggle.checked = checked ? 1 : 0;
    toggle_refresh(s);
}

int pw_toggle_get_checked(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.toggle.checked : 0;
}

void pw_toggle_set_label(int wid, const char *label)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    snprintf(s->u.toggle.label, sizeof(s->u.toggle.label),
             "%s", label ? label : "");
    toggle_refresh(s);
}

void pw_toggle_toggle(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.toggle.checked = !s->u.toggle.checked;
    toggle_refresh(s);
}

/* =========================================================================
 * Public API — Checkbox
 * ========================================================================= */

int pw_checkbox_create(const char *label, int checked, int parent_wid)
{
    int wid = pa_widget_add("Checkbox", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_focusable(wid, 1);

    PW_Slot *s = slot_alloc(PW_TYPE_CHECKBOX, wid);
    if (!s) return -1;
    snprintf(s->u.toggle.label, sizeof(s->u.toggle.label),
             "%s", label ? label : "");
    s->u.toggle.checked = checked ? 1 : 0;
    checkbox_refresh(s);
    return wid;
}

void pw_checkbox_set_checked(int wid, int checked)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.toggle.checked = checked ? 1 : 0;
    checkbox_refresh(s);
}

int pw_checkbox_get_checked(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.toggle.checked : 0;
}

void pw_checkbox_set_label(int wid, const char *label)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    snprintf(s->u.toggle.label, sizeof(s->u.toggle.label),
             "%s", label ? label : "");
    checkbox_refresh(s);
}

void pw_checkbox_toggle(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.toggle.checked = !s->u.toggle.checked;
    checkbox_refresh(s);
}

/* =========================================================================
 * Public API — Switch
 * ========================================================================= */

int pw_switch_create(int active, int parent_wid)
{
    int wid = pa_widget_add("Switch", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_focusable(wid, 1);
    /* Switch is always 4 visual columns wide, 1 row tall */
    pa_widget_set_size(wid,
        (PL_SizeValue){ PL_SIZE_FIXED, 4 },
        (PL_SizeValue){ PL_SIZE_FIXED, 1 });

    PW_Slot *s = slot_alloc(PW_TYPE_SWITCH, wid);
    if (!s) return -1;
    s->u.sw.active = active ? 1 : 0;
    switch_refresh(s);
    return wid;
}

void pw_switch_set_active(int wid, int active)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.sw.active = active ? 1 : 0;
    switch_refresh(s);
}

int pw_switch_get_active(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.sw.active : 0;
}

void pw_switch_toggle(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.sw.active = !s->u.sw.active;
    switch_refresh(s);
}

/* =========================================================================
 * Public API — Button
 * ========================================================================= */

int pw_button_create(const char *label, const char *variant, int parent_wid)
{
    char classes[64] = "";
    if (variant && variant[0])
        snprintf(classes, sizeof(classes), "%s", variant);

    int wid = pa_widget_add("Button", "", classes, parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_focusable(wid, 1);

    PW_Slot *s = slot_alloc(PW_TYPE_BUTTON, wid);
    if (!s) return -1;
    snprintf(s->u.button.label,   sizeof(s->u.button.label),
             "%s", label   ? label   : "");
    snprintf(s->u.button.variant, sizeof(s->u.button.variant),
             "%s", variant ? variant : "");
    pa_widget_set_text(wid, s->u.button.label);
    return wid;
}

void pw_button_set_label(int wid, const char *label)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    snprintf(s->u.button.label, sizeof(s->u.button.label),
             "%s", label ? label : "");
    pa_widget_set_text(wid, s->u.button.label);
}

/* =========================================================================
 * RadioButton — text generation
 * =========================================================================
 *  ◯  U+25EF  \xe2\x97\xaf  (unselected)
 *  ◉  U+25C9  \xe2\x97\x89  (selected)
 * ========================================================================= */

#define RADIO_OFF "\xe2\x97\xaf"  /* ◯ */
#define RADIO_ON  "\xe2\x97\x89"  /* ◉ */

static void radio_refresh(PW_Slot *s)
{
    char buf[140];
    const char *indicator = s->u.toggle.checked ? RADIO_ON : RADIO_OFF;
    snprintf(buf, sizeof(buf), "%s %s", indicator, s->u.toggle.label);
    pa_widget_set_text(s->wid, buf);
}

/* =========================================================================
 * List helpers — shared by OptionList, ListView, SelectionList
 * =========================================================================
 *  OptionList cursor:     ▸ item   U+25B8  \xe2\x96\xb8
 *  ListView   cursor:     ▶ item   U+25B6  \xe2\x96\xb6
 *  SelectionList cursor:  ▸ [X] item
 *  Normal rows:           "  " (two spaces, same visual width as indicators)
 * ========================================================================= */

#define LIST_CUR_OPT  "\xe2\x96\xb8 "   /* ▸  */
#define LIST_CUR_LIST "\xe2\x96\xb6 "   /* ▶  */
#define LIST_NORM     "  "

static void list_rebuild_text(PW_Slot *s)
{
    static char buf[4096];
    int pos = 0;
    buf[0] = '\0';

    for (int i = 0; i < s->u.list.count; i++) {
        int is_cur = (i == s->u.list.cursor);

        if (s->widget_type == PW_TYPE_SELECTIONLIST) {
            const char *cp  = is_cur ? LIST_CUR_OPT : LIST_NORM;
            const char *chk = s->u.list.checked[i] ? "[X]" : "[ ]";
            int w = snprintf(buf + pos, sizeof(buf) - pos,
                             "%s%s %s\n", cp, chk, s->u.list.items[i]);
            if (w > 0 && pos + w < (int)sizeof(buf)) pos += w;
        } else if (s->widget_type == PW_TYPE_LISTVIEW) {
            const char *cp = is_cur ? LIST_CUR_LIST : LIST_NORM;
            int w = snprintf(buf + pos, sizeof(buf) - pos,
                             "%s%s\n", cp, s->u.list.items[i]);
            if (w > 0 && pos + w < (int)sizeof(buf)) pos += w;
        } else { /* OptionList */
            const char *cp = is_cur ? LIST_CUR_OPT : LIST_NORM;
            int w = snprintf(buf + pos, sizeof(buf) - pos,
                             "%s%s\n", cp, s->u.list.items[i]);
            if (w > 0 && pos + w < (int)sizeof(buf)) pos += w;
        }
    }

    /* Trim trailing newline */
    if (pos > 0 && buf[pos - 1] == '\n') buf[--pos] = '\0';

    pa_widget_set_text(s->wid, buf);

    /* Scroll to keep cursor in view (2 lines of context above) */
    int scroll_y = s->u.list.cursor > 2 ? s->u.list.cursor - 2 : 0;
    pa_widget_scroll_to(s->wid, 0, scroll_y);
}

static int list_add_item(PW_Slot *s, const char *text, int checked)
{
    if (s->u.list.count >= PW_MAX_OPTIONS) return -1;
    int i = s->u.list.count;
    snprintf(s->u.list.items[i], PW_OPTION_TEXT_LEN, "%s", text ? text : "");
    s->u.list.checked[i] = (uint8_t)(checked ? 1 : 0);
    s->u.list.count++;
    list_rebuild_text(s);
    return i;
}

static void list_set_cursor(PW_Slot *s, int index)
{
    if (s->u.list.count == 0) { s->u.list.cursor = 0; return; }
    if (index < 0) index = 0;
    if (index >= s->u.list.count) index = s->u.list.count - 1;
    s->u.list.cursor = index;
    list_rebuild_text(s);
}

/* =========================================================================
 * Public API — RadioButton
 * ========================================================================= */

int pw_radio_create(const char *label, int checked, int parent_wid)
{
    int wid = pa_widget_add("RadioButton", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_focusable(wid, 1);

    PW_Slot *s = slot_alloc(PW_TYPE_RADIO, wid);
    if (!s) return -1;
    snprintf(s->u.toggle.label, sizeof(s->u.toggle.label),
             "%s", label ? label : "");
    s->u.toggle.checked = checked ? 1 : 0;
    radio_refresh(s);
    return wid;
}

void pw_radio_set_checked(int wid, int checked)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.toggle.checked = checked ? 1 : 0;
    radio_refresh(s);
}

int pw_radio_get_checked(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.toggle.checked : 0;
}

/* =========================================================================
 * Public API — RadioSet
 * ========================================================================= */

int pw_radioset_create(int parent_wid)
{
    int wid = pa_widget_add("RadioSet", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_layout(wid, PL_LAYOUT_VERTICAL);

    PW_Slot *s = slot_alloc(PW_TYPE_RADIOSET, wid);
    if (!s) return -1;
    s->u.radioset.count    = 0;
    s->u.radioset.selected = -1;
    return wid;
}

int pw_radioset_add(int set_wid, const char *label)
{
    PW_Slot *s = slot_find(set_wid);
    if (!s || s->u.radioset.count >= PW_RADIOSET_MAX) return -1;

    int radio_wid = pw_radio_create(label, 0, set_wid);
    if (radio_wid < 0) return -1;

    int idx = s->u.radioset.count;
    s->u.radioset.radio_wids[idx] = radio_wid;
    s->u.radioset.count++;

    /* Auto-select first radio added */
    if (s->u.radioset.selected < 0)
        pw_radioset_select(set_wid, 0);

    return radio_wid;
}

void pw_radioset_select(int set_wid, int index)
{
    PW_Slot *s = slot_find(set_wid);
    if (!s || index < 0 || index >= s->u.radioset.count) return;
    s->u.radioset.selected = index;
    for (int i = 0; i < s->u.radioset.count; i++) {
        PW_Slot *rs = slot_find(s->u.radioset.radio_wids[i]);
        if (rs) {
            rs->u.toggle.checked = (i == index) ? 1 : 0;
            radio_refresh(rs);
        }
    }
}

int pw_radioset_get_selected(int set_wid)
{
    PW_Slot *s = slot_find(set_wid);
    return s ? s->u.radioset.selected : -1;
}

int pw_radioset_count(int set_wid)
{
    PW_Slot *s = slot_find(set_wid);
    return s ? s->u.radioset.count : 0;
}

/* =========================================================================
 * Public API — OptionList
 * ========================================================================= */

int pw_optionlist_create(int parent_wid)
{
    int wid = pa_widget_add("OptionList", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_focusable(wid, 1);

    PW_Slot *s = slot_alloc(PW_TYPE_OPTIONLIST, wid);
    if (!s) return -1;
    s->u.list.count  = 0;
    s->u.list.cursor = 0;
    return wid;
}

int pw_optionlist_add_option(int wid, const char *text)
{
    PW_Slot *s = slot_find(wid);
    return s ? list_add_item(s, text, 0) : -1;
}

void pw_optionlist_clear(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.list.count  = 0;
    s->u.list.cursor = 0;
    pa_widget_set_text(wid, "");
    pa_widget_scroll_to(wid, 0, 0);
}

void pw_optionlist_set_cursor(int wid, int index)
{
    PW_Slot *s = slot_find(wid);
    if (s) list_set_cursor(s, index);
}

int pw_optionlist_get_cursor(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.list.cursor : 0;
}

void pw_optionlist_cursor_next(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->u.list.count == 0) return;
    int next = s->u.list.cursor + 1;
    if (next >= s->u.list.count) next = s->u.list.count - 1;
    list_set_cursor(s, next);
}

void pw_optionlist_cursor_prev(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->u.list.count == 0) return;
    int prev = s->u.list.cursor - 1;
    if (prev < 0) prev = 0;
    list_set_cursor(s, prev);
}

int pw_optionlist_count(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.list.count : 0;
}

/* =========================================================================
 * Public API — ListView
 * ========================================================================= */

int pw_listview_create(int parent_wid)
{
    int wid = pa_widget_add("ListView", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_focusable(wid, 1);

    PW_Slot *s = slot_alloc(PW_TYPE_LISTVIEW, wid);
    if (!s) return -1;
    s->u.list.count  = 0;
    s->u.list.cursor = 0;
    return wid;
}

int pw_listview_add_item(int wid, const char *text)
{
    PW_Slot *s = slot_find(wid);
    return s ? list_add_item(s, text, 0) : -1;
}

void pw_listview_clear(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.list.count  = 0;
    s->u.list.cursor = 0;
    pa_widget_set_text(wid, "");
    pa_widget_scroll_to(wid, 0, 0);
}

void pw_listview_set_cursor(int wid, int index)
{
    PW_Slot *s = slot_find(wid);
    if (s) list_set_cursor(s, index);
}

int pw_listview_get_cursor(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.list.cursor : 0;
}

void pw_listview_cursor_next(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->u.list.count == 0) return;
    int next = s->u.list.cursor + 1;
    if (next >= s->u.list.count) next = s->u.list.count - 1;
    list_set_cursor(s, next);
}

void pw_listview_cursor_prev(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->u.list.count == 0) return;
    int prev = s->u.list.cursor - 1;
    if (prev < 0) prev = 0;
    list_set_cursor(s, prev);
}

int pw_listview_count(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.list.count : 0;
}

/* =========================================================================
 * Public API — SelectionList
 * ========================================================================= */

int pw_selectionlist_create(int parent_wid)
{
    int wid = pa_widget_add("SelectionList", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_focusable(wid, 1);

    PW_Slot *s = slot_alloc(PW_TYPE_SELECTIONLIST, wid);
    if (!s) return -1;
    s->u.list.count  = 0;
    s->u.list.cursor = 0;
    return wid;
}

int pw_selectionlist_add_option(int wid, const char *text, int initially_selected)
{
    PW_Slot *s = slot_find(wid);
    return s ? list_add_item(s, text, initially_selected) : -1;
}

void pw_selectionlist_clear(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s) return;
    s->u.list.count  = 0;
    s->u.list.cursor = 0;
    pa_widget_set_text(wid, "");
    pa_widget_scroll_to(wid, 0, 0);
}

void pw_selectionlist_set_cursor(int wid, int index)
{
    PW_Slot *s = slot_find(wid);
    if (s) list_set_cursor(s, index);
}

int pw_selectionlist_get_cursor(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.list.cursor : 0;
}

void pw_selectionlist_cursor_next(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->u.list.count == 0) return;
    int next = s->u.list.cursor + 1;
    if (next >= s->u.list.count) next = s->u.list.count - 1;
    list_set_cursor(s, next);
}

void pw_selectionlist_cursor_prev(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->u.list.count == 0) return;
    int prev = s->u.list.cursor - 1;
    if (prev < 0) prev = 0;
    list_set_cursor(s, prev);
}

void pw_selectionlist_toggle_selection(int wid, int index)
{
    PW_Slot *s = slot_find(wid);
    if (!s || index < 0 || index >= s->u.list.count) return;
    s->u.list.checked[index] = !s->u.list.checked[index];
    list_rebuild_text(s);
}

void pw_selectionlist_set_selected(int wid, int index, int selected)
{
    PW_Slot *s = slot_find(wid);
    if (!s || index < 0 || index >= s->u.list.count) return;
    s->u.list.checked[index] = (uint8_t)(selected ? 1 : 0);
    list_rebuild_text(s);
}

int pw_selectionlist_is_selected(int wid, int index)
{
    PW_Slot *s = slot_find(wid);
    if (!s || index < 0 || index >= s->u.list.count) return 0;
    return s->u.list.checked[index] ? 1 : 0;
}

int pw_selectionlist_count(int wid)
{
    PW_Slot *s = slot_find(wid);
    return s ? s->u.list.count : 0;
}

/* =========================================================================
 * Default TCSS
 * ========================================================================= */

const char *pw_default_tcss(void)
{
    return DFLT_TCSS;
}
