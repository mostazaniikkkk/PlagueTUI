@echo off
if not exist bin mkdir bin

set APP=..\plague-app

gcc -shared -o bin\plague_widgets.dll ^
    src\pw_impl.c ^
    -I include ^
    -I %APP%\include ^
    -I %APP%\src ^
    -I ..\plague-geometry\include ^
    -I ..\plague-layout\include ^
    %APP%\bin\plague_app.dll ^
    -Os -Wall -Wextra -lm

if %errorlevel% neq 0 (
    echo BUILD FAILED
    exit /b 1
)

echo BUILD OK: bin\plague_widgets.dll

:: Copy all dependencies into local bin\ for tests
copy /Y %APP%\bin\plague_app.dll       bin\ >nul
copy /Y %APP%\bin\plague_layout.dll    bin\ >nul
copy /Y %APP%\bin\plague_terminal.dll  bin\ >nul
copy /Y %APP%\bin\plague_css.dll       bin\ >nul
copy /Y %APP%\bin\plague_events.dll    bin\ >nul
echo Dependencias copiadas a bin\
