@echo off
if not exist bin mkdir bin

set DEPS=..\plague-geometry\include
set LAYOUT=..\plague-layout
set TERMINAL=..\plague-terminal
set CSS=..\plague-css
set EVENTS=..\plague-events
set DRAWCTX=..\plague-drawcontext

gcc -shared -o bin\plague_app.dll ^
    src\app.c ^
    src\widgets.c ^
    src\render.c ^
    -I include ^
    -I src ^
    -I %DEPS% ^
    -I %LAYOUT%\include ^
    -I %TERMINAL%\include ^
    -I %CSS%\include ^
    -I %EVENTS%\include ^
    -I %DRAWCTX%\src ^
    %LAYOUT%\bin\plague_layout.dll ^
    %TERMINAL%\bin\plague_terminal.dll ^
    %CSS%\bin\plague_css.dll ^
    %EVENTS%\bin\plague_events.dll ^
    -Os -Wall -Wextra

if %errorlevel% neq 0 (
    echo BUILD FAILED
    exit /b 1
)

echo BUILD OK: bin\plague_app.dll

:: Copiar DLLs dependientes al bin local para que los tests las encuentren
copy /Y %LAYOUT%\bin\plague_layout.dll   bin\ >nul
copy /Y %TERMINAL%\bin\plague_terminal.dll bin\ >nul
copy /Y %CSS%\bin\plague_css.dll         bin\ >nul
copy /Y %EVENTS%\bin\plague_events.dll   bin\ >nul
echo Dependencias copiadas a bin\
