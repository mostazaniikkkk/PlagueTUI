#include "../include/plague_widgets.h"
#include "plague_app.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#ifdef _WIN32
#  include <windows.h>
#endif

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
    PW_TYPE_TOAST,
    PW_TYPE_INPUT,
    PW_TYPE_COLLAPSIBLE,
    PW_TYPE_TABS,
    PW_TYPE_TABBEDCONTENT,
    PW_TYPE_TREE,
    PW_TYPE_LOG,
    PW_TYPE_RICHLOG,
    PW_TYPE_TEXTAREA,
    PW_TYPE_MARKDOWN,
    PW_TYPE_DATATABLE,
    PW_TYPE_WELCOME
} PW_Type;

#define PW_MAX_SLOTS       128
#define PW_MAX_SPARK       256   /* max data points for sparkline */
#define PW_FOOTER_MAX_KEYS  16
#define PW_MAX_OPTIONS      48   /* max items per list widget */
#define PW_OPTION_TEXT_LEN  48   /* max chars per list item */
#define PW_RADIOSET_MAX     16   /* max RadioButtons per RadioSet */
#define PW_SWITCHER_MAX     16   /* max panels per ContentSwitcher */
#define PW_INPUT_MAX_LEN   255   /* max chars in an Input field */
#define PW_TABS_MAX         16   /* max tabs per Tabs / TabbedContent */
#define PW_TREE_MAX         48   /* max nodes per Tree */
#define PW_LOG_MAX_LINES   128   /* max lines in Log / RichLog */
#define PW_LOG_LINE_LEN    128   /* max chars per log line */
#define PW_TEXTAREA_MAX_LINES 64
#define PW_TEXTAREA_LINE_LEN  256
#define PW_MARKDOWN_TEXT_LEN  4096

/* DataTable limits — cell data kept in separate pool to avoid bloating PW_Slot */
#define PW_DT_MAX_TABLES   4
#define PW_DT_MAX_COLS    12
#define PW_DT_MAX_ROWS   128
#define PW_DT_HDR_LEN     24
#define PW_DT_CELL_LEN    20

#define PW_DT_CURSOR_NONE   0
#define PW_DT_CURSOR_CELL   1
#define PW_DT_CURSOR_ROW    2
#define PW_DT_CURSOR_COLUMN 3

typedef struct {
    char label[64];
    int  parent;        /* -1 = no parent (root) */
    int  first_child;   /* -1 = none */
    int  next_sibling;  /* -1 = none */
    int  depth;
    int  expanded;
    int  is_leaf;
} PW_TreeNode;

/* Internal key codes (same as pe_dispatch_key expects) */
#define PW_KEY_UP     0x0100
#define PW_KEY_DOWN   0x0101
#define PW_KEY_LEFT   0x0102
#define PW_KEY_RIGHT  0x0103
#define PW_KEY_HOME   0x0104
#define PW_KEY_END    0x0105
#define PW_KEY_PGUP   0x0106
#define PW_KEY_PGDN   0x0107
#define PW_KEY_DELETE 0x007F
#define PW_KEY_BS     0x0008
#define PW_KEY_ENTER  0x000D
#define PW_KEY_CTRL_A 0x0001
#define PW_KEY_CTRL_V 0x0016

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
            int     bid_click; /* auto-registered click binding */
        } list;

        /* RadioSet — manages child RadioButton wids */
        struct {
            int radio_wids [PW_RADIOSET_MAX];
            int bid_radio  [PW_RADIOSET_MAX]; /* click bid per child radio button */
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

        /* Input */
        struct {
            char value[PW_INPUT_MAX_LEN + 1];
            char placeholder[128];
            int  cursor;       /* byte offset of cursor in value */
            int  max_len;      /* 0 = use PW_INPUT_MAX_LEN */
            int  password;     /* 1 = show • instead of chars */
            int  submitted;    /* set to 1 when Enter is pressed */
            int  bid_print_lo; /* bid for space (0x20), printable range base */
            int  bid_backspace;
            int  bid_delete;
            int  bid_left;
            int  bid_right;
            int  bid_home;
            int  bid_end;
            int  bid_enter;
            int  bid_ctrl_a;
            int  bid_ctrl_v;
            int  blink_on;      /* 1 = cursor visible, 0 = hidden */
            int  blink_ms;      /* accumulated ms toward next blink toggle */
            int  was_focused;   /* focus state on last tick (for edge detection) */
        } input;

        /* Collapsible */
        struct {
            char title[128];
            int  collapsed;
            int  header_wid;  /* clickable title widget */
            int  body_wid;    /* container shown/hidden */
        } collapsible;

        /* Tabs */
        struct {
            char labels[PW_TABS_MAX][64];
            int  tab_wids[PW_TABS_MAX];  /* individual Static widget per tab */
            int  bid[PW_TABS_MAX];       /* click bid per tab */
            int  count;
            int  active;
            int  bar_wid;               /* horizontal container */
        } tabs;

        /* TabbedContent */
        struct {
            int  tabs_wid;      /* child Tabs widget */
            int  switcher_wid;  /* child ContentSwitcher */
            int  count;
        } tabbedcontent;

        /* Tree */
        struct {
            PW_TreeNode nodes[PW_TREE_MAX];
            int         count;
            int         cursor;
            int         visible[PW_TREE_MAX];
            int         visible_count;
        } tree;

        /* Log / RichLog (same struct, type discriminates) */
        struct {
            char lines[PW_LOG_MAX_LINES][PW_LOG_LINE_LEN];
            int  count;       /* total lines written (wraps at PW_LOG_MAX_LINES) */
            int  head;        /* index of oldest line (ring buffer) */
            int  scroll_top;  /* first visible line (user scroll) */
            int  bid_scroll;  /* auto-registered scroll binding */
        } log;

        /* TextArea */
        struct {
            char lines[PW_TEXTAREA_MAX_LINES][PW_TEXTAREA_LINE_LEN];
            int  line_count;
            int  cursor_line;
            int  cursor_col;     /* byte offset within line */
            int  scroll_top;     /* first visible line */
            int  bid_print_lo;
            int  bid_backspace, bid_delete;
            int  bid_left, bid_right, bid_up, bid_down;
            int  bid_home, bid_end;
            int  bid_enter;
            int  bid_ctrl_a;
            int  blink_on;
            int  blink_ms;
            int  was_focused;
        } textarea;

        /* Markdown */
        struct {
            char raw[PW_MARKDOWN_TEXT_LEN];
        } markdown;

        /* DataTable — full cell data lives in g_dt_pool[dt_idx] */
        struct {
            int dt_idx;
        } datatable;

        /* Welcome — composite of Markdown + Button */
        struct {
            int md_wid;
            int btn_wid;
        } welcome;
    } u;
} PW_Slot;

/* =========================================================================
 * DataTable cell-data pool (kept outside PW_Slot to avoid 100 KB union)
 * ========================================================================= */

typedef struct {
    int  used;
    int  wid;
    char col_keys  [PW_DT_MAX_COLS][PW_DT_HDR_LEN];
    int  col_widths[PW_DT_MAX_COLS];   /* 0 = auto */
    int  col_count;
    char cells[PW_DT_MAX_ROWS][PW_DT_MAX_COLS][PW_DT_CELL_LEN];
    int  row_count;
    /* Cursor / scroll */
    int  cursor_row, cursor_col;
    int  scroll_row, scroll_col;
    /* Config */
    int  cursor_type;
    int  show_header;
    int  show_cursor;
    int  zebra_stripes;
    /* Interaction state */
    int  selected;
    /* Key bindings */
    int  bid_up,   bid_down;
    int  bid_left, bid_right;
    int  bid_home, bid_end;
    int  bid_pgup, bid_pgdn;
    int  bid_enter;
    int  bid_click;
} PW_DtData;

static PW_DtData g_dt_pool[PW_DT_MAX_TABLES];

static PW_Slot g_pool[PW_MAX_SLOTS];
static int     g_initialised = 0;

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

void pw_init(void)
{
    memset(g_pool,    0, sizeof(g_pool));
    memset(g_dt_pool, 0, sizeof(g_dt_pool));
    g_initialised = 1;
}

