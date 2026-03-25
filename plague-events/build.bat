@echo off
if not exist bin mkdir bin

gcc -shared -o bin\plague_events.dll ^
    src\events.c ^
    src\queue.c ^
    src\tree.c ^
    src\bindings.c ^
    src\timers.c ^
    -I include ^
    -I src ^
    -Os -Wall -Wextra

if %errorlevel% neq 0 (
    echo BUILD FAILED
    exit /b 1
)
echo BUILD OK: bin\plague_events.dll
