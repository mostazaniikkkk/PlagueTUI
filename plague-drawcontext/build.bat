@echo off
if not exist bin mkdir bin
gcc -shared -o bin\plague_drawcontext.dll ^
    src\color.c ^
    src\text_style.c ^
    src\stub_context.c ^
    -I src ^
    -I ..\plague-geometry\include ^
    -Os