void pw_shutdown(void)
{
    if (!g_initialised) return;
    memset(g_pool,    0, sizeof(g_pool));
    memset(g_dt_pool, 0, sizeof(g_dt_pool));
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
    "Toast { color: #1a1a2e; background: #c8c8ff; bold: true; }\n"
    "Input { color: white; background: #1e1e30; }\n"
    "Collapsible { color: white; }\n"
    "Tab { color: #a0a0a0; background: #1a1a2e; }\n"
    "Tree     { color: white; background: #1e1e30; }\n"
    "Log      { color: #d0d0d0; background: #0d0d1a; }\n"
    "RichLog  { color: #d0d0d0; background: #0d0d1a; }\n"
    "TextArea { color: white;   background: #1e1e30; }\n";

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
    s->u.radioset.bid_radio[idx]  = pa_bind_click(radio_wid);
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
    s->u.list.count     = 0;
    s->u.list.cursor    = 0;
    s->u.list.bid_click = pa_bind_click(wid);
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
    s->u.list.count     = 0;
    s->u.list.cursor    = 0;
    s->u.list.bid_click = pa_bind_click(wid);
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
    s->u.list.count     = 0;
    s->u.list.cursor    = 0;
    s->u.list.bid_click = pa_bind_click(wid);
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
 * Input — private helpers
 * ========================================================================= */

/* ▌ U+258C → \xe2\x96\x8c  (left half block — cursor marker) */
/* •  U+2022 → \xe2\x80\xa2  (bullet — password mask) */

static void input_refresh(PW_Slot *s)
{
    /* buf must hold: PW_INPUT_MAX_LEN × 3 bytes (password •) + 3 (cursor) + 1 */
    static char buf[PW_INPUT_MAX_LEN * 3 + 4];
    int len     = (int)strlen(s->u.input.value);
    int focused = (pa_focus_get() == s->wid);
    int show_cursor = focused && s->u.input.blink_on;
    int pos = 0;

    if (len == 0) {
        if (focused) {
            /* Focused + empty: cursor visible or invisible depending on blink,
             * but NEVER show placeholder */
            if (show_cursor) {
                buf[pos++] = '\xe2'; buf[pos++] = '\x96'; buf[pos++] = '\x8c';
            }
            buf[pos] = '\0';
        } else {
            /* Not focused + empty: show placeholder */
            snprintf(buf, sizeof(buf), "%s", s->u.input.placeholder);
        }
    } else {
        /* Insert cursor between chars at cursor position */
        for (int i = 0; i <= len && pos + 6 < (int)sizeof(buf); i++) {
            if (i == s->u.input.cursor && show_cursor) {
                buf[pos++] = '\xe2'; buf[pos++] = '\x96'; buf[pos++] = '\x8c';
            }
            if (i < len) {
                if (s->u.input.password) {
                    buf[pos++] = '\xe2'; buf[pos++] = '\x80'; buf[pos++] = '\xa2';
                } else {
                    buf[pos++] = s->u.input.value[i];
                }
            }
        }
        buf[pos] = '\0';
    }
    pa_widget_set_text(s->wid, buf);
}

static void input_insert_char(PW_Slot *s, char ch)
{
    int len = (int)strlen(s->u.input.value);
    int cap = s->u.input.max_len > 0 ? s->u.input.max_len : PW_INPUT_MAX_LEN;
    if (len >= cap) return;
    memmove(s->u.input.value + s->u.input.cursor + 1,
            s->u.input.value + s->u.input.cursor,
            (size_t)(len - s->u.input.cursor + 1));
    s->u.input.value[s->u.input.cursor] = ch;
    s->u.input.cursor++;
}

static void input_backspace(PW_Slot *s)
{
    if (s->u.input.cursor == 0) return;
    int len = (int)strlen(s->u.input.value);
    memmove(s->u.input.value + s->u.input.cursor - 1,
            s->u.input.value + s->u.input.cursor,
            (size_t)(len - s->u.input.cursor + 1));
    s->u.input.cursor--;
}

static void input_delete_fwd(PW_Slot *s)
{
    int len = (int)strlen(s->u.input.value);
    if (s->u.input.cursor >= len) return;
    memmove(s->u.input.value + s->u.input.cursor,
            s->u.input.value + s->u.input.cursor + 1,
            (size_t)(len - s->u.input.cursor));
}

static void input_paste(PW_Slot *s)
{
#ifdef _WIN32
    if (!OpenClipboard(NULL)) return;
    HANDLE h = GetClipboardData(CF_TEXT);
    if (h) {
        const char *text = (const char *)GlobalLock(h);
        if (text) {
            for (int i = 0; text[i] && text[i] != '\r' && text[i] != '\n'; i++)
                input_insert_char(s, text[i]);
            GlobalUnlock(h);
        }
    }
    CloseClipboard();
#else
    (void)s;
#endif
}

/* =========================================================================
 * Input — public API
 * ========================================================================= */

int pw_input_create(const char *placeholder, int max_len, int parent_wid)
{
    int wid = pa_widget_add("Input", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_size(wid,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FIXED,    1 });
    pa_widget_set_focusable(wid, 1);

    PW_Slot *s = slot_alloc(PW_TYPE_INPUT, wid);
    if (!s) return wid;

    s->u.input.cursor    = 0;
    s->u.input.max_len   = max_len;
    s->u.input.password  = 0;
    s->u.input.submitted = 0;
    s->u.input.bid_print_lo = 0;
    s->u.input.blink_on  = 1;
    s->u.input.blink_ms  = 0;
    s->u.input.value[0]  = '\0';
    strncpy(s->u.input.placeholder, placeholder ? placeholder : "", 127);
    s->u.input.placeholder[127] = '\0';

    input_refresh(s);
    return wid;
}

void pw_input_register_keys(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_INPUT) return;

    /* Printable ASCII 0x20..0x7E registered consecutively so they form a
     * contiguous bid range: bid_print_lo + (key - 0x20). */
    s->u.input.bid_print_lo = pa_bind_key(wid, 0x20, 0);
    for (int k = 0x21; k <= 0x7E; k++) pa_bind_key(wid, k, 0);

    s->u.input.bid_backspace = pa_bind_key(wid, PW_KEY_BS,     0);
    s->u.input.bid_delete    = pa_bind_key(wid, PW_KEY_DELETE, 0);
    s->u.input.bid_left      = pa_bind_key(wid, PW_KEY_LEFT,   0);
    s->u.input.bid_right     = pa_bind_key(wid, PW_KEY_RIGHT,  0);
    s->u.input.bid_home      = pa_bind_key(wid, PW_KEY_HOME,   0);
    s->u.input.bid_end       = pa_bind_key(wid, PW_KEY_END,    0);
    s->u.input.bid_enter     = pa_bind_key(wid, PW_KEY_ENTER,  0);
    s->u.input.bid_ctrl_a    = pa_bind_key(wid, PW_KEY_CTRL_A, 0);
    s->u.input.bid_ctrl_v    = pa_bind_key(wid, PW_KEY_CTRL_V, 0);
}

int pw_input_handle(int wid, int bid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_INPUT || bid <= 0) return 0;

    /* Printable character range */
    if (s->u.input.bid_print_lo > 0) {
        int hi = s->u.input.bid_print_lo + (0x7E - 0x20);
        if (bid >= s->u.input.bid_print_lo && bid <= hi) {
            int ch = 0x20 + (bid - s->u.input.bid_print_lo);
            input_insert_char(s, (char)ch);
            input_refresh(s);
            return 1;
        }
    }

    if (s->u.input.bid_backspace && bid == s->u.input.bid_backspace) {
        input_backspace(s); input_refresh(s); return 1;
    }
    if (s->u.input.bid_delete && bid == s->u.input.bid_delete) {
        input_delete_fwd(s); input_refresh(s); return 1;
    }
    if (s->u.input.bid_left && bid == s->u.input.bid_left) {
        if (s->u.input.cursor > 0) s->u.input.cursor--;
        input_refresh(s); return 1;
    }
    if (s->u.input.bid_right && bid == s->u.input.bid_right) {
        int len = (int)strlen(s->u.input.value);
        if (s->u.input.cursor < len) s->u.input.cursor++;
        input_refresh(s); return 1;
    }
    if (s->u.input.bid_home && bid == s->u.input.bid_home) {
        s->u.input.cursor = 0; input_refresh(s); return 1;
    }
    if (s->u.input.bid_end && bid == s->u.input.bid_end) {
        s->u.input.cursor = (int)strlen(s->u.input.value);
        input_refresh(s); return 1;
    }
    if (s->u.input.bid_enter && bid == s->u.input.bid_enter) {
        s->u.input.submitted = 1; return 1;
    }
    if (s->u.input.bid_ctrl_a && bid == s->u.input.bid_ctrl_a) {
        s->u.input.cursor = (int)strlen(s->u.input.value);
        input_refresh(s); return 1;
    }
    if (s->u.input.bid_ctrl_v && bid == s->u.input.bid_ctrl_v) {
        input_paste(s); input_refresh(s); return 1;
    }
    return 0;
}

const char *pw_input_get_value(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_INPUT) return "";
    return s->u.input.value;
}

void pw_input_set_value(int wid, const char *text)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_INPUT) return;
    strncpy(s->u.input.value, text ? text : "", PW_INPUT_MAX_LEN);
    s->u.input.value[PW_INPUT_MAX_LEN] = '\0';
    s->u.input.cursor = (int)strlen(s->u.input.value);
    input_refresh(s);
}

int pw_input_is_submitted(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_INPUT) return 0;
    if (s->u.input.submitted) { s->u.input.submitted = 0; return 1; }
    return 0;
}

void pw_input_set_password(int wid, int on)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_INPUT) return;
    s->u.input.password = on ? 1 : 0;
    input_refresh(s);
}

void pw_input_tick(int wid, int elapsed_ms)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_INPUT) return;

    int focused = (pa_focus_get() == s->wid);

    if (focused && !s->u.input.was_focused) {
        /* Just gained focus: show cursor immediately, reset blink */
        s->u.input.blink_on  = 1;
        s->u.input.blink_ms  = 0;
        s->u.input.was_focused = 1;
        input_refresh(s);
        return;
    }

    if (!focused && s->u.input.was_focused) {
        /* Just lost focus: hide cursor immediately */
        s->u.input.blink_on    = 0;
        s->u.input.blink_ms    = 0;
        s->u.input.was_focused = 0;
        input_refresh(s);
        return;
    }

    if (!focused) return;

    /* Already focused: advance blink every 500 ms */
    s->u.input.blink_ms += elapsed_ms;
    if (s->u.input.blink_ms >= 500) {
        s->u.input.blink_ms = 0;
        s->u.input.blink_on = !s->u.input.blink_on;
        input_refresh(s);
    }
}

/* =========================================================================
 * Collapsible
 * ========================================================================= */

static void collapsible_refresh_header(PW_Slot *s)
{
    static char buf[144];
    const char *sym = s->u.collapsible.collapsed ? "\xe2\x96\xb6 " : "\xe2\x96\xbc ";
    snprintf(buf, sizeof(buf), "%s%s", sym, s->u.collapsible.title);
    pa_widget_set_text(s->u.collapsible.header_wid, buf);
}

int pw_collapsible_create(const char *title, int parent_wid)
{
    int outer = pa_widget_add("Collapsible", "", "", parent_wid);
    if (outer < 0) return -1;
    pa_widget_set_size(outer,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 });
    pa_widget_set_layout(outer, PL_LAYOUT_VERTICAL);

    int header = pa_widget_add("Static", "", "collapsible-header", outer);
    pa_widget_set_size(header,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FIXED,    1 });
    pa_widget_set_focusable(header, 1);

    int body = pa_widget_add("container", "", "collapsible-body", outer);
    pa_widget_set_size(body,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 });
    pa_widget_set_layout(body, PL_LAYOUT_VERTICAL);

    PW_Slot *s = slot_alloc(PW_TYPE_COLLAPSIBLE, outer);
    if (!s) return outer;
    strncpy(s->u.collapsible.title, title ? title : "", 127);
    s->u.collapsible.title[127] = '\0';
    s->u.collapsible.collapsed  = 0;
    s->u.collapsible.header_wid = header;
    s->u.collapsible.body_wid   = body;

    collapsible_refresh_header(s);
    return outer;
}

