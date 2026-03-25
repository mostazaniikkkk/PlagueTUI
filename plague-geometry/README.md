# plague-geometry

Librería de geometría 2D nativa para PlagueTUI. Compilada como DLL (`plague_geometry.dll` / `libplague_geometry.so` / `libplague_geometry.dylib`) con ABI C pura.

---

## Tipos

| Tipo | Campos | Descripción |
|------|--------|-------------|
| `Offset` | `x, y: f32` | Posición o desplazamiento 2D |
| `Size` | `width, height: f32` | Dimensiones 2D |
| `Region` | `x, y, width, height: f32` | Rectángulo posicionado |
| `Spacing` | `top, right, bottom, left: f32` | Márgenes o padding |

---

## Compilar

```bat
zig build
```

El DLL queda en `zig-out/bin/plague_geometry.dll` (Windows) o `zig-out/lib/` (Linux/macOS).

---

## Usar desde Python

```python
import ctypes
from pathlib import Path

lib = ctypes.CDLL(str(Path("zig-out/bin/plague_geometry.dll")))

class Offset(ctypes.Structure):
    _fields_ = [("x", ctypes.c_float), ("y", ctypes.c_float)]

class Region(ctypes.Structure):
    _fields_ = [("x", ctypes.c_float), ("y", ctypes.c_float),
                ("width", ctypes.c_float), ("height", ctypes.c_float)]

lib.tg_offset_add.argtypes = [Offset, Offset]
lib.tg_offset_add.restype  = Offset

lib.tg_region_center.argtypes = [Region]
lib.tg_region_center.restype  = Offset

# Sumar dos offsets
result = lib.tg_offset_add(Offset(10.0, 20.0), Offset(5.0, 5.0))
print(result.x, result.y)  # 15.0 25.0

# Centro de una región
center = lib.tg_region_center(Region(0.0, 0.0, 100.0, 200.0))
print(center.x, center.y)  # 50.0 100.0
```

---

## Usar desde C / C++

```c
#include "include/plague_geometry.h"

TG_Offset a = {10.0f, 20.0f};
TG_Offset b = {5.0f,  5.0f};
TG_Offset r = tg_offset_add(a, b);  // {15.0, 25.0}

TG_Region region = {0.0f, 0.0f, 100.0f, 200.0f};
TG_Offset center = tg_region_center(region);  // {50.0, 100.0}
```

---

## API Completa

### Offset
| Función | Descripción |
|---------|-------------|
| `tg_offset_add(a, b)` | Suma dos offsets |
| `tg_offset_sub(a, b)` | Resta dos offsets |
| `tg_offset_scale(a, factor)` | Escala por factor |
| `tg_offset_eq(a, b)` | Igualdad |

### Size
| Función | Descripción |
|---------|-------------|
| `tg_size_add(a, b)` | Suma dos sizes |
| `tg_size_scale(a, factor)` | Escala por factor |
| `tg_size_area(a)` | Área (`width * height`) |
| `tg_size_eq(a, b)` | Igualdad |

### Region
| Función | Descripción |
|---------|-------------|
| `tg_region_contains(r, o)` | Contiene el punto `o` (inclusivo) |
| `tg_region_clip(a, b)` | Intersección — retorna `REGION_ZERO` si no se tocan |
| `tg_region_union(a, b)` | Unión mínima que cubre ambas |
| `tg_region_translate(r, o)` | Traslada la región por `o` |
| `tg_region_inflate(r, s)` | Expande por spacing |
| `tg_region_deflate(r, s)` | Contrae por spacing (mínimo `0`) |
| `tg_region_is_empty(r)` | `true` si `width <= 0` o `height <= 0` |
| `tg_region_center(r)` | Punto central como `Offset` |
| `tg_region_size(r)` | Extrae dimensiones como `Size` |
| `tg_region_eq(a, b)` | Igualdad |

### Spacing
| Función | Descripción |
|---------|-------------|
| `tg_spacing_uniform(value)` | Crea spacing igual en los 4 lados |
| `tg_spacing_add(a, b)` | Suma dos spacings |
| `tg_spacing_eq(a, b)` | Igualdad |

---

## Tests

```bat
test.bat
```

Compila el DLL y corre los 61 tests (unitarios + contraste contra Textual).

---

## Notas

- Coordenadas en `f32` — precisión suficiente para layout de UI a cualquier resolución.
- Sin memoria dinámica, sin dependencias externas.
- `region_clip` sin intersección retorna `{0,0,0,0}`. Difiere del comportamiento de Textual, que preserva el extent en el eje que sí se superpone.

---

## Créditos

Los tests de contraste (`tests/test_contrast.py`) utilizan `tests/geometry.py`, extraído del proyecto [Textual](https://github.com/Textualize/textual) como implementación de referencia.

Textual es software libre distribuido bajo la licencia MIT:

```
Copyright (c) 2021 Will McGugan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
