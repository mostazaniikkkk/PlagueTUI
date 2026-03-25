# plague-css

Parser y motor de cascada CSS para PlagueTUI. Parsea hojas de estilo TCSS y calcula el estilo computado de un widget dado su tipo, id, clases y estado.

## Responsabilidad

Dado un string TCSS y la descripción de un widget (tipo, id, clases, pseudo-estados), devuelve un `PC_ComputedStyle` con todas las propiedades resueltas aplicando especificidad y orden de cascada.

## API pública

```c
// Cargar/liberar hoja de estilos
int  pc_stylesheet_load(const char *tcss);    // devuelve handle (>=1) o 0
void pc_stylesheet_free(int handle);
int  pc_stylesheet_rule_count(int handle);

// Computar estilos
PC_ComputedStyle pc_compute_style(
    int         ss_handle,
    const char *type_name,  // "Button", "" si no aplica
    const char *id,         // "my-btn", "" si no aplica
    const char *classes,    // "primary large" (separadas por espacio)
    int         state       // bitmask PC_STATE_*
);

PC_ComputedStyle pc_default_style(void);
```

## Selectores soportados

| Selector | Ejemplo | Especificidad |
|----------|---------|---------------|
| Tipo | `Button { }` | 1 |
| Clase | `.primary { }` | 10 |
| ID | `#my-btn { }` | 100 |
| Pseudo-clase | `:hover` `:focus` `:pressed` | 10 |
| Universal | `* { }` | 0 |
| Compuesto | `Button.primary:hover { }` | suma |

## Propiedades soportadas

| Propiedad | Valores |
|-----------|---------|
| `color`, `background` | nombre, `#rrggbb`, `#rgb`, `transparent` |
| `border` | `none`, `solid <color>`, `dashed`, `rounded`, `thick`, `double` |
| `margin`, `padding` | `2`, `1 2`, `1 2 3 4` (celdas) |
| `width`, `height` | `20` (fixed), `1fr`, `2fr`, `auto` |
| `dock` | `top`, `bottom`, `left`, `right`, `center` |
| `display` | `block`, `none` |
| `overflow` | `hidden`, `scroll`, `auto` |
| `text-style` | `bold`, `italic`, `underline`, combinaciones, `none` |

## Colores con nombre

Incluye ~35 colores: `white`, `black`, `red`, `green`, `blue`, `yellow`, `cyan`, `magenta`, `darkblue`, `gray`, `orange`, `purple`, `transparent`, etc.

## Especificidad y cascada

- Las reglas se ordenan por especificidad ascendente (menor primero).
- A igual especificidad, gana la última regla en la hoja.
- Las propiedades con `$variable` en el valor se ignoran.

## `PC_ComputedStyle`

```c
typedef struct {
    uint32_t       set_flags;    // bitmask PC_HAS_* para saber qué fue establecido
    PC_Color       color;
    PC_Color       background;
    PC_Color       border_color;
    PC_BorderStyle border_style;
    TG_Spacing     margin;
    TG_Spacing     padding;
    PL_SizeValue   width;
    PL_SizeValue   height;
    int            dock;         // PL_DockEdge o -1
    int            display;      // 1=block, 0=none
    int            overflow;
    uint8_t        bold, italic, underline;
} PC_ComputedStyle;
```

## Dependencias

- `plague-geometry` — `TG_Spacing`
- `plague-layout` — `PL_SizeValue`, `PL_DockEdge`

## Compilar y testear

```bat
build.bat   # → bin\plague_css.dll
test.bat    # compila + ejecuta tests con unittest
```

## Tests

| Archivo | Cubre |
|---------|-------|
| `test_parser.py` | Parseo de reglas, comentarios, variables ignoradas, handles múltiples |
| `test_cascade.py` | Matching por tipo/clase/id/pseudo, colores, tamaños, spacing, dock, display, text-style, especificidad |
