@echo off
if not exist bin mkdir bin
gcc -shared -o bin\plague_layout.dll ^
    src\layout.c ^
    src\tree.c ^
    src\compute.c ^
    src\helpers.c ^
    -I include ^
    -I src ^
    -I ..\plague-geometry\include ^
    -Os