int pw_collapsible_header_wid(int wid)
{
    PW_Slot *s = slot_find(wid);
    return (s && s->widget_type == PW_TYPE_COLLAPSIBLE) ? s->u.collapsible.header_wid : -1;
}

int pw_collapsible_content(int wid)
{
    PW_Slot *s = slot_find(wid);
    return (s && s->widget_type == PW_TYPE_COLLAPSIBLE) ? s->u.collapsible.body_wid : -1;
}

void pw_collapsible_toggle(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_COLLAPSIBLE) return;
    s->u.collapsible.collapsed = !s->u.collapsible.collapsed;
    pa_widget_set_visible(s->u.collapsible.body_wid,
                          s->u.collapsible.collapsed ? 0 : 1);
    collapsible_refresh_header(s);
}

int pw_collapsible_is_collapsed(int wid)
{
    PW_Slot *s = slot_find(wid);
    return (s && s->widget_type == PW_TYPE_COLLAPSIBLE) ? s->u.collapsible.collapsed : 0;
}

/* =========================================================================
 * Tabs
 * ========================================================================= */

static void tabs_refresh(PW_Slot *s)
{
    static char buf[80];
    for (int i = 0; i < s->u.tabs.count; i++) {
        if (i == s->u.tabs.active)
            snprintf(buf, sizeof(buf), "[%s]", s->u.tabs.labels[i]);
        else
            snprintf(buf, sizeof(buf), " %s ", s->u.tabs.labels[i]);
        pa_widget_set_text(s->u.tabs.tab_wids[i], buf);
    }
}

int pw_tabs_create(int parent_wid)
{
    int bar = pa_widget_add("Tab", "", "", parent_wid);
    if (bar < 0) return -1;
    pa_widget_set_size(bar,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FIXED,    1 });
    pa_widget_set_layout(bar, PL_LAYOUT_HORIZONTAL);

    PW_Slot *s = slot_alloc(PW_TYPE_TABS, bar);
    if (!s) return bar;
    s->u.tabs.count   = 0;
    s->u.tabs.active  = 0;
    s->u.tabs.bar_wid = bar;
    return bar;
}

int pw_tabs_add(int wid, const char *label)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABS) return -1;
    if (s->u.tabs.count >= PW_TABS_MAX) return -1;

    int idx = s->u.tabs.count;
    strncpy(s->u.tabs.labels[idx], label ? label : "", 63);
    s->u.tabs.labels[idx][63] = '\0';

    int tw = pa_widget_add("Static", "", "tab", s->u.tabs.bar_wid);
    /* Width: label length + 2 (brackets/spaces) */
    int w = (int)strlen(s->u.tabs.labels[idx]) + 2;
    pa_widget_set_size(tw,
        (PL_SizeValue){ PL_SIZE_FIXED, w },
        (PL_SizeValue){ PL_SIZE_FIXED, 1 });
    pa_widget_set_focusable(tw, 1);

    s->u.tabs.tab_wids[idx] = tw;
    s->u.tabs.bid[idx]      = 0; /* registered later via pw_tabs_register_clicks */
    s->u.tabs.count++;

    tabs_refresh(s);
    return idx;
}

void pw_tabs_set_active(int wid, int index)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABS) return;
    if (index < 0 || index >= s->u.tabs.count) return;
    s->u.tabs.active = index;
    tabs_refresh(s);
}

void pw_tabs_register_clicks(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABS) return;
    for (int i = 0; i < s->u.tabs.count; i++)
        s->u.tabs.bid[i] = pa_bind_click(s->u.tabs.tab_wids[i]);
}

int pw_tabs_handle_click(int wid, int bid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABS || bid <= 0) return 0;
    for (int i = 0; i < s->u.tabs.count; i++) {
        if (s->u.tabs.bid[i] == bid) {
            pw_tabs_set_active(wid, i);
            return 1;
        }
    }
    return 0;
}

int pw_tabs_get_active(int wid)
{
    PW_Slot *s = slot_find(wid);
    return (s && s->widget_type == PW_TYPE_TABS) ? s->u.tabs.active : 0;
}

void pw_tabs_next(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABS || s->u.tabs.count == 0) return;
    pw_tabs_set_active(wid, (s->u.tabs.active + 1) % s->u.tabs.count);
}

void pw_tabs_prev(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABS || s->u.tabs.count == 0) return;
    pw_tabs_set_active(wid, (s->u.tabs.active - 1 + s->u.tabs.count) % s->u.tabs.count);
}

int pw_tabs_count(int wid)
{
    PW_Slot *s = slot_find(wid);
    return (s && s->widget_type == PW_TYPE_TABS) ? s->u.tabs.count : 0;
}

/* =========================================================================
 * TabbedContent
 * ========================================================================= */

int pw_tabbedcontent_create(int parent_wid)
{
    int outer = pa_widget_add("container", "", "tabbedcontent", parent_wid);
    if (outer < 0) return -1;
    pa_widget_set_size(outer,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 });
    pa_widget_set_layout(outer, PL_LAYOUT_VERTICAL);

    int tabs_wid     = pw_tabs_create(outer);
    int switcher_wid = pw_switcher_create(outer);
    pa_widget_set_size(switcher_wid,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 });

    PW_Slot *s = slot_alloc(PW_TYPE_TABBEDCONTENT, outer);
    if (!s) return outer;
    s->u.tabbedcontent.tabs_wid     = tabs_wid;
    s->u.tabbedcontent.switcher_wid = switcher_wid;
    s->u.tabbedcontent.count        = 0;
    return outer;
}

int pw_tabbedcontent_add_pane(int wid, const char *label)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABBEDCONTENT) return -1;

    pw_tabs_add(s->u.tabbedcontent.tabs_wid, label);

    int pane = pa_widget_add("container", "", "tabpane", s->u.tabbedcontent.switcher_wid);
    pa_widget_set_size(pane,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 });
    pa_widget_set_layout(pane, PL_LAYOUT_VERTICAL);

    pw_switcher_add(s->u.tabbedcontent.switcher_wid, pane);
    s->u.tabbedcontent.count++;
    return pane;
}

void pw_tabbedcontent_register_clicks(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABBEDCONTENT) return;
    pw_tabs_register_clicks(s->u.tabbedcontent.tabs_wid);
}

int pw_tabbedcontent_handle_click(int wid, int bid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABBEDCONTENT) return 0;
    if (!pw_tabs_handle_click(s->u.tabbedcontent.tabs_wid, bid)) return 0;
    int idx = pw_tabs_get_active(s->u.tabbedcontent.tabs_wid);
    pw_switcher_show(s->u.tabbedcontent.switcher_wid, idx);
    return 1;
}

void pw_tabbedcontent_set_active(int wid, int index)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABBEDCONTENT) return;
    pw_tabs_set_active(s->u.tabbedcontent.tabs_wid, index);
    pw_switcher_show(s->u.tabbedcontent.switcher_wid, index);
}

int pw_tabbedcontent_get_active(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TABBEDCONTENT) return 0;
    return pw_tabs_get_active(s->u.tabbedcontent.tabs_wid);
}

/* =========================================================================
 * Tree
 * ========================================================================= */

static void tree_build_visible(PW_Slot *s);
static void tree_refresh(PW_Slot *s);

static void tree_dfs(PW_Slot *s, int node_idx)
{
    if (s->u.tree.visible_count >= PW_TREE_MAX) return;
    s->u.tree.visible[s->u.tree.visible_count++] = node_idx;
    if (!s->u.tree.nodes[node_idx].is_leaf && s->u.tree.nodes[node_idx].expanded) {
        int child = s->u.tree.nodes[node_idx].first_child;
        while (child != -1 && s->u.tree.visible_count < PW_TREE_MAX) {
            tree_dfs(s, child);
            child = s->u.tree.nodes[child].next_sibling;
        }
    }
}

static void tree_build_visible(PW_Slot *s)
{
    s->u.tree.visible_count = 0;
    /* Visit every root node (parent == -1) in insertion order */
    for (int i = 0; i < s->u.tree.count; i++) {
        if (s->u.tree.nodes[i].parent == -1)
            tree_dfs(s, i);
    }
}

static void tree_refresh(PW_Slot *s)
{
    static char buf[PW_TREE_MAX * 80];
    int pos = 0;

    for (int vi = 0; vi < s->u.tree.visible_count; vi++) {
        int ni = s->u.tree.visible[vi];
        PW_TreeNode *n = &s->u.tree.nodes[ni];

        /* Cursor marker: ▶ for selected row, space otherwise */
        if (vi == s->u.tree.cursor) {
            /* ▶ U+25B6 → \xe2\x96\xb6 */
            buf[pos++] = '\xe2'; buf[pos++] = '\x96'; buf[pos++] = '\xb6';
            buf[pos++] = ' ';
        } else {
            buf[pos++] = ' '; buf[pos++] = ' ';
        }

        /* Indentation */
        for (int d = 0; d < n->depth; d++) {
            buf[pos++] = ' '; buf[pos++] = ' ';
        }

        /* Node symbol */
        if (!n->is_leaf) {
            if (n->expanded) {
                /* ▼ U+25BC → \xe2\x96\xbc */
                buf[pos++] = '\xe2'; buf[pos++] = '\x96'; buf[pos++] = '\xbc';
            } else {
                /* ▶ U+25B6 → \xe2\x96\xb6 */
                buf[pos++] = '\xe2'; buf[pos++] = '\x96'; buf[pos++] = '\xb6';
            }
            buf[pos++] = ' ';
        }

        /* Label */
        int llen = (int)strlen(n->label);
        if (pos + llen + 2 < (int)sizeof(buf)) {
            memcpy(buf + pos, n->label, (size_t)llen);
            pos += llen;
        }

        buf[pos++] = '\n';
    }
    if (pos > 0 && buf[pos-1] == '\n') pos--; /* trim trailing newline */
    buf[pos] = '\0';
    pa_widget_set_text(s->wid, buf);
}

