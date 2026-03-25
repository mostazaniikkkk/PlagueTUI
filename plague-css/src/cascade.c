#include "cascade.h"
#include "colors.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* ---------------------------------------------------------------------------
 * Utilidades de string
 * --------------------------------------------------------------------------- */

static int str_eq_ci(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

/* Comprueba si `word` aparece como token separado por espacios en `list`. */
static int has_word(const char *list, const char *word)
{
    const char *p = list;
    int wlen = (int)strlen(word);
    while (*p) {
        while (*p == ' ') p++;
        int n = 0;
        while (p[n] && p[n] != ' ') n++;
        if (n == wlen && strncmp(p, word, n) == 0) return 1;
        p += n;
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * Matching de selector
 * --------------------------------------------------------------------------- */

static int selector_matches(
    const CSSRule *rule,
    const char    *type_name,
    const char    *id,
    const char    *classes,
    int            state)
{
    /* Sin partes de selector: coincide con todo (regla degenerada) */
    if (rule->part_count == 0) return 1;

    for (int i = 0; i < rule->part_count; i++) {
        const SelPart *p = &rule->parts[i];
        switch (p->type) {
            case SEL_UNIVERSAL:
                break;  /* * siempre coincide */

            case SEL_TYPE:
                if (!str_eq_ci(type_name, p->name)) return 0;
                break;

            case SEL_CLASS:
                if (!has_word(classes, p->name)) return 0;
                break;

            case SEL_ID:
                if (!str_eq_ci(id, p->name)) return 0;
                break;

            case SEL_PSEUDO:
                if (str_eq_ci(p->name, "hover")) {
                    if (!(state & PC_STATE_HOVER)) return 0;
                } else if (str_eq_ci(p->name, "focus")) {
                    if (!(state & PC_STATE_FOCUS)) return 0;
                } else if (str_eq_ci(p->name, "pressed") || str_eq_ci(p->name, "active")) {
                    if (!(state & PC_STATE_PRESSED)) return 0;
                } else if (str_eq_ci(p->name, "disabled")) {
                    if (!(state & PC_STATE_DISABLED)) return 0;
                }
                /* pseudo-clases desconocidas se ignoran (no descartan) */
                break;
        }
    }
    return 1;
}

/* ---------------------------------------------------------------------------
 * Parseo de valores de propiedades
 * --------------------------------------------------------------------------- */

static PL_SizeValue parse_size(const char *v)
{
    PL_SizeValue sv;
    int n = atoi(v);
    int len = (int)strlen(v);
    /* detectar "Nfr" */
    if (len >= 2 && tolower((unsigned char)v[len-2]) == 'f'
                 && tolower((unsigned char)v[len-1]) == 'r') {
        sv.type  = PL_SIZE_FRACTION;
        sv.value = (n > 0) ? n : 1;
    } else if (str_eq_ci(v, "auto")) {
        sv.type  = PL_SIZE_FRACTION;
        sv.value = 1;
    } else {
        sv.type  = PL_SIZE_FIXED;
        sv.value = n;
    }
    return sv;
}

/*
 * Parsea "1 2 3 4" en top/right/bottom/left.
 * Soporta 1, 2 o 4 valores.  (3 valores no es TCSS estándar, se ignorarán.)
 */
static TG_Spacing parse_spacing(const char *v)
{
    int vals[4] = {0, 0, 0, 0};
    int count = 0;
    const char *p = v;
    while (*p && count < 4) {
        while (*p == ' ') p++;
        if (!*p) break;
        vals[count++] = atoi(p);
        while (*p && *p != ' ') p++;
    }
    TG_Spacing sp;
    if (count == 1) {
        sp.top = sp.right = sp.bottom = sp.left = vals[0];
    } else if (count == 2) {
        sp.top = sp.bottom = vals[0];
        sp.right = sp.left = vals[1];
    } else if (count >= 4) {
        sp.top    = vals[0];
        sp.right  = vals[1];
        sp.bottom = vals[2];
        sp.left   = vals[3];
    } else {
        sp.top = sp.right = sp.bottom = sp.left = 0;
    }
    return sp;
}

static int parse_dock(const char *v)
{
    if (str_eq_ci(v, "top"))    return 0;  /* PL_DOCK_TOP    */
    if (str_eq_ci(v, "bottom")) return 1;  /* PL_DOCK_BOTTOM */
    if (str_eq_ci(v, "left"))   return 2;  /* PL_DOCK_LEFT   */
    if (str_eq_ci(v, "right"))  return 3;  /* PL_DOCK_RIGHT  */
    if (str_eq_ci(v, "center")) return 4;  /* PL_DOCK_CENTER */
    return -1;
}

static int parse_overflow(const char *v)
{
    if (str_eq_ci(v, "hidden")) return 0;
    if (str_eq_ci(v, "scroll")) return 1;
    if (str_eq_ci(v, "auto"))   return 2;
    return 0;
}

/*
 * Parsea "border: solid white" o "border: none".
 * Extrae estilo y color.
 */
static void parse_border(const char *v, PC_BorderStyle *style, PC_Color *color)
{
    char keyword[CSS_NAME_LEN] = {0};
    char colorstr[CSS_NAME_LEN] = {0};

    /* separar primer token y el resto */
    int n = 0;
    const char *p = v;
    while (*p && *p != ' ' && n < CSS_NAME_LEN-1) keyword[n++] = *p++;
    keyword[n] = '\0';
    while (*p == ' ') p++;
    n = 0;
    while (*p && n < CSS_NAME_LEN-1) colorstr[n++] = *p++;
    colorstr[n] = '\0';

    if (str_eq_ci(keyword, "none")) {
        *style = PC_BORDER_NONE;
        return;
    }
    if (str_eq_ci(keyword, "solid"))   *style = PC_BORDER_SOLID;
    else if (str_eq_ci(keyword, "dashed"))  *style = PC_BORDER_DASHED;
    else if (str_eq_ci(keyword, "rounded")) *style = PC_BORDER_ROUNDED;
    else if (str_eq_ci(keyword, "thick"))   *style = PC_BORDER_THICK;
    else if (str_eq_ci(keyword, "double"))  *style = PC_BORDER_DOUBLE;
    else *style = PC_BORDER_SOLID;

    if (colorstr[0]) css_parse_color(colorstr, color);
}

/* ---------------------------------------------------------------------------
 * Aplicar una declaración al estilo acumulado
 * --------------------------------------------------------------------------- */

static void apply_decl(PC_ComputedStyle *s, const CSSDecl *d)
{
    const char *p = d->prop;
    const char *v = d->value;

    if (str_eq_ci(p, "color")) {
        PC_Color c = {0,0,0,255};
        if (css_parse_color(v, &c)) { s->color = c; s->set_flags |= PC_HAS_COLOR; }
    }
    else if (str_eq_ci(p, "background") || str_eq_ci(p, "background-color")) {
        PC_Color c = {0,0,0,255};
        if (css_parse_color(v, &c)) { s->background = c; s->set_flags |= PC_HAS_BACKGROUND; }
    }
    else if (str_eq_ci(p, "border")) {
        parse_border(v, &s->border_style, &s->border_color);
        s->set_flags |= PC_HAS_BORDER;
    }
    else if (str_eq_ci(p, "margin")) {
        s->margin = parse_spacing(v);
        s->set_flags |= PC_HAS_MARGIN;
    }
    else if (str_eq_ci(p, "padding")) {
        s->padding = parse_spacing(v);
        s->set_flags |= PC_HAS_PADDING;
    }
    else if (str_eq_ci(p, "width")) {
        s->width = parse_size(v);
        s->set_flags |= PC_HAS_WIDTH;
    }
    else if (str_eq_ci(p, "height")) {
        s->height = parse_size(v);
        s->set_flags |= PC_HAS_HEIGHT;
    }
    else if (str_eq_ci(p, "dock")) {
        int dock = parse_dock(v);
        if (dock >= 0) { s->dock = dock; s->set_flags |= PC_HAS_DOCK; }
    }
    else if (str_eq_ci(p, "display")) {
        s->display = str_eq_ci(v, "none") ? 0 : 1;
        s->set_flags |= PC_HAS_DISPLAY;
    }
    else if (str_eq_ci(p, "overflow") || str_eq_ci(p, "overflow-x")
                                      || str_eq_ci(p, "overflow-y")) {
        s->overflow = parse_overflow(v);
        s->set_flags |= PC_HAS_OVERFLOW;
    }
    else if (str_eq_ci(p, "text-style")) {
        /* "bold", "italic", "underline", "bold italic", etc. o "none" */
        if (str_eq_ci(v, "none")) {
            s->bold = s->italic = s->underline = 0;
            s->set_flags |= (PC_HAS_BOLD|PC_HAS_ITALIC|PC_HAS_UNDERLINE);
        } else {
            if (has_word(v, "bold"))      { s->bold      = 1; s->set_flags |= PC_HAS_BOLD; }
            if (has_word(v, "italic"))    { s->italic    = 1; s->set_flags |= PC_HAS_ITALIC; }
            if (has_word(v, "underline")) { s->underline = 1; s->set_flags |= PC_HAS_UNDERLINE; }
        }
    }
    else if (str_eq_ci(p, "bold")) {
        s->bold = str_eq_ci(v, "true") || str_eq_ci(v, "1");
        s->set_flags |= PC_HAS_BOLD;
    }
    /* propiedades desconocidas se ignoran silenciosamente */
}

/* ---------------------------------------------------------------------------
 * Comparador para qsort (por especificidad ascendente)
 * --------------------------------------------------------------------------- */

typedef struct { int specificity; int index; } RuleRef;

static int cmp_rule_ref(const void *a, const void *b)
{
    int ds = ((RuleRef*)a)->specificity - ((RuleRef*)b)->specificity;
    if (ds != 0) return ds;
    /* misma especificidad: la regla posterior en la hoja gana (se aplica última) */
    return ((RuleRef*)a)->index - ((RuleRef*)b)->index;
}

/* ---------------------------------------------------------------------------
 * Cascada principal
 * --------------------------------------------------------------------------- */

PC_ComputedStyle css_cascade(
    const CSSStylesheet *ss,
    const char          *type_name,
    const char          *id,
    const char          *classes,
    int                  state)
{
    PC_ComputedStyle style;
    memset(&style, 0, sizeof(style));
    style.display  = 1;
    style.overflow = 0;
    style.dock     = -1;
    PL_SizeValue fr1 = { PL_SIZE_FRACTION, 1 };
    style.width  = fr1;
    style.height = fr1;

    /* recopilar reglas que coinciden */
    RuleRef refs[CSS_MAX_RULES];
    int     nrefs = 0;

    for (int i = 0; i < ss->rule_count; i++) {
        if (selector_matches(&ss->rules[i], type_name, id, classes, state)) {
            refs[nrefs].specificity = ss->rules[i].specificity;
            refs[nrefs].index       = i;
            nrefs++;
        }
    }
    if (nrefs == 0) return style;

    /* ordenar por especificidad (menor primero — se sobreescribe con mayor) */
    qsort(refs, nrefs, sizeof(RuleRef), cmp_rule_ref);

    /* aplicar declaraciones en orden */
    for (int i = 0; i < nrefs; i++) {
        const CSSRule *rule = &ss->rules[refs[i].index];
        for (int j = 0; j < rule->decl_count; j++)
            apply_decl(&style, &rule->decls[j]);
    }

    return style;
}
