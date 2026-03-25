@echo off
if not exist bin mkdir bin

gcc -shared -o bin\plague_css.dll ^
    src\css.c ^
    src\parser.c ^
    src\cascade.c ^
    src\colors.c ^
    -I include ^
    -I src ^
    -I ..\plague-geometry\include ^
    -I ..\plague-layout\include ^
    -Os -Wall -Wextra

if %errorlevel% neq 0 (
    echo BUILD FAILED
    exit /b 1
)
echo BUILD OK: bin\plague_css.dll