int pw_tree_create(int parent_wid)
{
    int wid = pa_widget_add("Tree", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_size(wid,
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
        (PL_SizeValue){ PL_SIZE_FRACTION, 1 });
    pa_widget_set_focusable(wid, 1);

    PW_Slot *s = slot_alloc(PW_TYPE_TREE, wid);
    if (!s) return wid;
    s->u.tree.count         = 0;
    s->u.tree.cursor        = 0;
    s->u.tree.visible_count = 0;
    return wid;
}

int pw_tree_add_node(int wid, int parent_idx, const char *label, int is_leaf)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TREE) return -1;
    if (s->u.tree.count >= PW_TREE_MAX) return -1;

    int idx = s->u.tree.count++;
    PW_TreeNode *n = &s->u.tree.nodes[idx];
    strncpy(n->label, label ? label : "", 63);
    n->label[63]   = '\0';
    n->parent       = parent_idx;
    n->first_child  = -1;
    n->next_sibling = -1;
    n->expanded     = 0;
    n->is_leaf      = is_leaf;
    n->depth        = (parent_idx < 0) ? 0 : s->u.tree.nodes[parent_idx].depth + 1;

    /* Attach to parent's child list */
    if (parent_idx >= 0 && parent_idx < idx) {
        PW_TreeNode *p = &s->u.tree.nodes[parent_idx];
        if (p->first_child == -1) {
            p->first_child = idx;
        } else {
            int sib = p->first_child;
            while (s->u.tree.nodes[sib].next_sibling != -1)
                sib = s->u.tree.nodes[sib].next_sibling;
            s->u.tree.nodes[sib].next_sibling = idx;
        }
    }

    tree_build_visible(s);
    tree_refresh(s);
    return idx;
}

void pw_tree_expand(int wid, int node_idx)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TREE) return;
    if (node_idx < 0 || node_idx >= s->u.tree.count) return;
    s->u.tree.nodes[node_idx].expanded = 1;
    tree_build_visible(s);
    tree_refresh(s);
}

void pw_tree_collapse(int wid, int node_idx)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TREE) return;
    if (node_idx < 0 || node_idx >= s->u.tree.count) return;
    s->u.tree.nodes[node_idx].expanded = 0;
    tree_build_visible(s);
    tree_refresh(s);
}

void pw_tree_toggle(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TREE) return;
    if (s->u.tree.visible_count == 0) return;
    int ni = s->u.tree.visible[s->u.tree.cursor];
    if (s->u.tree.nodes[ni].is_leaf) return;
    s->u.tree.nodes[ni].expanded = !s->u.tree.nodes[ni].expanded;
    /* Keep cursor valid after collapse */
    tree_build_visible(s);
    if (s->u.tree.cursor >= s->u.tree.visible_count)
        s->u.tree.cursor = s->u.tree.visible_count - 1;
    tree_refresh(s);
}

void pw_tree_cursor_next(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TREE) return;
    if (s->u.tree.cursor < s->u.tree.visible_count - 1) s->u.tree.cursor++;
    tree_refresh(s);
}

void pw_tree_cursor_prev(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TREE) return;
    if (s->u.tree.cursor > 0) s->u.tree.cursor--;
    tree_refresh(s);
}

int pw_tree_get_cursor(int wid)
{
    PW_Slot *s = slot_find(wid);
    return (s && s->widget_type == PW_TYPE_TREE) ? s->u.tree.cursor : 0;
}

int pw_tree_get_cursor_node(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TREE) return -1;
    if (s->u.tree.visible_count == 0) return -1;
    return s->u.tree.visible[s->u.tree.cursor];
}

void pw_tree_set_cursor(int wid, int visible_idx)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TREE) return;
    if (visible_idx < 0 || visible_idx >= s->u.tree.visible_count) return;
    s->u.tree.cursor = visible_idx;
    tree_refresh(s);
}

int pw_tree_click_row(int wid, int mouse_y)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TREE) return 0;
    TG_Region r = pa_widget_region(wid);
    int row = mouse_y - r.y;
    if (row < 0 || row >= s->u.tree.visible_count) return 0;
    s->u.tree.cursor = row;
    /* Toggle expand/collapse on click if not a leaf */
    int ni = s->u.tree.visible[row];
    if (!s->u.tree.nodes[ni].is_leaf) {
        s->u.tree.nodes[ni].expanded = !s->u.tree.nodes[ni].expanded;
        tree_build_visible(s);
        if (s->u.tree.cursor >= s->u.tree.visible_count)
            s->u.tree.cursor = s->u.tree.visible_count - 1;
    }
    tree_refresh(s);
    return 1;
}

/* =========================================================================
 * Log / RichLog
 * ========================================================================= */

/* Strip Textual-style markup tags like [red], [/red], [bold], etc. */
/* =========================================================================
 * Log colorization — inline escapes: \x01RRGGBB (set fg) / \x02 (reset)
 * ========================================================================= */

static int col_emit(char *buf, int pos, int max, uint8_t r, uint8_t g, uint8_t b)
{
    if (pos + 7 >= max) return pos;
    static const char hex[] = "0123456789ABCDEF";
    buf[pos++] = '\x01';
    buf[pos++] = hex[r >> 4]; buf[pos++] = hex[r & 0xF];
    buf[pos++] = hex[g >> 4]; buf[pos++] = hex[g & 0xF];
    buf[pos++] = hex[b >> 4]; buf[pos++] = hex[b & 0xF];
    return pos;
}

static int col_reset(char *buf, int pos, int max)
{
    if (pos + 1 >= max) return pos;
    buf[pos++] = '\x02';
    return pos;
}

static int col_text(char *buf, int pos, int max, const char *text, int len)
{
    if (len < 0) len = (int)strlen(text);
    if (pos + len >= max) len = max - pos - 1;
    if (len <= 0) return pos;
    memcpy(buf + pos, text, (size_t)len);
    return pos + len;
}

typedef enum { LV_NONE, LV_INFO, LV_WARN, LV_ERR, LV_DEBUG } LvType;

static LvType detect_level(const char *s, int len)
{
    char tmp[16]; int n = (len < 15) ? len : 15;
    for (int i = 0; i < n; i++)
        tmp[i] = (s[i] >= 'a' && s[i] <= 'z') ? (char)(s[i] - 32) : s[i];
    tmp[n] = '\0';
    if (!strcmp(tmp,"INFO")  || !strcmp(tmp,"INF"))                          return LV_INFO;
    if (!strcmp(tmp,"WARN")  || !strcmp(tmp,"WARNING") || !strcmp(tmp,"WRN")) return LV_WARN;
    if (!strcmp(tmp,"ERR")   || !strcmp(tmp,"ERROR")   || !strcmp(tmp,"FATAL") ||
        !strcmp(tmp,"CRIT")  || !strcmp(tmp,"CRITICAL"))                     return LV_ERR;
    if (!strcmp(tmp,"DEBUG") || !strcmp(tmp,"DBG")     || !strcmp(tmp,"TRACE")) return LV_DEBUG;
    return LV_NONE;
}

static int is_datetime_str(const char *s, int len)
{
    int has_digit = 0, has_sep = 0;
    for (int i = 0; i < len; i++) {
        if (s[i] >= '0' && s[i] <= '9') has_digit = 1;
        if (s[i] == '-' || s[i] == '/' || s[i] == 'T' || s[i] == ':' || s[i] == ' ') has_sep = 1;
    }
    return has_digit && has_sep && len >= 5;
}

static void level_rgb(LvType lv, uint8_t *r, uint8_t *g, uint8_t *b)
{
    switch (lv) {
    case LV_INFO:  *r=80;  *g=200; *b=80;  break;
    case LV_WARN:  *r=220; *g=200; *b=0;   break;
    case LV_ERR:   *r=220; *g=60;  *b=60;  break;
    case LV_DEBUG: *r=140; *g=140; *b=180; break;
    default:       *r=200; *g=200; *b=200; break;
    }
}

static int richlog_named_color(const char *name, int len, uint8_t *r, uint8_t *g, uint8_t *b)
{
    char tmp[32]; int n = (len < 31) ? len : 31;
    memcpy(tmp, name, (size_t)n); tmp[n] = '\0';
    if (!strcmp(tmp,"green"))   { *r=80;  *g=200; *b=80;  return 1; }
    if (!strcmp(tmp,"yellow"))  { *r=220; *g=200; *b=0;   return 1; }
    if (!strcmp(tmp,"red"))     { *r=220; *g=60;  *b=60;  return 1; }
    if (!strcmp(tmp,"cyan"))    { *r=0;   *g=200; *b=220; return 1; }
    if (!strcmp(tmp,"blue"))    { *r=100; *g=140; *b=220; return 1; }
    if (!strcmp(tmp,"magenta")) { *r=200; *g=80;  *b=200; return 1; }
    if (!strcmp(tmp,"white"))   { *r=220; *g=220; *b=220; return 1; }
    if (!strcmp(tmp,"orange"))  { *r=220; *g=140; *b=60;  return 1; }
    return 0;
}

/* Coloriza una línea emitiendo escapes inline.
   is_richlog=1: también maneja [color]text[/color] markup explícito. */
