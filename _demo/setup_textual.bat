@echo off
REM Crea un venv con Textual para la demo de comparación
cd /d "%~dp0"

if exist .venv (
    echo [OK] .venv ya existe
    goto done
)

echo [*] Creando venv...
python -m venv .venv
if errorlevel 1 (
    echo [ERROR] Fallo al crear el venv
    exit /b 1
)

echo [*] Instalando Textual...
.venv\Scripts\pip install textual
if errorlevel 1 (
    echo [ERROR] Fallo al instalar Textual
    exit /b 1
)

:done
echo.
echo [OK] Listo. Ejecuta:
echo     .venv\Scripts\python demo_textual.py
