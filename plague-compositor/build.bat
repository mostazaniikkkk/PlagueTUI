@echo off
if not exist bin mkdir bin
gcc -shared -o bin\plague_compositor.dll ^
    src\tree.c ^
    src\renderer.c ^
    src\helpers.c ^
    -I src ^
    -I ..\plague-geometry\include ^
    -I ..\plague-drawcontext\include ^
    -I ..\plague-drawcontext\src ^
    -Os