static void log_colorize(const char *src, char *dst, int dst_len, int is_richlog)
{
    int si = 0, di = 0;

    while (src[si] && di < dst_len - 2) {
        char c = src[si];

        /* ── RichLog: [color]content[/color] ─────────────────────────── */
        if (is_richlog && c == '[' && src[si+1] && src[si+1] != '/') {
            int k = si + 1;
            while (src[k] && src[k] != ']') k++;
            if (src[k] == ']') {
                int tag_len = k - (si + 1);
                uint8_t r, g, b;
                if (richlog_named_color(src + si + 1, tag_len, &r, &g, &b)) {
                    di = col_emit(dst, di, dst_len, r, g, b);
                    si = k + 1; /* saltar [tag] completo */
                    while (src[si] && di < dst_len - 2) {
                        if (src[si] == '[' && src[si+1] == '/') {
                            /* saltar [/tag] completo */
                            int e = si + 2;
                            while (src[e] && src[e] != ']') e++;
                            if (src[e] == ']') si = e + 1;
                            else dst[di++] = src[si++];
                            break;
                        }
                        dst[di++] = src[si++];
                    }
                    di = col_reset(dst, di, dst_len);
                    continue;
                }
            }
        }

        /* ── Corchetes: level o datetime ──────────────────────────────── */
        if (c == '[') {
            int k = si + 1;
            while (src[k] && src[k] != ']' && src[k] != '\n') k++;
            if (src[k] == ']') {
                const char *ct = src + si + 1;
                int clen = k - (si + 1);
                uint8_t r, g, b;
                LvType lv = detect_level(ct, clen);
                if (lv != LV_NONE) {
                    level_rgb(lv, &r, &g, &b);
                    di = col_emit(dst, di, dst_len, r, g, b);
                    dst[di++] = '['; di = col_text(dst, di, dst_len, ct, clen); dst[di++] = ']';
                    di = col_reset(dst, di, dst_len);
                    si = k + 1; continue;
                }
                if (is_datetime_str(ct, clen)) {
                    di = col_emit(dst, di, dst_len, 80, 200, 220);
                    dst[di++] = '['; di = col_text(dst, di, dst_len, ct, clen); dst[di++] = ']';
                    di = col_reset(dst, di, dst_len);
                    si = k + 1; continue;
                }
            }
            dst[di++] = src[si++]; continue;
        }

        /* ── Paréntesis ────────────────────────────────────────────────── */
        if (c == '(') {
            int k = si + 1;
            while (src[k] && src[k] != ')' && src[k] != '\n') k++;
            if (src[k] == ')') {
                di = col_emit(dst, di, dst_len, 180, 150, 100);
                dst[di++] = '('; di = col_text(dst, di, dst_len, src+si+1, k-(si+1)); dst[di++] = ')';
                di = col_reset(dst, di, dst_len);
                si = k + 1; continue;
            }
            dst[di++] = src[si++]; continue;
        }

        /* ── Llaves ─────────────────────────────────────────────────────── */
        if (c == '{') {
            int k = si + 1;
            while (src[k] && src[k] != '}' && src[k] != '\n') k++;
            if (src[k] == '}') {
                di = col_emit(dst, di, dst_len, 180, 110, 180);
                dst[di++] = '{'; di = col_text(dst, di, dst_len, src+si+1, k-(si+1)); dst[di++] = '}';
                di = col_reset(dst, di, dst_len);
                si = k + 1; continue;
            }
            dst[di++] = src[si++]; continue;
        }

        /* ── Dos puntos ─────────────────────────────────────────────────── */
        if (c == ':') {
            di = col_emit(dst, di, dst_len, 120, 120, 150);
            dst[di++] = ':';
            di = col_reset(dst, di, dst_len);
            si++; continue;
        }

        dst[di++] = src[si++];
    }
    dst[di] = '\0';
}

/* Buffer de salida por línea — incluye espacio para escapes de color */
#define PW_LOG_COLORED_LEN  (PW_LOG_LINE_LEN * 8)

static void log_rebuild_text(PW_Slot *s)
{
    static char buf    [PW_LOG_MAX_LINES * PW_LOG_COLORED_LEN];
    static char colored[PW_LOG_COLORED_LEN];
    int pos = 0;
    int count = s->u.log.count < PW_LOG_MAX_LINES ? s->u.log.count : PW_LOG_MAX_LINES;
    int is_rich = (s->widget_type == PW_TYPE_RICHLOG);

    /* Scroll: respetar scroll_top del usuario, clampear al rango válido */
    TG_Region r = pa_widget_region(s->wid);
    int visible = (r.height > 0) ? r.height : count;
    int max_top = (count > visible) ? (count - visible) : 0;
    if (s->u.log.scroll_top > max_top) s->u.log.scroll_top = max_top;
    if (s->u.log.scroll_top < 0)       s->u.log.scroll_top = 0;
    int start = s->u.log.scroll_top;

    for (int i = start; i < count; i++) {
        int idx = (s->u.log.head + i) % PW_LOG_MAX_LINES;
        if (is_rich) {
            log_colorize(s->u.log.lines[idx], colored, PW_LOG_COLORED_LEN, 1);
            int n = snprintf(buf + pos, (int)sizeof(buf) - pos, "%s\n", colored);
            if (n > 0) pos += n;
        } else {
            int n = snprintf(buf + pos, (int)sizeof(buf) - pos, "%s\n", s->u.log.lines[idx]);
            if (n > 0) pos += n;
        }
    }
    if (pos > 0 && buf[pos - 1] == '\n') buf[pos - 1] = '\0';
    pa_widget_set_text(s->wid, buf);
}

static int log_create_impl(PW_Type type, int parent_wid)
{
    int wid = pa_widget_add(type == PW_TYPE_LOG ? "Log" : "RichLog", "", "", parent_wid);
    if (wid < 0) return -1;
    PW_Slot *s = slot_alloc(type, wid);
    if (!s) return -1;
    s->u.log.count      = 0;
    s->u.log.head       = 0;
    s->u.log.scroll_top = 0;
    s->u.log.bid_scroll = pa_bind_scroll(wid);
    return wid;
}

int pw_log_create(int parent_wid)     { return log_create_impl(PW_TYPE_LOG,     parent_wid); }
int pw_richlog_create(int parent_wid) { return log_create_impl(PW_TYPE_RICHLOG, parent_wid); }

void pw_log_write(int wid, const char *line)
{
    PW_Slot *s = slot_find(wid);
    if (!s || (s->widget_type != PW_TYPE_LOG && s->widget_type != PW_TYPE_RICHLOG)) return;
    int idx;
    if (s->u.log.count < PW_LOG_MAX_LINES) {
        idx = s->u.log.count++;
    } else {
        idx = s->u.log.head;
        s->u.log.head = (s->u.log.head + 1) % PW_LOG_MAX_LINES;
    }
    snprintf(s->u.log.lines[idx], PW_LOG_LINE_LEN, "%s", line);
    /* Auto-scroll al fondo en cada nueva línea */
    s->u.log.scroll_top = s->u.log.count; /* log_rebuild_text lo clampea */
    log_rebuild_text(s);
}

void pw_log_clear(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || (s->widget_type != PW_TYPE_LOG && s->widget_type != PW_TYPE_RICHLOG)) return;
    s->u.log.count = 0;
    s->u.log.head  = 0;
    pa_widget_set_text(s->wid, "");
}

int pw_log_line_count(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || (s->widget_type != PW_TYPE_LOG && s->widget_type != PW_TYPE_RICHLOG)) return 0;
    return s->u.log.count < PW_LOG_MAX_LINES ? s->u.log.count : PW_LOG_MAX_LINES;
}

/* =========================================================================
 * TextArea
 * ========================================================================= */

#define PW_KEY_UP   0x0100
#define PW_KEY_DOWN 0x0101

static void textarea_rebuild_text(PW_Slot *s)
{
    static char buf[PW_TEXTAREA_MAX_LINES * (PW_TEXTAREA_LINE_LEN + 4)];
    int focused = (pa_focus_get() == s->wid);

    /* Ajustar scroll_top para que cursor_line quede visible */
    TG_Region r = pa_widget_region(s->wid);
    int visible_lines = (r.height > 0) ? r.height : 5;
    int cl = s->u.textarea.cursor_line;
    if (cl < s->u.textarea.scroll_top)
        s->u.textarea.scroll_top = cl;
    if (cl >= s->u.textarea.scroll_top + visible_lines)
        s->u.textarea.scroll_top = cl - visible_lines + 1;

    int top  = s->u.textarea.scroll_top;
    int end  = top + visible_lines;
    if (end > s->u.textarea.line_count) end = s->u.textarea.line_count;

    int pos = 0;
    for (int i = top; i < end; i++) {
        const char *ln = s->u.textarea.lines[i];
        int len = (int)strlen(ln);
        if (focused && i == cl) {
            int col = s->u.textarea.cursor_col;
            if (col > len) col = len;
            if (s->u.textarea.blink_on) {
                int n = snprintf(buf + pos, (int)sizeof(buf) - pos,
                                 "%.*s\xe2\x96\x8c%s", col, ln, ln + col);
                if (n > 0) pos += n;
            } else {
                int n = snprintf(buf + pos, (int)sizeof(buf) - pos, "%s", ln);
                if (n > 0) pos += n;
            }
        } else {
            int n = snprintf(buf + pos, (int)sizeof(buf) - pos, "%s", ln);
            if (n > 0) pos += n;
        }
        if (i < end - 1 && pos < (int)sizeof(buf) - 1)
            buf[pos++] = '\n';
    }
    buf[pos] = '\0';
    pa_widget_set_text(s->wid, buf);
}

int pw_textarea_create(int parent_wid)
{
    int wid = pa_widget_add("TextArea", "", "", parent_wid);
    if (wid < 0) return -1;
    PW_Slot *s = slot_alloc(PW_TYPE_TEXTAREA, wid);
    if (!s) return -1;
    pa_widget_set_focusable(wid, 1);
    s->u.textarea.line_count  = 1;
    s->u.textarea.cursor_line = 0;
    s->u.textarea.cursor_col  = 0;
    s->u.textarea.scroll_top  = 0;
    s->u.textarea.blink_on    = 0;
    s->u.textarea.blink_ms    = 0;
    s->u.textarea.was_focused = 0;
    memset(s->u.textarea.lines, 0, sizeof(s->u.textarea.lines));
    return wid;
}

void pw_textarea_register_keys(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TEXTAREA) return;
    s->u.textarea.bid_print_lo  = pa_bind_key(wid, 0x20, 0);
    for (int c = 0x21; c <= 0x7E; c++) pa_bind_key(wid, c, 0);
    s->u.textarea.bid_backspace = pa_bind_key(wid, PW_KEY_BS,     0);
    s->u.textarea.bid_delete    = pa_bind_key(wid, PW_KEY_DELETE, 0);
    s->u.textarea.bid_left      = pa_bind_key(wid, PW_KEY_LEFT,   0);
    s->u.textarea.bid_right     = pa_bind_key(wid, PW_KEY_RIGHT,  0);
    s->u.textarea.bid_up        = pa_bind_key(wid, PW_KEY_UP,     0);
    s->u.textarea.bid_down      = pa_bind_key(wid, PW_KEY_DOWN,   0);
    s->u.textarea.bid_home      = pa_bind_key(wid, PW_KEY_HOME,   0);
    s->u.textarea.bid_end       = pa_bind_key(wid, PW_KEY_END,    0);
    s->u.textarea.bid_enter     = pa_bind_key(wid, PW_KEY_ENTER,  0);
    s->u.textarea.bid_ctrl_a    = pa_bind_key(wid, PW_KEY_CTRL_A, 0);
}

