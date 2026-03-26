@echo off
echo Verificando builds...

if not exist ..\plague-app\bin\plague_app.dll (
    echo Construyendo plague-app...
    pushd ..\plague-app && call build.bat && popd
)

if not exist ..\plague-widgets\bin\plague_widgets.dll (
    echo Construyendo plague-widgets...
    pushd ..\plague-widgets && call build.bat && popd
)

echo Iniciando demo...
python demo_plague_widgets.py
