#include "../include/plague_css.h"
#include "css_types.h"
#include "parser.h"
#include "cascade.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Pool estático de hojas de estilos (handles 1-based)
 * --------------------------------------------------------------------------- */

static CSSStylesheet g_sheets[CSS_MAX_STYLESHEETS];
static int           g_used  [CSS_MAX_STYLESHEETS];  /* 1 = en uso */

static void pool_init(void)
{
    static int done = 0;
    if (!done) {
        memset(g_sheets, 0, sizeof(g_sheets));
        memset(g_used,   0, sizeof(g_used));
        done = 1;
    }
}

/* ---------------------------------------------------------------------------
 * API pública
 * --------------------------------------------------------------------------- */

int pc_stylesheet_load(const char *tcss)
{
    pool_init();
    for (int i = 0; i < CSS_MAX_STYLESHEETS; i++) {
        if (!g_used[i]) {
            memset(&g_sheets[i], 0, sizeof(g_sheets[i]));
            css_parse(&g_sheets[i], tcss ? tcss : "");
            g_used[i] = 1;
            return i + 1;  /* handle 1-based */
        }
    }
    return 0;  /* pool lleno */
}

void pc_stylesheet_free(int handle)
{
    pool_init();
    if (handle < 1 || handle > CSS_MAX_STYLESHEETS) return;
    g_used[handle - 1] = 0;
}

int pc_stylesheet_rule_count(int handle)
{
    pool_init();
    if (handle < 1 || handle > CSS_MAX_STYLESHEETS) return 0;
    if (!g_used[handle - 1]) return 0;
    return g_sheets[handle - 1].rule_count;
}

PC_ComputedStyle pc_compute_style(
    int         ss_handle,
    const char *type_name,
    const char *id,
    const char *classes,
    int         state)
{
    pool_init();
    if (ss_handle < 1 || ss_handle > CSS_MAX_STYLESHEETS || !g_used[ss_handle-1])
        return pc_default_style();

    return css_cascade(
        &g_sheets[ss_handle - 1],
        type_name ? type_name : "",
        id        ? id        : "",
        classes   ? classes   : "",
        state
    );
}

PC_ComputedStyle pc_default_style(void)
{
    PC_ComputedStyle s;
    memset(&s, 0, sizeof(s));
    s.display  = 1;
    s.overflow = 0;
    s.dock     = -1;
    PL_SizeValue fr1 = { PL_SIZE_FRACTION, 1 };
    s.width  = fr1;
    s.height = fr1;
    return s;
}