int pw_textarea_handle(int wid, int bid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TEXTAREA) return 0;
    if (s->u.textarea.bid_print_lo <= 0) return 0;

    int cl   = s->u.textarea.cursor_line;
    int cc   = s->u.textarea.cursor_col;
    char *ln = s->u.textarea.lines[cl];
    int   len = (int)strlen(ln);

    int lo = s->u.textarea.bid_print_lo;
    if (bid >= lo && bid <= lo + (0x7E - 0x20)) {
        char ch = (char)(0x20 + (bid - lo));
        if (len < PW_TEXTAREA_LINE_LEN - 1) {
            memmove(ln + cc + 1, ln + cc, len - cc + 1);
            ln[cc] = ch;
            s->u.textarea.cursor_col++;
        }
        textarea_rebuild_text(s); return 1;
    }
    if (bid == s->u.textarea.bid_backspace) {
        if (cc > 0) {
            memmove(ln + cc - 1, ln + cc, len - cc + 1);
            s->u.textarea.cursor_col--;
        } else if (cl > 0) {
            char *prev = s->u.textarea.lines[cl - 1];
            int prev_len = (int)strlen(prev);
            if (prev_len + len < PW_TEXTAREA_LINE_LEN - 1) {
                memcpy(prev + prev_len, ln, len + 1);
                for (int i = cl; i < s->u.textarea.line_count - 1; i++)
                    memcpy(s->u.textarea.lines[i], s->u.textarea.lines[i+1], PW_TEXTAREA_LINE_LEN);
                memset(s->u.textarea.lines[s->u.textarea.line_count-1], 0, PW_TEXTAREA_LINE_LEN);
                s->u.textarea.line_count--;
                s->u.textarea.cursor_line--;
                s->u.textarea.cursor_col = prev_len;
            }
        }
        textarea_rebuild_text(s); return 1;
    }
    if (bid == s->u.textarea.bid_delete) {
        if (cc < len) {
            memmove(ln + cc, ln + cc + 1, len - cc);
        } else if (cl < s->u.textarea.line_count - 1) {
            char *next = s->u.textarea.lines[cl+1];
            int next_len = (int)strlen(next);
            if (len + next_len < PW_TEXTAREA_LINE_LEN - 1) {
                memcpy(ln + len, next, next_len + 1);
                for (int i = cl+1; i < s->u.textarea.line_count - 1; i++)
                    memcpy(s->u.textarea.lines[i], s->u.textarea.lines[i+1], PW_TEXTAREA_LINE_LEN);
                memset(s->u.textarea.lines[s->u.textarea.line_count-1], 0, PW_TEXTAREA_LINE_LEN);
                s->u.textarea.line_count--;
            }
        }
        textarea_rebuild_text(s); return 1;
    }
    if (bid == s->u.textarea.bid_left) {
        if (cc > 0) s->u.textarea.cursor_col--;
        else if (cl > 0) {
            s->u.textarea.cursor_line--;
            s->u.textarea.cursor_col = (int)strlen(s->u.textarea.lines[s->u.textarea.cursor_line]);
        }
        textarea_rebuild_text(s); return 1;
    }
    if (bid == s->u.textarea.bid_right) {
        if (cc < len) s->u.textarea.cursor_col++;
        else if (cl < s->u.textarea.line_count - 1) {
            s->u.textarea.cursor_line++;
            s->u.textarea.cursor_col = 0;
        }
        textarea_rebuild_text(s); return 1;
    }
    if (bid == s->u.textarea.bid_up) {
        if (cl > 0) {
            s->u.textarea.cursor_line--;
            int plen = (int)strlen(s->u.textarea.lines[s->u.textarea.cursor_line]);
            if (s->u.textarea.cursor_col > plen) s->u.textarea.cursor_col = plen;
        }
        textarea_rebuild_text(s); return 1;
    }
    if (bid == s->u.textarea.bid_down) {
        if (cl < s->u.textarea.line_count - 1) {
            s->u.textarea.cursor_line++;
            int nlen = (int)strlen(s->u.textarea.lines[s->u.textarea.cursor_line]);
            if (s->u.textarea.cursor_col > nlen) s->u.textarea.cursor_col = nlen;
        }
        textarea_rebuild_text(s); return 1;
    }
    if (bid == s->u.textarea.bid_home) {
        s->u.textarea.cursor_col = 0;
        textarea_rebuild_text(s); return 1;
    }
    if (bid == s->u.textarea.bid_end) {
        s->u.textarea.cursor_col = len;
        textarea_rebuild_text(s); return 1;
    }
    if (bid == s->u.textarea.bid_enter) {
        if (s->u.textarea.line_count < PW_TEXTAREA_MAX_LINES) {
            char tail[PW_TEXTAREA_LINE_LEN];
            strncpy(tail, ln + cc, PW_TEXTAREA_LINE_LEN - 1);
            tail[PW_TEXTAREA_LINE_LEN - 1] = '\0';
            ln[cc] = '\0';
            for (int i = s->u.textarea.line_count; i > cl + 1; i--)
                memcpy(s->u.textarea.lines[i], s->u.textarea.lines[i-1], PW_TEXTAREA_LINE_LEN);
            snprintf(s->u.textarea.lines[cl+1], PW_TEXTAREA_LINE_LEN, "%s", tail);
            s->u.textarea.line_count++;
            s->u.textarea.cursor_line++;
            s->u.textarea.cursor_col = 0;
        }
        textarea_rebuild_text(s); return 1;
    }
    if (bid == s->u.textarea.bid_ctrl_a) {
        s->u.textarea.cursor_col = 0;
        textarea_rebuild_text(s); return 1;
    }
    return 0;
}

const char *pw_textarea_get_text(int wid)
{
    static char buf[PW_TEXTAREA_MAX_LINES * PW_TEXTAREA_LINE_LEN];
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TEXTAREA) return "";
    int pos = 0;
    for (int i = 0; i < s->u.textarea.line_count; i++) {
        int n = snprintf(buf + pos, (int)sizeof(buf) - pos, "%s", s->u.textarea.lines[i]);
        if (n > 0) pos += n;
        if (i < s->u.textarea.line_count - 1 && pos < (int)sizeof(buf) - 1)
            buf[pos++] = '\n';
    }
    buf[pos] = '\0';
    return buf;
}

void pw_textarea_set_text(int wid, const char *text)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TEXTAREA) return;
    memset(s->u.textarea.lines, 0, sizeof(s->u.textarea.lines));
    s->u.textarea.line_count  = 0;
    s->u.textarea.cursor_line = 0;
    s->u.textarea.cursor_col  = 0;
    const char *p = text;
    while (*p && s->u.textarea.line_count < PW_TEXTAREA_MAX_LINES) {
        int i = s->u.textarea.line_count++;
        int j = 0;
        while (*p && *p != '\n' && j < PW_TEXTAREA_LINE_LEN - 1)
            s->u.textarea.lines[i][j++] = *p++;
        s->u.textarea.lines[i][j] = '\0';
        if (*p == '\n') p++;
    }
    if (s->u.textarea.line_count == 0) s->u.textarea.line_count = 1;
    textarea_rebuild_text(s);
}

void pw_textarea_tick(int wid, int elapsed_ms)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_TEXTAREA) return;
    int focused = (pa_focus_get() == s->wid);
    if (focused && !s->u.textarea.was_focused) {
        s->u.textarea.blink_on    = 1;
        s->u.textarea.blink_ms    = 0;
        s->u.textarea.was_focused = 1;
        textarea_rebuild_text(s); return;
    }
    if (!focused && s->u.textarea.was_focused) {
        s->u.textarea.blink_on    = 0;
        s->u.textarea.blink_ms    = 0;
        s->u.textarea.was_focused = 0;
        textarea_rebuild_text(s); return;
    }
    if (!focused) return;
    s->u.textarea.blink_ms += elapsed_ms;
    if (s->u.textarea.blink_ms >= 500) {
        s->u.textarea.blink_ms = 0;
        s->u.textarea.blink_on = !s->u.textarea.blink_on;
        textarea_rebuild_text(s);
    }
}

/* =========================================================================
 * Markdown widget
 * ========================================================================= */

/* Process inline markdown within a segment of length `len` from `src`,
 * emitting colorized text into dst starting at position *di.
 * Returns updated *di. */
/* Helper: emit \x03 (bold on) escape */
static int col_bold_on(char *buf, int pos, int max)
{
    if (pos + 1 >= max) return pos;
    buf[pos++] = '\x03';
    return pos;
}

/* Helper: emit \x04 (italic on) escape */
static int col_italic_on(char *buf, int pos, int max)
{
    if (pos + 1 >= max) return pos;
    buf[pos++] = '\x04';
    return pos;
}

static int md_inline(const char *src, int len, char *dst, int di, int dst_len)
{
    int i = 0;
    while (i < len && di < dst_len - 2) {
        /* Bold: **text** — render as actual bold */
        if (i + 1 < len && src[i] == '*' && src[i+1] == '*') {
            int j = i + 2;
            while (j + 1 < len && !(src[j] == '*' && src[j+1] == '*')) j++;
            if (j + 1 < len) {
                di = col_bold_on(dst, di, dst_len);
                di = col_text(dst, di, dst_len, src + i + 2, j - (i + 2));
                di = col_reset(dst, di, dst_len);
                i = j + 2;
                continue;
            }
        }
        /* Italic: *text* (not **) — render as actual italic */
        if (src[i] == '*') {
            int j = i + 1;
            while (j < len && src[j] != '*') j++;
            if (j < len) {
                di = col_italic_on(dst, di, dst_len);
                di = col_text(dst, di, dst_len, src + i + 1, j - (i + 1));
                di = col_reset(dst, di, dst_len);
                i = j + 1;
                continue;
            }
        }
        /* Code: `text` — orange color, no bold/italic */
        if (src[i] == '`') {
            int j = i + 1;
            while (j < len && src[j] != '`') j++;
            if (j < len) {
                di = col_emit(dst, di, dst_len, 220, 140, 80);
                di = col_text(dst, di, dst_len, src + i + 1, j - (i + 1));
                di = col_reset(dst, di, dst_len);
                i = j + 1;
                continue;
            }
        }
        /* Plain char */
        if (di < dst_len - 1) dst[di++] = src[i];
        i++;
    }
    return di;
}

