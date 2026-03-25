#ifndef CSS_TYPES_H
#define CSS_TYPES_H

#include "../include/plague_css.h"

/* ---- límites ---- */
#define CSS_MAX_STYLESHEETS  8
#define CSS_MAX_RULES        128
#define CSS_MAX_DECLS        16
#define CSS_MAX_SEL_PARTS    8
#define CSS_NAME_LEN         64
#define CSS_VALUE_LEN        128

/* ---- parte de selector ---- */
typedef enum {
    SEL_TYPE,       /* Button          */
    SEL_CLASS,      /* .primary        */
    SEL_ID,         /* #my-btn         */
    SEL_PSEUDO,     /* :hover          */
    SEL_UNIVERSAL,  /* *               */
} SelPartType;

typedef struct {
    SelPartType type;
    char        name[CSS_NAME_LEN];
} SelPart;

/* ---- declaración ---- */
typedef struct {
    char prop [CSS_NAME_LEN];
    char value[CSS_VALUE_LEN];
} CSSDecl;

/* ---- regla ---- */
typedef struct {
    SelPart  parts[CSS_MAX_SEL_PARTS];
    int      part_count;
    CSSDecl  decls[CSS_MAX_DECLS];
    int      decl_count;
    int      specificity;  /* suma: ID=100, class/pseudo=10, type=1 */
} CSSRule;

/* ---- hoja de estilos ---- */
typedef struct {
    CSSRule rules[CSS_MAX_RULES];
    int     rule_count;
    int     used;
} CSSStylesheet;

#endif /* CSS_TYPES_H */
