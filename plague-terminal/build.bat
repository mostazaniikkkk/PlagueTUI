@echo off
if not exist bin mkdir bin
gcc -shared -o bin\plague_terminal.dll ^
    src\terminal.c ^
    src\cell_buffer.c ^
    src\ansi_writer.c ^
    src\draw_impl.c ^
    src\win32\terminal_io.c ^
    -I include ^
    -I src ^
    -I ..\plague-geometry\include ^
    -I ..\plague-drawcontext\src ^
    -Os