static void markdown_colorize(const char *src, char *dst, int dst_len)
{
    int di = 0;
    const char *p = src;

    while (*p && di < dst_len - 2) {
        /* Find end of line */
        const char *eol = p;
        while (*eol && *eol != '\n') eol++;
        int line_len = (int)(eol - p);

        /* H3: ### — green + bold */
        if (line_len >= 4 && p[0]=='#' && p[1]=='#' && p[2]=='#' && p[3]==' ') {
            di = col_emit(dst, di, dst_len, 80, 200, 120);
            di = col_bold_on(dst, di, dst_len);
            di = md_inline(p + 4, line_len - 4, dst, di, dst_len);
            di = col_reset(dst, di, dst_len);
        }
        /* H2: ## — blue + bold */
        else if (line_len >= 3 && p[0]=='#' && p[1]=='#' && p[2]==' ') {
            di = col_emit(dst, di, dst_len, 100, 180, 220);
            di = col_bold_on(dst, di, dst_len);
            di = md_inline(p + 3, line_len - 3, dst, di, dst_len);
            di = col_reset(dst, di, dst_len);
        }
        /* H1: # — yellow + bold */
        else if (line_len >= 2 && p[0]=='#' && p[1]==' ') {
            di = col_emit(dst, di, dst_len, 220, 200, 60);
            di = col_bold_on(dst, di, dst_len);
            di = md_inline(p + 2, line_len - 2, dst, di, dst_len);
            di = col_reset(dst, di, dst_len);
        }
        /* Bullet: - or * — colored bullet marker, then inline content */
        else if (line_len >= 2 && (p[0]=='-' || p[0]=='*') && p[1]==' ') {
            di = col_emit(dst, di, dst_len, 160, 180, 220);
            di = col_text(dst, di, dst_len, "\xe2\x80\xa2 ", 4); /* • + space */
            di = col_reset(dst, di, dst_len);
            di = md_inline(p + 2, line_len - 2, dst, di, dst_len);
        }
        /* Normal line: process inline markup only */
        else {
            di = md_inline(p, line_len, dst, di, dst_len);
        }

        if (di < dst_len - 1) dst[di++] = '\n';
        if (*eol == '\n') p = eol + 1;
        else break;
    }
    /* Trim trailing newline */
    if (di > 0 && dst[di - 1] == '\n') di--;
    dst[di] = '\0';
}

int pw_markdown_create(int parent_wid)
{
    int wid = pa_widget_add("Markdown", "", "", parent_wid);
    if (wid < 0) return -1;
    PW_Slot *s = slot_alloc(PW_TYPE_MARKDOWN, wid);
    if (!s) return -1;
    s->u.markdown.raw[0] = '\0';
    return wid;
}

void pw_markdown_set_text(int wid, const char *text)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_MARKDOWN) return;
    snprintf(s->u.markdown.raw, PW_MARKDOWN_TEXT_LEN, "%s", text);
    static char colored[PW_MARKDOWN_TEXT_LEN * 4];
    markdown_colorize(s->u.markdown.raw, colored, (int)sizeof(colored));
    pa_widget_set_text(wid, colored);
}

/* =========================================================================
 * DataTable widget
 * ========================================================================= */

static PW_DtData *dt_pool_alloc(int wid)
{
    for (int i = 0; i < PW_DT_MAX_TABLES; i++) {
        if (!g_dt_pool[i].used) {
            memset(&g_dt_pool[i], 0, sizeof(PW_DtData));
            g_dt_pool[i].used = 1;
            g_dt_pool[i].wid  = wid;
            return &g_dt_pool[i];
        }
    }
    return NULL;
}

static PW_DtData *dt_get(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_DATATABLE) return NULL;
    int idx = s->u.datatable.dt_idx;
    if (idx < 0 || idx >= PW_DT_MAX_TABLES) return NULL;
    return &g_dt_pool[idx];
}

/* Effective width for column c (auto = max of header + all cell lengths) */
static int dt_col_w(PW_DtData *d, int c)
{
    if (d->col_widths[c] > 0) return d->col_widths[c];
    int w = (int)strlen(d->col_keys[c]);
    for (int r = 0; r < d->row_count; r++) {
        int l = (int)strlen(d->cells[r][c]);
        if (l > w) w = l;
    }
    if (w < 4)               w = 4;
    if (w > PW_DT_CELL_LEN - 1) w = PW_DT_CELL_LEN - 1;
    return w;
}

/* Write a UTF-8 string to buf padded/truncated to `width` chars */
static int dt_write_cell(char *buf, int pos, int max, const char *text, int width)
{
    int len = (int)strlen(text);
    for (int i = 0; i < width && pos < max - 1; i++) {
        buf[pos++] = (i < len) ? text[i] : ' ';
    }
    return pos;
}

/* Write UTF-8 sequence (n bytes) to buf */
static int dt_write_utf8(char *buf, int pos, int max, const char *seq, int n)
{
    for (int i = 0; i < n && pos < max - 1; i++)
        buf[pos++] = seq[i];
    return pos;
}

static const char DT_COL_SEP[3] = {'\xe2', '\x94', '\x82'}; /* │ */
static const char DT_H_LINE[3]  = {'\xe2', '\x94', '\x80'}; /* ─ */
static const char DT_CROSS [3]  = {'\xe2', '\x94', '\xbc'}; /* ┼ */

static void dt_rebuild_text(PW_DtData *d)
{
    static char buf[8192];
    int pos = 0, max = (int)sizeof(buf);

    if (d->col_count == 0) { pa_widget_set_text(d->wid, ""); return; }

    /* Pre-compute column widths */
    int cw[PW_DT_MAX_COLS];
    for (int c = 0; c < d->col_count; c++) cw[c] = dt_col_w(d, c);

    /* Clamp scroll_row */
    TG_Region reg  = pa_widget_region(d->wid);
    int hdr_lines  = d->show_header ? 2 : 0;
    int visible    = (reg.height > hdr_lines) ? (reg.height - hdr_lines) : 1;
    int max_sr     = (d->row_count > visible) ? (d->row_count - visible) : 0;
    if (d->scroll_row > max_sr) d->scroll_row = max_sr;
    if (d->scroll_row < 0)      d->scroll_row = 0;

    /* ── Header ── */
    if (d->show_header) {
        for (int c = 0; c < d->col_count; c++) {
            /* Header column highlighted if COLUMN cursor */
            int hl = d->show_cursor && d->cursor_type == PW_DT_CURSOR_COLUMN
                     && c == d->cursor_col;
            if (hl) pos = col_emit(buf, pos, max, 100, 180, 255);
            pos = col_bold_on(buf, pos, max);
            if (pos < max - 1) buf[pos++] = ' ';
            pos = dt_write_cell(buf, pos, max, d->col_keys[c], cw[c]);
            if (pos < max - 1) buf[pos++] = ' ';
            pos = col_reset(buf, pos, max);
            if (c < d->col_count - 1)
                pos = dt_write_utf8(buf, pos, max, DT_COL_SEP, 3);
        }
        if (pos < max - 1) buf[pos++] = '\n';

        /* Separator ─┼─┼─ */
        for (int c = 0; c < d->col_count; c++) {
            for (int i = 0; i < cw[c] + 2; i++)
                pos = dt_write_utf8(buf, pos, max, DT_H_LINE, 3);
            if (c < d->col_count - 1)
                pos = dt_write_utf8(buf, pos, max, DT_CROSS, 3);
        }
        if (pos < max - 1) buf[pos++] = '\n';
    }

    /* ── Data rows ── */
    for (int r = d->scroll_row;
         r < d->row_count && (r - d->scroll_row) < visible;
         r++)
    {
        int cursor_row = d->show_cursor && d->cursor_type == PW_DT_CURSOR_ROW
                         && r == d->cursor_row;

        for (int c = 0; c < d->col_count; c++) {
            int cursor_col  = d->show_cursor && d->cursor_type == PW_DT_CURSOR_COLUMN
                              && c == d->cursor_col;
            int cursor_cell = d->show_cursor && d->cursor_type == PW_DT_CURSOR_CELL
                              && r == d->cursor_row && c == d->cursor_col;
            int hl = cursor_row || cursor_col || cursor_cell;

            if (hl) {
                pos = col_emit(buf, pos, max, 255, 220, 60);
            } else if (d->zebra_stripes && (r % 2 == 1)) {
                pos = col_emit(buf, pos, max, 160, 170, 200);
            }

            /* ▶ marker for cursor row (first column only) */
            if (cursor_row && c == 0) {
                const char arrow[3] = {'\xe2', '\x96', '\xb6'}; /* ▶ */
                pos = dt_write_utf8(buf, pos, max, arrow, 3);
            } else {
                if (pos < max - 1) buf[pos++] = ' ';
            }

            pos = dt_write_cell(buf, pos, max, d->cells[r][c], cw[c]);
            if (pos < max - 1) buf[pos++] = ' ';

            if (hl || (d->zebra_stripes && r % 2 == 1))
                pos = col_reset(buf, pos, max);

            if (c < d->col_count - 1)
                pos = dt_write_utf8(buf, pos, max, DT_COL_SEP, 3);
        }
        if (pos < max - 1) buf[pos++] = '\n';
    }

    if (pos > 0 && buf[pos - 1] == '\n') pos--;
    buf[pos] = '\0';
    pa_widget_set_text(d->wid, buf);
}

int pw_datatable_create(int parent_wid)
{
    int wid = pa_widget_add("DataTable", "", "", parent_wid);
    if (wid < 0) return -1;
    PW_DtData *d = dt_pool_alloc(wid);
    if (!d) return -1;
    int idx = (int)(d - g_dt_pool);
    PW_Slot *s = slot_alloc(PW_TYPE_DATATABLE, wid);
    if (!s) return -1;
    s->u.datatable.dt_idx = idx;

    d->show_header  = 1;
    d->show_cursor  = 1;
    d->cursor_type  = PW_DT_CURSOR_ROW;
    d->zebra_stripes = 1;
    return wid;
}

