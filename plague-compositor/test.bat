@echo off
echo [1/2] Compilando DLL...
call build.bat
if %errorlevel% neq 0 (
    echo ERROR: Fallo la compilacion.
    exit /b 1
)

echo [2/2] Ejecutando tests...
python -m unittest discover -s tests -v
