#include "parser.h"
#include <string.h>
#include <ctype.h>

/* ---------------------------------------------------------------------------
 * Cursor
 * --------------------------------------------------------------------------- */

typedef struct {
    const char *src;
    int         pos;
    int         len;
} Cur;

static int  peek(Cur *c)           { return (c->pos < c->len) ? (unsigned char)c->src[c->pos] : 0; }
static void adv(Cur *c)            { if (c->pos < c->len) c->pos++; }
static int  at_end(Cur *c)         { return c->pos >= c->len; }

/* Salta whitespace y comentarios CSS */
static void skip_ws(Cur *c)
{
    while (!at_end(c)) {
        int ch = peek(c);
        if (isspace(ch)) { adv(c); continue; }
        /* comentario CSS: barra-asterisco ... asterisco-barra */
        if (ch == '/' && c->pos+1 < c->len && c->src[c->pos+1] == '*') {
            adv(c); adv(c);
            while (!at_end(c)) {
                if (peek(c) == '*' && c->pos+1 < c->len && c->src[c->pos+1] == '/') {
                    adv(c); adv(c); break;
                }
                adv(c);
            }
            continue;
        }
        break;
    }
}

/* Lee un identificador CSS: [a-zA-Z0-9_-]+ */
static int read_ident(Cur *c, char *out, int max)
{
    int n = 0;
    while (!at_end(c)) {
        int ch = peek(c);
        if (isalnum(ch) || ch == '_' || ch == '-') {
            if (n < max - 1) out[n++] = (char)ch;
            adv(c);
        } else break;
    }
    out[n] = '\0';
    return n;
}

/* Lee hasta ';' o '}', recortando espacios al final */
static int read_value(Cur *c, char *out, int max)
{
    int n = 0;
    while (!at_end(c)) {
        int ch = peek(c);
        if (ch == ';' || ch == '}') break;
        if (n < max - 1) out[n++] = (char)ch;
        adv(c);
    }
    /* recortar espacios finales */
    while (n > 0 && isspace((unsigned char)out[n-1])) n--;
    out[n] = '\0';
    return n;
}

/* ---------------------------------------------------------------------------
 * Cálculo de especificidad
 * --------------------------------------------------------------------------- */

static int calc_specificity(const CSSRule *rule)
{
    int sp = 0;
    for (int i = 0; i < rule->part_count; i++) {
        switch (rule->parts[i].type) {
            case SEL_ID:      sp += 100; break;
            case SEL_CLASS:
            case SEL_PSEUDO:  sp +=  10; break;
            case SEL_TYPE:    sp +=   1; break;
            default:          break;
        }
    }
    return sp;
}

/* ---------------------------------------------------------------------------
 * Parseo de un selector
 *
 * Un selector TCSS es una secuencia de partes compuestas (sin espacios entre
 * ellas).  Los espacios separan selectores de descendiente — por simplicidad
 * tratamos cada grupo como parte del mismo selector compuesto (todo debe
 * coincidir con el mismo nodo).
 *
 * Soportado:  Button  .class  #id  :pseudo  *
 * Ignorado:   selectores de descendiente  (Button Label)
 * --------------------------------------------------------------------------- */

static int parse_selector(Cur *c, CSSRule *rule)
{
    rule->part_count = 0;
    skip_ws(c);

    while (!at_end(c)) {
        int ch = peek(c);

        if (ch == '{') break;       /* fin del selector */

        if (ch == ',') {            /* múltiples selectores no soportados */
            adv(c);
            /* saltamos hasta '{' */
            while (!at_end(c) && peek(c) != '{') adv(c);
            break;
        }

        if (isspace(ch)) {
            skip_ws(c);
            /* Si lo siguiente es '{', fin del selector */
            if (peek(c) == '{') break;
            /* Si no, ignoramos el espacio (tratamos como compuesto) */
            continue;
        }

        if (rule->part_count >= CSS_MAX_SEL_PARTS) { adv(c); continue; }

        SelPart *part = &rule->parts[rule->part_count];

        if (ch == '.') {
            adv(c);
            part->type = SEL_CLASS;
            read_ident(c, part->name, CSS_NAME_LEN);
            rule->part_count++;
        } else if (ch == '#') {
            adv(c);
            part->type = SEL_ID;
            read_ident(c, part->name, CSS_NAME_LEN);
            rule->part_count++;
        } else if (ch == ':') {
            adv(c);
            part->type = SEL_PSEUDO;
            read_ident(c, part->name, CSS_NAME_LEN);
            rule->part_count++;
        } else if (ch == '*') {
            adv(c);
            part->type = SEL_UNIVERSAL;
            part->name[0] = '\0';
            rule->part_count++;
        } else if (isalpha(ch) || ch == '_') {
            part->type = SEL_TYPE;
            read_ident(c, part->name, CSS_NAME_LEN);
            rule->part_count++;
        } else {
            adv(c);  /* carácter desconocido, saltar */
        }
    }

    return rule->part_count;
}

/* ---------------------------------------------------------------------------
 * Parseo de un bloque de declaraciones  { prop: value; ... }
 * --------------------------------------------------------------------------- */

static void parse_block(Cur *c, CSSRule *rule)
{
    rule->decl_count = 0;
    if (peek(c) != '{') return;
    adv(c);  /* consume '{' */

    while (!at_end(c)) {
        skip_ws(c);
        if (peek(c) == '}') { adv(c); break; }
        if (at_end(c)) break;

        if (rule->decl_count >= CSS_MAX_DECLS) {
            /* saltar hasta '}' */
            while (!at_end(c) && peek(c) != '}') adv(c);
            if (!at_end(c)) adv(c);
            break;
        }

        CSSDecl *d = &rule->decls[rule->decl_count];

        /* propiedad */
        read_ident(c, d->prop, CSS_NAME_LEN);
        if (d->prop[0] == '\0') { adv(c); continue; }

        skip_ws(c);
        if (peek(c) != ':') {
            /* línea malformada: saltar hasta ';' o '}' */
            while (!at_end(c) && peek(c) != ';' && peek(c) != '}') adv(c);
            if (peek(c) == ';') adv(c);
            continue;
        }
        adv(c);  /* consume ':' */
        skip_ws(c);

        /* valor */
        read_value(c, d->value, CSS_VALUE_LEN);
        if (peek(c) == ';') adv(c);

        /* ignorar propiedades con variables TCSS ($var) */
        if (d->value[0] == '$') continue;

        rule->decl_count++;
    }
}

/* ---------------------------------------------------------------------------
 * Entrada principal
 * --------------------------------------------------------------------------- */

int css_parse(CSSStylesheet *ss, const char *tcss)
{
    ss->rule_count = 0;

    Cur c;
    c.src = tcss;
    c.pos = 0;
    c.len = (int)strlen(tcss);

    while (!at_end(&c)) {
        skip_ws(&c);
        if (at_end(&c)) break;

        if (ss->rule_count >= CSS_MAX_RULES) break;

        CSSRule *rule = &ss->rules[ss->rule_count];
        rule->part_count = 0;
        rule->decl_count = 0;

        parse_selector(&c, rule);
        skip_ws(&c);

        if (peek(&c) != '{') {
            /* selector sin bloque: saltar hasta '}' */
            while (!at_end(&c) && peek(&c) != '}') adv(&c);
            if (!at_end(&c)) adv(&c);
            continue;
        }

        parse_block(&c, rule);
        rule->specificity = calc_specificity(rule);

        /* Solo añadir la regla si tiene al menos un selector o declaración */
        if (rule->part_count > 0 || rule->decl_count > 0)
            ss->rule_count++;
    }

    return ss->rule_count;
}