void pw_datatable_register_keys(int wid)
{
    PW_DtData *d = dt_get(wid);
    if (!d) return;
    d->bid_up    = pa_bind_key(wid, PW_KEY_UP,    0);
    d->bid_down  = pa_bind_key(wid, PW_KEY_DOWN,  0);
    d->bid_left  = pa_bind_key(wid, PW_KEY_LEFT,  0);
    d->bid_right = pa_bind_key(wid, PW_KEY_RIGHT, 0);
    d->bid_home  = pa_bind_key(wid, PW_KEY_HOME,  0);
    d->bid_end   = pa_bind_key(wid, PW_KEY_END,   0);
    d->bid_pgup  = pa_bind_key(wid, PW_KEY_PGUP,  0);
    d->bid_pgdn  = pa_bind_key(wid, PW_KEY_PGDN,  0);
    d->bid_enter = pa_bind_key(wid, PW_KEY_ENTER, 0);
    d->bid_click = pa_bind_click(wid);
}

int pw_datatable_add_column(int wid, const char *label, int width)
{
    PW_DtData *d = dt_get(wid);
    if (!d || d->col_count >= PW_DT_MAX_COLS) return -1;
    int c = d->col_count++;
    snprintf(d->col_keys[c], PW_DT_HDR_LEN, "%s", label ? label : "");
    d->col_widths[c] = width;
    dt_rebuild_text(d);
    return c;
}

int pw_datatable_add_row(int wid)
{
    PW_DtData *d = dt_get(wid);
    if (!d || d->row_count >= PW_DT_MAX_ROWS) return -1;
    int r = d->row_count++;
    for (int c = 0; c < PW_DT_MAX_COLS; c++) d->cells[r][c][0] = '\0';
    dt_rebuild_text(d);
    return r;
}

void pw_datatable_set_cell(int wid, int row, int col, const char *text)
{
    PW_DtData *d = dt_get(wid);
    if (!d || row < 0 || row >= d->row_count || col < 0 || col >= d->col_count) return;
    snprintf(d->cells[row][col], PW_DT_CELL_LEN, "%s", text ? text : "");
    dt_rebuild_text(d);
}

void pw_datatable_clear_rows(int wid)
{
    PW_DtData *d = dt_get(wid);
    if (!d) return;
    d->row_count  = 0;
    d->cursor_row = 0;
    d->cursor_col = 0;
    d->scroll_row = 0;
    d->scroll_col = 0;
    dt_rebuild_text(d);
}

void pw_datatable_set_cursor_type(int wid, int cursor_type)
{
    PW_DtData *d = dt_get(wid);
    if (!d) return;
    d->cursor_type = cursor_type;
    dt_rebuild_text(d);
}

void pw_datatable_set_show_header(int wid, int show)
{
    PW_DtData *d = dt_get(wid);
    if (!d) return;
    d->show_header = show;
    dt_rebuild_text(d);
}

void pw_datatable_set_show_cursor(int wid, int show)
{
    PW_DtData *d = dt_get(wid);
    if (!d) return;
    d->show_cursor = show;
    dt_rebuild_text(d);
}

void pw_datatable_set_zebra(int wid, int enable)
{
    PW_DtData *d = dt_get(wid);
    if (!d) return;
    d->zebra_stripes = enable;
    dt_rebuild_text(d);
}

int pw_datatable_get_cursor_row(int wid)
{
    PW_DtData *d = dt_get(wid);
    return d ? d->cursor_row : -1;
}

int pw_datatable_get_cursor_col(int wid)
{
    PW_DtData *d = dt_get(wid);
    return d ? d->cursor_col : -1;
}

void pw_datatable_move_cursor(int wid, int row, int col)
{
    PW_DtData *d = dt_get(wid);
    if (!d) return;
    if (row < 0) row = 0;
    if (row >= d->row_count) row = d->row_count - 1;
    if (col < 0) col = 0;
    if (col >= d->col_count) col = d->col_count - 1;
    d->cursor_row = row;
    d->cursor_col = col;
    /* Auto-scroll to keep cursor visible */
    TG_Region reg = pa_widget_region(d->wid);
    int hdr = d->show_header ? 2 : 0;
    int vis = (reg.height > hdr) ? (reg.height - hdr) : 1;
    if (d->cursor_row < d->scroll_row) d->scroll_row = d->cursor_row;
    if (d->cursor_row >= d->scroll_row + vis) d->scroll_row = d->cursor_row - vis + 1;
    dt_rebuild_text(d);
}

int pw_datatable_is_selected(int wid)
{
    PW_DtData *d = dt_get(wid);
    if (!d || !d->selected) return 0;
    d->selected = 0;
    return 1;
}

int pw_datatable_handle(int wid, int bid)
{
    PW_DtData *d = dt_get(wid);
    if (!d) return 0;

    TG_Region reg = pa_widget_region(wid);
    int hdr = d->show_header ? 2 : 0;
    int vis = (reg.height > hdr) ? (reg.height - hdr) : 1;

    if (bid == d->bid_up) {
        pw_datatable_move_cursor(wid, d->cursor_row - 1, d->cursor_col);
        return 1;
    }
    if (bid == d->bid_down) {
        pw_datatable_move_cursor(wid, d->cursor_row + 1, d->cursor_col);
        return 1;
    }
    if (bid == d->bid_left) {
        pw_datatable_move_cursor(wid, d->cursor_row, d->cursor_col - 1);
        return 1;
    }
    if (bid == d->bid_right) {
        pw_datatable_move_cursor(wid, d->cursor_row, d->cursor_col + 1);
        return 1;
    }
    if (bid == d->bid_home) {
        pw_datatable_move_cursor(wid, d->cursor_row, 0);
        return 1;
    }
    if (bid == d->bid_end) {
        pw_datatable_move_cursor(wid, d->cursor_row, d->col_count - 1);
        return 1;
    }
    if (bid == d->bid_pgup) {
        pw_datatable_move_cursor(wid, d->cursor_row - vis, d->cursor_col);
        return 1;
    }
    if (bid == d->bid_pgdn) {
        pw_datatable_move_cursor(wid, d->cursor_row + vis, d->cursor_col);
        return 1;
    }
    if (bid == d->bid_enter) {
        d->selected = 1;
        return 1;
    }
    if (bid == d->bid_click) {
        int mx, my;
        pa_mouse_pos(&mx, &my);
        int row_clicked = (my - reg.y - hdr) + d->scroll_row;
        if (row_clicked >= 0 && row_clicked < d->row_count) {
            /* Determine column from x position */
            int x = reg.x;
            int col_clicked = 0;
            for (int c = 0; c < d->col_count; c++) {
                int w = dt_col_w(d, c) + 2 + 3; /* cell + spaces + separator */
                if (mx < x + w) { col_clicked = c; break; }
                x += w;
                col_clicked = c;
            }
            pw_datatable_move_cursor(wid, row_clicked, col_clicked);
        }
        return 1;
    }
    return 0;
}

/* =========================================================================
 * Welcome widget (Markdown + Button composite)
 * ========================================================================= */

int pw_welcome_create(int parent_wid, const char *markdown_text, const char *button_label)
{
    int wid = pa_widget_add("Welcome", "", "", parent_wid);
    if (wid < 0) return -1;
    pa_widget_set_layout(wid, PL_LAYOUT_VERTICAL);

    int md_wid = pw_markdown_create(wid);
    if (md_wid >= 0) {
        pa_widget_set_size(md_wid,
            (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
            (PL_SizeValue){ PL_SIZE_FRACTION, 1 });
        pw_markdown_set_text(md_wid, markdown_text ? markdown_text : "");
    }

    int btn_wid = pw_button_create(button_label ? button_label : "Continue", "primary", wid);
    if (btn_wid >= 0)
        pa_widget_set_size(btn_wid,
            (PL_SizeValue){ PL_SIZE_FRACTION, 1 },
            (PL_SizeValue){ PL_SIZE_FIXED,    1 });

    PW_Slot *s = slot_alloc(PW_TYPE_WELCOME, wid);
    if (!s) return -1;
    s->u.welcome.md_wid  = md_wid;
    s->u.welcome.btn_wid = btn_wid;
    return wid;
}

int pw_welcome_button_wid(int wid)
{
    PW_Slot *s = slot_find(wid);
    if (!s || s->widget_type != PW_TYPE_WELCOME) return -1;
    return s->u.welcome.btn_wid;
}

/* =========================================================================
 * Automatic click dispatch for multi-option widgets
 * ========================================================================= */

int pw_dispatch_click(int bid)
{
    int mx, my;
    pa_mouse_pos(&mx, &my);

    for (int i = 0; i < PW_MAX_SLOTS; i++) {
        PW_Slot *s = &g_pool[i];
        if (!s->wid) continue;

        switch (s->widget_type) {
        case PW_TYPE_OPTIONLIST:
        case PW_TYPE_LISTVIEW:
        case PW_TYPE_SELECTIONLIST:
            if (bid != s->u.list.bid_click) break;
            {
                TG_Region r = pa_widget_region(s->wid);
                int row = my - r.y;
                if (row < 0 || row >= s->u.list.count) return 1;
                if (s->widget_type == PW_TYPE_OPTIONLIST)
                    pw_optionlist_set_cursor(s->wid, row);
                else if (s->widget_type == PW_TYPE_LISTVIEW)
                    pw_listview_set_cursor(s->wid, row);
                else {
                    pw_selectionlist_set_cursor(s->wid, row);
                    pw_selectionlist_toggle_selection(s->wid, row);
                }
                return 1;
            }
        case PW_TYPE_RADIOSET:
            for (int r = 0; r < s->u.radioset.count; r++) {
                if (bid == s->u.radioset.bid_radio[r]) {
                    pw_radioset_select(s->wid, r);
                    return 1;
                }
            }
            break;
        default:
            break;
        }
    }
    return 0;
}

int pw_dispatch_scroll(int bid)
{
    int dy = pa_scroll_dy();
    for (int i = 0; i < PW_MAX_SLOTS; i++) {
        PW_Slot *s = &g_pool[i];
        if (!s->wid) continue;
        if (s->widget_type != PW_TYPE_LOG && s->widget_type != PW_TYPE_RICHLOG) continue;
        if (bid != s->u.log.bid_scroll) continue;
        s->u.log.scroll_top += dy;
        log_rebuild_text(s);
        return 1;
    }
    return 0;
}

/* =========================================================================
 * Default TCSS
 * ========================================================================= */

const char *pw_default_tcss(void)
{
    return DFLT_TCSS;
}
