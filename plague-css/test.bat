@echo off
call build.bat
if %errorlevel% neq 0 exit /b 1

cd tests
python -m unittest discover -v
cd ..
